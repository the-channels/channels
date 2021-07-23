![image](https://user-images.githubusercontent.com/1666014/126013799-b499d24f-88e2-42b0-8d3d-c77991c4f9ac.png)

ZX Spectrum client for imageboards.

https://user-images.githubusercontent.com/1666014/126671608-02e74708-55c7-4141-b330-0c9b010e1c9d.mov

## Download

You can do that from the [Releases](https://github.com/the-channels/channels/releases) page.

## Requirements
* Because speccy does not support any internet connectivity by default, you'll need [Spectranet Cartridge](https://www.bytedelight.com/?page_id=3515). It may be pricey and hard to get, but please support the manufacturer as it's a very Niche product.
* [Fuse Enumator](http://fuse-emulator.sourceforge.net/) supports such cartrige, you'll just need to enable it via "Peripherials->Spectranet" radio button

## To Do List
1. Refactor img2spec tool and split display and processing logic in separate static
   library with no executable
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

##### Windows
* You need a copy of [SDL2 Development Libraries](https://www.libsdl.org/download-2.0.php), copied into `SDL2` folder next to this repository.
The runtime library is actually included in this repository.

##### Mac Os X
* You need a copy of [SDL2 Development Libraries](https://www.libsdl.org/download-2.0.php) from inside of the .dmg file (`SDL2.framework`), copied to `/Library/Frameworks`.

##### Linux
Install these:
```bash
sudo apt-get update
sudo apt-get install -y \
    gcc \
    git \
    g++ \
    libgtk-3-0 gtk+-3.0 \
    gcc-multilib \
    g++-multilib \
    build-essential \
    xutils-dev \
    libssl-dev \
    libsdl2-dev \
    libsdl2-gfx-dev \
    libsdl2-image-dev \
    libsdl2-mixer-dev \
    libsdl2-net-dev \
    libsdl2-ttf-dev \
    libreadline6-dev \
    libncurses5-dev \
    mingw-w64 \
    cmake
```
Proceed with cmake as usual.

## Debugging
* The only way to debug is to use netlog, a printf-alike function that
sends printed text to UDP port 9468 on the same host as proxy.
* To see these logs on Linux/Mac, simply do `make listen-for-logs` while
on Windows, you'd need to install netcat and do `nc -lLu -w 1 -p 9468`.
