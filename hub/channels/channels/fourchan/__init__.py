from channels.base import Channel, ChannelsError, ChannelBoard, ChannelThread, ChannelPost

import requests
import os
import re


class FourChanChannel(Channel):
    base_url = "https://a.4cdn.org"
    attachment_url = "https://i.4cdn.org"
    tag_pattern = re.compile('<.*?>')

    def __init__(self):
        super().__init__()

    def name(self):
        return "4chan"

    @staticmethod
    def __strip__html__(s: str):
        s = s.replace("<br>", "\n")
        s = re.sub(FourChanChannel.tag_pattern, '', s)
        s = s.replace("&#039;", "'")
        s = s.replace("&quot;", "\"")
        s = s.replace("&amp;", "&")
        s = s.replace("&gt;", ">")
        s = s.replace("&lt;", "<")
        return s

    def get_attachment(self, client, board, thread, post, attachment, width, height):
        attachment_path = self.get_cache_key(board + "_" + attachment)
        if os.path.isfile(attachment_path):
            return attachment_path

        r = requests.get(FourChanChannel.attachment_url + "/" + board + "/" + attachment)
        if r.status_code != 200:
            raise ChannelsError(ChannelsError.UNKNOWN_ERROR)
        with open(attachment_path, mode='wb') as f:
            f.write(r.content)
        return attachment_path

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
                if ("tim" in thread) and ("ext" in thread) and ("w" in thread) and ("h" in thread) :
                    result_thread.attachment = str(thread["tim"]) + thread["ext"]
                    result_thread.attachment_width = thread["w"]
                    result_thread.attachment_height = thread["h"]
                result_thread.comment = FourChanChannel.__strip__html__(thread["com"])
                threads.append(result_thread)

        return threads

    def get_thread(self, client, board, thread):
        r = requests.get(FourChanChannel.base_url + "/" + board + "/thread/" + thread + ".json")
        if r.status_code != 200:
            raise ChannelsError(ChannelsError.UNKNOWN_ERROR)
        result = r.json()
        posts = []

        for post in result["posts"]:
            if "no" not in post:
                continue
            if "com" not in post:
                continue
            result_post = ChannelPost(str(post["no"]))
            if "sub" in post:
                result_post.title = post["sub"]
            if ("tim" in post) and ("ext" in post) and ("w" in post) and ("h" in post) :
                result_post.attachment = str(post["tim"]) + post["ext"]
                result_post.attachment_width = post["w"]
                result_post.attachment_height = post["h"]
            result_post.comment = FourChanChannel.__strip__html__(post["com"])
            posts.append(result_post)

        return posts


CHANNEL_NAME = "4chan"
CHANNEL_CLASS = FourChanChannel
CHANNEL_DESCRIPTION = "4chan is a simple image-based bulletin board where anyone can post comments and share images."

