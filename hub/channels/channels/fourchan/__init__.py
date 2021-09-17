from channels.base import Channel, ChannelsError, ChannelAttachment, ChannelBoard, ChannelThread
from channels.base import ChannelPost, SettingDefinition

import requests
import re


class FourChanChannel(Channel):
    base_url = "https://a.4cdn.org"
    attachment_url = "https://i.4cdn.org"
    post_reply_pattern = re.compile(r'>>([0-9]+)', re.MULTILINE)

    def __init__(self):
        super().__init__()

    def name(self):
        return "4chan"

    def get_setting_definitions(self, client):
        return [
            # SettingDefinition("pass", "Pass code for posting"),
        ]

    def get_boards(self, client, limit):
        r = requests.get(FourChanChannel.base_url + "/boards.json")
        if r.status_code != 200:
            raise ChannelsError(ChannelsError.UNKNOWN_ERROR)
        response = r.json()
        boards = []

        for board in response["boards"]:
            boards.append(ChannelBoard(board["board"], None, board["title"]))

        return boards

    def get_threads(self, client, board):
        r = requests.get(FourChanChannel.base_url + "/" + board + "/catalog.json")
        if r.status_code != 200:
            raise ChannelsError(ChannelsError.UNKNOWN_ERROR)
        pages = r.json()
        threads = []

        for page in pages:
            for thread in page["threads"]:
                if "no" not in thread:
                    continue
                if "com" not in thread:
                    continue
                result_thread = ChannelThread(str(thread["no"]))
                if "sub" in thread:
                    result_thread.title = thread["sub"]
                result_thread.num_replies = thread["replies"] if "replies" in thread else 0
                if ("tim" in thread) and ("ext" in thread):
                    url = FourChanChannel.attachment_url + "/" + board + "/" + str(thread["tim"]) + thread["ext"]
                    result_thread.attachments.append(ChannelAttachment(url))
                result_thread.comment = Channel.strip_html(thread["com"])
                threads.append(result_thread)

        return threads

    def get_thread(self, client, board, thread):
        r = requests.get(FourChanChannel.base_url + "/" + board + "/thread/" + thread + ".json")
        if r.status_code != 200:
            raise ChannelsError(ChannelsError.UNKNOWN_ERROR)
        result = r.json()
        posts = []
        posts_by_id = {}

        for post in result["posts"]:
            if "no" not in post:
                continue
            if "com" not in post:
                continue
            result_post = ChannelPost(str(post["no"]))
            if "sub" in post:
                result_post.title = post["sub"]
            if ("tim" in post) and ("ext" in post):
                url = FourChanChannel.attachment_url + "/" + board + "/" + str(post["tim"]) + post["ext"]
                result_post.attachments.append(ChannelAttachment(url))
            result_post.comment = Channel.strip_html(post["com"])

            for reply in re.finditer(FourChanChannel.post_reply_pattern, result_post.comment):
                reply_to = reply.group(1)
                if reply_to in posts_by_id:
                    posts_by_id[reply_to].replies.append(result_post.id)

            posts.append(result_post)
            posts_by_id[result_post.id] = result_post

        return posts


CHANNEL_NAME = "4chan"
CHANNEL_CLASS = FourChanChannel
CHANNEL_DESCRIPTION = "4chan is a simple image-based bulletin board where anyone can post comments and share images."

