FROM alpine:3
RUN apk update && apk add --no-cache cmake git build-base python3 python3-dev py3-setuptools \
    build-base libxml2 libxml2-dev m4 perl

RUN mkdir /channels && mkdir /channels/hub/ && mkdir /channels/tnfsd/  \
    && mkdir /channels/client/ && mkdir /channels/hub/bin && mkdir /channels/hub/bin/cache

# z88dk
RUN git clone --recursive https://github.com/z88dk/z88dk.git /z88dk
WORKDIR /z88dk
RUN chmod 777 build.sh
RUN ./build.sh -p zx
RUN make install

ADD proto /channels/proto

# client
ADD client/ /channels/client/
WORKDIR /channels/client
RUN make PROTO_PATH=/channels/proto
RUN cp bin/boot.zx /channels/tnfsd && cp bin/channels /channels/tnfsd
# read only
RUN chmod 0444 -R /channels/tnfsd

# hub
ADD hub/configurations /channels/hub/configurations
ADD hub/tnfsd /channels/hub/tnfsd
ADD hub/pybind11 /channels/hub/pybind11
ADD hub/channels /channels/hub/channels
ADD hub/img2spec /channels/hub/img2spec
ADD hub/src /channels/hub/src
ADD hub/CMakeLists.txt /channels/hub
RUN mkdir /build
WORKDIR /channels/hub/tnfsd
RUN make OS=LINUX TARGET_DIR=/channels/tnfsd
WORKDIR /build
RUN cmake -DCMAKE_BUILD_TYPE=Release -S /channels/hub -B /build
RUN make

WORKDIR /channels/hub/bin
ADD docker/start.sh /start.sh
EXPOSE 9493
EXPOSE 16384/udp
ENTRYPOINT ["/bin/sh", "/start.sh"]