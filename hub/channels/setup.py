from setuptools import setup, find_packages

setup(
    name='channels',
    version="0.3",
    namespace_packages=['channels'],
    install_requires=['requests', 'beautifulsoup4'],
    packages=find_packages(),
)
