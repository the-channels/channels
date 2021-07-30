![image](https://user-images.githubusercontent.com/1666014/126013799-b499d24f-88e2-42b0-8d3d-c77991c4f9ac.png)

ZX Spectrum client for imageboards.

https://user-images.githubusercontent.com/1666014/126671608-02e74708-55c7-4141-b330-0c9b010e1c9d.mov

## Download

You can do that from the [Releases](https://github.com/the-channels/channels/releases) page.

## Requirements
* Because speccy does not support any internet connectivity by default, you'll need [Spectranet Cartridge](https://www.bytedelight.com/?page_id=3515). It may be pricey and hard to get, but please support the manufacturer as it's a very Niche product.
* [Fuse Enumator](http://fuse-emulator.sourceforge.net/) supports such cartrige, you'll just need to enable it via "Peripherials->Spectranet" radio button

## To Do List
1. ~~Refactor img2spec tool and split display and processing logic in separate static
   library with no executable~~
2. Ability to save settings
3. Tracking and showing replies to particular post
4. Posting
5. Optimize font rendering speed
6. Cleanup cache after certain timeout
7. Cache the transcoded images and not only originals

## Channels Hub
Because speccy is nowhere near as powerful for SSL, you need an intermediate proxy to preprocess the data.
A special proxy called Hub (`hub/` folder) is supposed to be run somewhere on your local network, e.g. a PC, a router, a raspberry PI etc.

* This proxy does all the heavy lifting, like image spectrumisation (with help of [img2spec](https://github.com/the-channels/img2spec) tool)
* The proxy implements a set of Channels (like a channel for 4chan) and facilitates external calls to provide a standartized API for each channel
* The client doesn't even care what channels are, as long as they use the same API, so potential of having the client to browse pretty much anything is limitless, as long as you have boards (or categories, you can just have one) and a set of conversations (threads) with each thread having posts.

### How channels work?

Despite being written in C++, the Hub uses Python to convert channel specifics into
a standardized form.

![channels](https://user-images.githubusercontent.com/1666014/127716547-670110c4-4c16-47a9-8a72-954963ec54fc.png)

Logic blocks marked here in green are Python packages.
The hub simply scans installed packages that match and loads them up.

Some packaged are distributed with the hub, but other could be insalled into the sytem,
and the Hub would detect it. To read on how to write a new channel, refer to [Creating A New Channel](NewChannel.md).

## Channels Proto
A client and the hub communicate via a special protocol, as described in [this document](./proto/Readme.md).

## How to build from source
* The Hub is simply compiled using CMake. 
* Take care to fetch all submodules.
* The Client uses Makefiles and can only be compiled using [z88dk](https://github.com/z88dk/z88dk)
* You'd need UNIX to compile the client, so for Windows, you would need to use WSL
* Having all that done, then simply shoot `make`
* While it's included in this repository, 
  you may refer to [spectranet dev library and headers](https://github.com/spectrumero/spectranet)

##### Linux
Install these:
```bash
sudo apt-get update
sudo apt-get install -y \
    gcc \
    git \
    g++ \
    gcc-multilib \
    g++-multilib \
    build-essential \
    xutils-dev \
    libssl-dev \
    cmake
```
Proceed with cmake as usual.

##### Docker
It is also available as a [docker image](https://hub.docker.com/repository/docker/desertkun/channels-hub):
```bash
docker run -d --tmpfs /channels/hub/bin/cache -p 9493:9493 -it desertkun/channels-hub:latest
```
You can build a local image if you want:
```bash
# build
docker build -t channel_hub -f Dockerfile.hub .
# run
docker run -d --tmpfs /channels/hub/bin/cache -p 9493:9493 -it channel_hub
```

## Debugging
* The only way to debug is to use netlog, a printf-alike function that
sends printed text to UDP port 9468 on the same host as proxy.
* To see these logs on Linux/Mac, simply do `make listen-for-logs` while
on Windows, you'd need to install netcat and do `nc -lLu -w 1 -p 9468`.
