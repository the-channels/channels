![image](https://user-images.githubusercontent.com/1666014/126013799-b499d24f-88e2-42b0-8d3d-c77991c4f9ac.png)

ZX Spectrum browser for forums and imageboards.

https://user-images.githubusercontent.com/1666014/126671608-02e74708-55c7-4141-b330-0c9b010e1c9d.mov

## Download

You can do that from the [Releases](https://github.com/the-channels/channels/releases) page 
or simply tune in TNFS to `zx.desertkun.in` as described [Here](https://channels.desertkun.in/#public-tnfs)

## Emulation
| Emulator | How To Enable           |
|------------|-------------------------|
| [Es.pectrum](https://www.habisoft.com/espectrum/EN.htm#Descargas) (Windows)  | Select, Options->Hardware, check "Spectranet", do steps described above |
| [Fuse Emulator](http://fuse-emulator.sourceforge.net/)   | Peripherials->Spectranet, open snapshot (see Releases), NMI->Configure network->Change IP & Gateway, do steps described above |

## Required hardware
Because speccy does not support any internet connectivity by default, with physical device
you'll need [Spectranet Cartridge](https://www.bytedelight.com/?page_id=3515).
It may be pricey and hard to get, but please support the manufacturer as it's a very Niche product.

## To Do List
1. ~~Refactor img2spec tool and split display and processing logic in separate static
   library with no executable~~
2. ~~Ability to save settings~~
3. ~~Tracking and showing replies to particular post~~
4. ~~Posting~~
5. ~~Optimize font rendering speed~~
6. Cleanup cache after certain timeout
7. Cache the transcoded images and not only originals

## Channels Hub
Because speccy is nowhere near as powerful for SSL, you need an intermediate proxy to preprocess the data.
A special proxy called Hub (`hub/` folder) is supposed to be run somewhere on your local network, e.g. a PC, a router, a raspberry PI etc.

* This proxy does all the heavy lifting, like image spectrumisation (with help of [img2spec](https://github.com/the-channels/img2spec) tool)
* The proxy implements a set of Channels (like a channel for 4chan) and facilitates external calls to provide a standartized API for each channel
* The client doesn't even care what channels are, as long as they use the same API, so potential of having the client to browse pretty much anything is limitless, as long as you have boards (or categories, you can just have one) and a set of conversations (threads) with each thread having posts.

### How channels work?

Want to see your favorite website with this browser? Refer to [Creating A New Channel](NewChannel.md).

The Hub uses Python to convert channel specifics into
a standardized form.

![channels](https://user-images.githubusercontent.com/1666014/127716547-670110c4-4c16-47a9-8a72-954963ec54fc.png)

Logic blocks marked here in green are Python packages.
The hub simply scans installed packages that match and loads them up.

Some packaged are distributed with the hub, but other could be insalled into the sytem,
and the Hub would detect it. 

## Channels Proto
A client and the hub communicate via a special protocol, as described in [this document](./proto/Readme.md).

## Architecture support

Channels client can be build for zx spectrum (`make`), but also it can be compiled for desktop (`make ARCH=desktop`) as a terminal.
There is not much point in having such a limited browser for PC, but it showcases the project could be build for implemented platforms.

The platform-specific files are located in `arch/<platform>`, files in that location need to satisfy various symbols that system needs,
like `get_device_unique_key`.

The `include/system.h` has various platform-specific defines (like `STACKLESS_PROCESS`) 
that shared code could use for `#ifdef` statements.

For network capabilities, the system expects POSIX-like sockets to be implemented.

## How to build from source
* The Hub is simply compiled using CMake. 
* Take care to fetch all submodules.
* The python CMake finds could be different from the python in your PATH, in that case you'd need to address that
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
It is also available as a [docker image](https://hub.docker.com/r/desertkun/channels-hub):
```bash
docker run -d --tmpfs /channels/hub/bin/cache -p 9493:9493 -p 16384:16384/udp -it desertkun/channels-hub:latest
```
You can build a local image if you want:
```bash
# build
docker build -t channel_hub .
# run
docker run -d --tmpfs /channels/hub/bin/cache -p 9493:9493 -p 16384:16384/udp -it channel_hub
```

## Debugging

### Debugging python packages (channels) with PyCharm Professional

* Build cmake in Debug mode. This way, python library would be installed in
  "develop" mode (`setup.py develop`), and you won't have to reinstall updated
  packages every time.
* Open PyCharm and open up `hub/channels` folder in it. Run a `Python Debug Server`
  configuration on port `5678`.
* Specify the `-d` option to the Hub when running. When run, the hub will
  attempt to join to the pycharm with `pydevd` to port `5678`.
  Note that with `-d` option, the hub is running in a single threaded mode, so
  it only can serve one client connection.
* PyCharm breakpoints should work. After any modification, all you need is to
  restart the hub (assuming you did the first step).
  
### Debugging python packages (channels) with Eclipse

* Alternatively, install [Eclipse](https://www.eclipse.org/downloads/packages/installer)
* Build cmake in Debug mode. This way, python library would be installed in
  "develop" mode (`setup.py develop`), and you won't have to reinstall updated
  packages every time.
* Install `python3 pip install pydevd`
* Select `Eclipse IDE for Eclipse Committers`
* Select `Help -> Install New Software..`, put `http://pydev.org/updates` into "Work With",
  search for `PyDev` and install it. Restart IDE.
* Select `Open Perspective` and select `PyDev`.
* Select `Window -> Preferences -> PyDev -> Inperpreters -> Python interpreter`,
  select `New... -> Browse from python/pypy exe`, hit `Apply -> Apply and Close`.
* Select `File -> New -> PyDev project`, select `/hub/channels` folder of this repository.
* Select `PyDev -> Start Debug Server` (Note: That menu item should be present at the debug perspective and it can be enabled 
  in other perspectives through `Window -> Perspective -> Customize perspective -> Tool Bar Visibility -> PyDev debug`).
  If that thing complains, fiddle with `Action Set Availability`.
* Specify the `-d` option to the Hub when running. When run, the hub will
  attempt to join to the pycharm with `pydevd` to port `5678`.
  Note that with `-d` option, the hub is running in a single threaded mode, so
  it only can serve one client connection.
  
First time thing could be slow, so have some patience.

### Debugging the client

* To build with debugging symbols, use `make DEBUG=1`
* The only way to debug the client is to use netlog, a printf-alike function that
sends printed text to UDP port 9468 on the same host as proxy.
* To see these logs on Linux/Mac, simply do `make listen-for-logs` while
on Windows, you'd need to install netcat and do `nc -lLu -w 1 -p 9468`.
