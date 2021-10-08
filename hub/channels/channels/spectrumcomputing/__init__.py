from channels.base import Channel, ChannelsError, ChannelBoard, ChannelThread, ChannelPost
from channels.base import ChannelAttachment, SettingDefinition, Client, PostingError

try:
    from BeautifulSoup import BeautifulSoup
except ImportError:
    from bs4 import BeautifulSoup

import requests
import re
import time


class SpectrumComputingClient(Client):
    def __init__(self, channel_name, client_id):
        super().__init__(channel_name, client_id)
        self.session = requests.Session()
        self.authorized = False


class SpectrumComputing(Channel):
    base_url = "https://spectrumcomputing.co.uk/forums"
    board_href_re = re.compile(".*viewforum\\.php\\?f=([0-9]+)")
    threads_href_re = re.compile(".*viewtopic\\.php\\?f=[0-9]+&t=([0-9]+)")
    posts_href_re = re.compile(".*viewtopic\\.php\\?p=([0-9]+)")

    def __init__(self):
        super().__init__()

    def name(self):
        return "spectrumcomputing"

    def get_setting_definitions(self, client):
        return [
            SettingDefinition("username", "Username for posting (optional)"),
            SettingDefinition("password", "Password for posting (optional)"),
            SettingDefinition("max_pages", "Maximum number of pages to preload (default = no limit)"),
        ]

    def new_client(self, client_id):
        return SpectrumComputingClient(self.name(), client_id)

    def authorize(self, client: SpectrumComputingClient):
        if client.authorized:
            return True
        if not client.settings.get("username", None):
            return False
        if not client.settings.get("password", None):
            return False

        r = client.session.get("https://spectrumcomputing.co.uk/forums/ucp.php?mode=login")

        if r.status_code != 200:
            raise PostingError("Cannot login. Is your credentials valid?")

        auth_form = BeautifulSoup(r.content, "html.parser")

        form_token = auth_form.body.find("input", attrs={"name": "form_token"})
        creation_time = auth_form.body.find("input", attrs={"name": "creation_time"})
        redirect = auth_form.body.find("input", attrs={"name": "redirect"})
        sid = auth_form.body.find("input", attrs={"name": "sid"})

        if form_token is None or creation_time is None or redirect is None or sid is None:
            raise PostingError("Cannot login. Is your credentials valid?")

        time.sleep(2)

        r = client.session.post("https://spectrumcomputing.co.uk/forums/ucp.php?mode=login", data={
            "username": client.settings["username"],
            "password": client.settings["password"],
            "form_token": form_token["value"],
            "creation_time": creation_time["value"],
            "redirect": redirect["value"],
            "sid": sid["value"],
            "Referrer": "https://spectrumcomputing.co.uk/forums/ucp.php?mode=login",
            "login": "Login"
        }, headers={'User-Agent': 'Mozilla/5.0'}, allow_redirects=False)
        if r.status_code == 302 and ("location" in r.headers):
            client.authorized = True
            time.sleep(2)
            return True
        return False

    def get_boards(self, client, limit):
        r = client.session.get(SpectrumComputing.base_url)
        if r.status_code != 200:
            raise ChannelsError(ChannelsError.UNKNOWN_ERROR)

        parsed_html = BeautifulSoup(r.content, "html.parser")

        boards = []

        for a in parsed_html.body.find_all('a', attrs={'class': 'forumtitle'}):
            description = a.get_text()
            href = re.match(SpectrumComputing.board_href_re, a["href"])
            _id = str(href.group(1))
            boards.append(ChannelBoard(_id, description.lower().replace('/', ' '), description))

        return boards

    def get_threads(self, client, board):
        print("Fetching threads for board {0}".format(board))
        r = client.session.get(SpectrumComputing.base_url + "/viewforum.php?f={0}".format(board))
        if r.status_code != 200:
            raise ChannelsError(ChannelsError.UNKNOWN_ERROR)

        parsed_html = BeautifulSoup(r.content, "html.parser")

        threads = []

        for dl in parsed_html.body.find_all('dl', attrs={'class': 'row-item'}):
            a = dl.find('a', class_='topictitle')
            if not a:
                continue
            try:
                href = re.match(SpectrumComputing.threads_href_re, a["href"])
                _id = str(href.group(1))
                th = ChannelThread(_id)
                th.title = a.get_text()
                posts_count = dl.find('dd', class_="posts")
                if posts_count:
                    try:
                        th.num_replies = int(posts_count.contents[0])
                    except ValueError:
                        th.num_replies = 0
                if a.parent:
                    comment_preview = a.parent.find(
                        'div', class_='topic_preview_content').find('div', class_='topic_preview_first')
                    if comment_preview:
                        th.comment = Channel.strip_html(comment_preview.get_text())
                    else:
                        th.comment = "<no comment>"
                else:
                    th.comment = "<no comment>"
            except Exception as e:
                print("Failed to process a thread {0}: {1}".format(str(a), str(e)))
            else:
                threads.append(th)

        return threads

    def get_thread(self, client, board, thread):
        posts = []
        posts_by_id = {}

        posts_per_page = 10
        start = 0
        limit = 0
        if "max_pages" in client.settings:
            try:
                limit = int(client.settings["max_pages"])
            except ValueError:
                limit = 0

        while True:
            print("Fetching posts for thread {0}/{1} at start {2}".format(board, thread, start))
            r = client.session.get(
                SpectrumComputing.base_url +
                "/viewtopic.php?f={0}&t={1}&start={2}".format(board, thread, start))

            if r.status_code != 200:
                if not posts:
                    raise ChannelsError(ChannelsError.UNKNOWN_ERROR)
                else:
                    return posts

            start += posts_per_page
            parsed_html = BeautifulSoup(r.content, "html.parser")

            for post_body in parsed_html.body.find_all('div', attrs={'class': 'post'}):
                try:
                    h3 = post_body.find('h3')
                    if h3 is None:
                        continue
                    a = h3.find('a')
                    href = re.match(SpectrumComputing.posts_href_re, a["href"])
                    _id = str(href.group(1))
                    p = ChannelPost(_id)
                    content = post_body.find('div', class_="content")
                    author = post_body.find('a', class_="username")
                    if author:
                        p.title = "by {0}".format(author.get_text())

                    for quote in content.find_all('blockquote'):
                        for link in quote.find_all('a'):
                            try:
                                reply_to = link["data-post-id"]
                            except KeyError:
                                continue
                            if reply_to and (reply_to in posts_by_id):
                                posts_by_id[reply_to].replies.append(p.id)
                        quote.replace_with(">quote")

                    for img in content.find_all('img', class_="postimage"):
                        p.attachments.append(ChannelAttachment(img["src"]))
                        img.replace_with("")

                    p.comment = Channel.strip_html(content.get_text())
                except Exception as e:
                    print("Failed to process a post {0}".format(post_body.get_text()))
                else:
                    posts.append(p)
                    posts_by_id[p.id] = p

            if not parsed_html.find('li', class_="arrow next"):
                break

            if limit:
                limit -= 1
                if limit == 0:
                    break

        return posts

    def post(self, client, board, thread, comment, reply_to):
        if not self.authorize(client):
            raise PostingError("Cannot authorize. Provide valid credentials.")

        if reply_to:
            req_url = "https://spectrumcomputing.co.uk/forums/posting.php?mode=quote&f={0}&p={1}".format(board, reply_to)
        else:
            req_url = "https://spectrumcomputing.co.uk/forums/posting.php?mode=reply&f={0}&t={1}".format(board, thread)

        r = client.session.get(req_url, headers={'User-Agent': 'Mozilla/5.0'})

        if r.status_code != 200:
            raise PostingError("Cannot get a post form: {0}. Is your credentials valid?".format(r.status_code))

        html_form = BeautifulSoup(r.content, "html.parser")
        form = html_form.body.find('form', id="postform")
        if form is None:
            raise PostingError("Cannot get a post form ref. Is your credentials valid?")

        subject = form.find('input', attrs={"name": "subject"})
        if subject is None:
            raise PostingError("Cannot get subject ref. Is your credentials valid?")

        topic_cur_post_id = form.find("input", attrs={"name": "topic_cur_post_id"})
        if topic_cur_post_id is None:
            raise PostingError("Cannot get topic_cur_post_id.")

        creation_time = form.find("input", attrs={"name": "creation_time"})
        if creation_time is None:
            raise PostingError("Cannot get creation_time.")

        form_token = form.find("input", attrs={"name": "form_token"})
        if form_token is None:
            raise PostingError("Cannot get form_token.")

        if reply_to:
            existing_message = form.find('textarea', attrs={"name": "message"})
            comment = "{0}\n{1}".format(existing_message.contents[0], comment)

        if reply_to:
            post_url = "https://spectrumcomputing.co.uk/forums/posting.php?mode=quote&f={0}&t={1}&p={2}".format(
                board, thread, reply_to)
        else:
            post_url = "https://spectrumcomputing.co.uk/forums/posting.php?mode=reply&f={0}&t={1}".format(board, thread)

        r = client.session.post(post_url, data={
            "message": comment,
            "subject": subject["value"],
            "form_token": form_token["value"],
            "creation_time": creation_time["value"],
            "post": "Submit",
            "topic_cur_post_id": topic_cur_post_id["value"]
        }, headers={'User-Agent': 'Mozilla/5.0'}, allow_redirects=False)

        if r.status_code != 302 or ("location" not in r.headers):
            raise PostingError("Cannot post: {0}. Is your credentials valid?".format(r.status_code))


CHANNEL_NAME = "spectrumcomputing"
CHANNEL_CLASS = SpectrumComputing
CHANNEL_DESCRIPTION = "The community forum for all ZX Spectrum users"

