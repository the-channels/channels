
class ChannelBoard(object):
    def __init__(self, id, title, description):
        self.id = id
        self.title = title
        self.description = description


class ChannelThread(object):
    def __init__(self, id):
        self.id = id
        self.title = None
        self.comment = None
        self.attachment = None
        self.attachment_width = 0
        self.attachment_height = 0
        self.num_replies = 0
        self.date = 0


class ChannelPost(object):
    def __init__(self, id):
        self.id = id
        self.title = None
        self.comment = None
        self.attachment = None
        self.attachment_width = 0
        self.attachment_height = 0
        self.replies = []
        self.date = 0


class ChannelsError(Exception):
    UNKNOWN_ERROR = 0

    def __init__(self, code):
        self.code = code


class Client(object):
    """
    A class the represents a connected client. A client could have it's settings, which it can retrieve and update.
    :param id: would be a client's id
    TODO: Settings API is not implemented yet.
    """
    def __init__(self, id):
        self.id = id
        self.settings = {}


class Channel(object):
    """
    Take care to implement all methods. Pretty much all methods receive a client instance,
    which could be used for some internal caching.
    """
    def name(self):
        """
        This function should return channel name
        """
        raise NotImplementedError()

    def get_cache_key(self, key):
        return "cache/" + self.name() + "_" + key

    def get_attachment(self, client, board, thread, post, attachment, width, height):
        """
        This function should download an attachment off its source and return it's cached path.
        get_cache_key function could be used to obtain local target to download the file to.

        :param client: a Client instance
        :param board: Board
        :param thread: Thread
        :param post: Post
        :param attachment: An attachment (no domain) URL obtained eirlier
        :param width: Attachment width for reference
        :param height: Attachment height for reference
        :return: A location of the downloaded attachment inside of the cache (get_cache_key should be used)
        """
        raise NotImplementedError()

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
