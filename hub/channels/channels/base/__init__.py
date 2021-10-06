import re
import requests
import hashlib
import os
import json


class ChannelBoard(object):
    def __init__(self, id, title, description):
        self.id = id
        self.title = title
        self.description = description


class ChannelAttachment(object):
    def __init__(self, url):
        self.url = url


class ChannelThread(object):
    def __init__(self, id):
        self.id = id
        self.title = None
        self.comment = None
        self.attachments = []
        self.num_replies = 0
        self.date = 0


class ChannelPost(object):
    def __init__(self, id):
        self.id = id
        self.title = None
        self.comment = None
        self.attachments = []
        self.replies = []
        self.date = 0


class SettingDefinition(object):
    def __init__(self, key, description):
        self.id = key
        self.description = description


class ChannelsError(Exception):
    UNKNOWN_ERROR = 0

    def __init__(self, code):
        self.code = code


class Client(object):
    """
    A class the represents a connected client. A client could have it's settings, which it can retrieve and update.
    :param id: would be a client's id
    """
    def __init__(self, channel_name, id):
        self.channel_name = channel_name
        self.id = id
        self.settings = {}
        self.key = None
        self.hashed_key = None

    def set_option(self, key, value):
        self.settings[key] = value

    def save_options(self):
        if self.hashed_key:
            with open(os.path.join("conf", self.hashed_key), "w") as f:
                json.dump(self.settings, f)

    def get_option(self, key):
        return self.settings.get(key, "")

    def set_key(self, key):
        self.key = key
        s = hashlib.sha256(self.channel_name.encode("utf8"))
        s.update(self.key)
        self.hashed_key = s.hexdigest()
        if not os.path.isdir("conf"):
            os.mkdir("conf")
        if os.path.isfile(os.path.join("conf", self.hashed_key)):
            with open(os.path.join("conf", self.hashed_key), "r") as f:
                self.settings = json.load(f)


class Channel(object):
    tag_pattern = re.compile(r'<.*?>', re.MULTILINE)

    """
    Take care to implement all methods. Pretty much all methods receive a client instance,
    which could be used for some internal caching.
    """
    def name(self):
        """
        This function should return channel name
        """
        raise NotImplementedError()

    @staticmethod
    def strip_html(s: str):
        s = s.replace("<br>", "\n")
        s = re.sub(Channel.tag_pattern, '', s)
        s = s.replace("&#039;", "'")
        s = s.replace("&quot;", "\"")
        s = s.replace("&amp;", "&")
        s = s.replace("&gt;", ">")
        s = s.replace("&lt;", "<")
        return s

    @staticmethod
    def split_index(s: str, text_width: int):
        index = 0
        height = 0
        for line in s.split('\n'):
            while len(line) > text_width:
                line = line[text_width:]
                index += text_width
                height += 1
                if height >= 22:
                    return index

            index += len(line)
            height += 1
            if height >= 22:
                return index

            index += 1
        return None

    @staticmethod
    def split_comment(s: str):
        while True:
            i = Channel.split_index(s, 44)
            if i is None:
                break
            yield s[:i]
            s = s[i:]
        yield s

    def get_cache_key(self, key):
        return "cache/" + self.name() + "_" + key

    def get_setting_definitions(self, client):
        """
        This method should return a list of SettingDefinition objects, each defining a particular setting this
        channel supports. The client can use those definitions as a guide to setup client.settings object, which is
        provided on every call. The client, however, can put there whatever it wants.
        """
        return []

    def get_attachment(self, client, url):
        """
        This function should download an attachment off its source and return it's cached path.
        get_cache_key function could be used to obtain local target to download the file to.

        :param client: a Client instance
        :param url: a url to download attachment from
        :return: A location of the downloaded attachment inside of the cache (get_cache_key should be used)
        """
        attachment_hash = hashlib.sha256(url.encode("utf8")).hexdigest() + ".jpg"
        attachment_path = self.get_cache_key(attachment_hash)
        if os.path.isfile(attachment_path):
            return attachment_path
        r = requests.get(url)
        if r.status_code != 200:
            raise ChannelsError(ChannelsError.UNKNOWN_ERROR)
        with open(attachment_path, mode='wb') as f:
            f.write(r.content)
        return attachment_path

    def get_boards(self, client, limit):
        """
        :param client: a Client instance
        :param limit: Maximum amount the client requests
        :return: A list of ChannelBoard objects this channel supports. Take care to fill object's properties
        """
        raise NotImplementedError()

    def get_threads(self, client, board):
        """
        :param client: a Client instance
        :param board: a board id (str)
        :return: A list of ChannelThread objects for a particular board. Take care to fill object's properties
        """
        raise NotImplementedError()

    def get_thread(self, client, board, thread):
        """
        :param client: a Client instance
        :param board: a board id (str)
        :param thread: a thread id (str)
        :return: A list of ChannelPost objects for a particular board's thread. Take care to fill object's properties
        """
        raise NotImplementedError()


# noinspection PyUnresolvedReferences
def import_modules(register_channel):
    """
    This function should not be called, it is only called by the hub itself
    """
    import pkgutil
    import channels

    for pkg in pkgutil.iter_modules(channels.__path__, channels.__name__ + "."):
        if pkg.name == "channels.base":
            continue
        print("Importing module: {0}".format(pkg.name))
        submodule = pkg.name.split(".")[1]
        module = getattr(__import__(pkg.name), submodule)
        try:
            channel_name = getattr(module, "CHANNEL_NAME")
            channel_description = getattr(module, "CHANNEL_DESCRIPTION")
            channel_class = getattr(module, "CHANNEL_CLASS")
        except AttributeError:
            print("Skipping module {0} (no CHANNEL_NAME or CHANNEL_DESCRIPTION or CHANNEL_CLASS)".format(pkg.name))
            continue
        register_channel(channel_name, channel_description, channel_class)
