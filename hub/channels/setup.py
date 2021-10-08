from setuptools import setup, find_packages

setup(
    name='the-channels',
    version="0.5",
    namespace_packages=['channels'],
    install_requires=['requests', 'beautifulsoup4'],
    packages=find_packages(),
)
