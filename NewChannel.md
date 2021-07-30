![image](https://user-images.githubusercontent.com/1666014/126013799-b499d24f-88e2-42b0-8d3d-c77991c4f9ac.png)

# How channels work

Despite being written in C++, the Hub uses Python to convert channel specifics into
a standardized form.

![channels](https://user-images.githubusercontent.com/1666014/127716547-670110c4-4c16-47a9-8a72-954963ec54fc.png)

Logic blocks marked here in green are Python packages.
The hub simply scans installed packages that match and loads them up.

Some packaged are distributed with the hub, but other could be insalled into the sytem,
and the Hub would detect it. 

# How to create a new channel

You need to create a python package like so:
```bash
your-project-path/
.. channels/
.... __init__.py
.... your_channel/
...... __init__.py
.. setup.py
```
Let's take a look at each individual file.

### `setup.py`

A setup script which most python packages use, could look like so:
```python
from setuptools import setup, find_packages

setup(
    name='your_channel',
    version="0.3",
    namespace_packages=['channels'],
    packages=find_packages(),
)
```

It's important to have the `namespace_packages` property as the Hub relies on python's [Namespace Packages](https://packaging.python.org/guides/packaging-namespace-packages/).

### `channels/__init__.py`

This file should be exactly this:

```python
__import__('pkg_resources').declare_namespace(__name__)
```

### `channels/your_channel/__init__.py`

This is your main file, which will have your channel implementation.
You can refer to [4chan's implementation](hub/channels/channels/fourchan/__init__.py) as an example.
It should work like so:
* It should have a class derived from `Channel`, which will be your channel's logic.
* This class should implement all [methods as documented](hub/channels/channels/base/__init__.py)
* Your package should expose variables like so:
  `CHANNEL_NAME`, `CHANNEL_CLASS` and `CHANNEL_DESCRIPTION`
  which should be your channel name as client would see it,
  the reference to your new class and channel description.

# How to install it?
Well, just shoot `python3 setup.py install`.
You can even publish it on pip if you want, package name is irrelevant,
as long as it keeps the integrity of the `channels.*` namespace package.

You can also have a docker image that's based [off the one Hub uses](Dockerfile.hub)
and install additional python packages for new channels you want.

# But how does it work?
Upon startup the Hub calls `channels.base.import_modules(cb)`,
this functions then finds all suitable packages and calls callback for it,
which is a function to register a channel.