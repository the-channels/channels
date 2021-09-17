from channels.base import Channel, ChannelsError, ChannelBoard, ChannelThread, ChannelPost
from channels.base import ChannelAttachment, SettingDefinition

try:
    from BeautifulSoup import BeautifulSoup
except ImportError:
    from bs4 import BeautifulSoup

import requests
import re


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
            SettingDefinition("max_pages", "Maximum number of pages to preload (default = no limit)"),
        ]

    def get_boards(self, client, limit):
        r = requests.get(SpectrumComputing.base_url)
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
        r = requests.get(SpectrumComputing.base_url + "/viewforum.php?f={0}".format(board))
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
            r = requests.get(
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


CHANNEL_NAME = "spectrumcomputing"
CHANNEL_CLASS = SpectrumComputing
CHANNEL_DESCRIPTION = "The community forum for all ZX Spectrum users"

