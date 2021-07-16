ROOT_DIR:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))

JUST_PRINT := $(findstring n,$(MAKEFLAGS))
TARGET := sccz80
NETWORK_ENGINE:="spectranet"

COMMON_LIBS = -llibsocket -llibspectranet
INCLUDE_PATH = /usr/local/share/z88dk/include
BIN_NAME = channels__.bin

channels_OUT = bin/channels
ifneq (,$(JUST_PRINT))
PHONY_OBJS := yes
CC = gcc
CFLAGS += -I$(ROOT_DIR)/include/spectranet -I$(INCLUDE_PATH) -I$(ROOT_DIR)/include -I$(ROOT_DIR)/gui -I$(ROOT_DIR)/netlog -I$(ROOT_DIR)/../proto -D__FASTCALL__="" \
	-DCHANNELS_PROTO_CLIENT="1" -D__CALLEE__="" -D__SCCZ80="1" -D__SPECTRUM="1" -D__Z80="1" -D__z88dk_callee="" -D__z88dk_fastcall="" -D__preserves_regs="" -D__LIB__="" -D__no_z88dk_declspec="1"
else
CC = zcc
CFLAGS += -I$(ROOT_DIR)/include/spectranet -L$(ROOT_DIR)/libs -I$(INCLUDE_PATH) -SO3 -I$(ROOT_DIR)/include -I$(ROOT_DIR)/gui -I$(ROOT_DIR)/netlog -I$(ROOT_DIR)/../proto -DCHANNELS_PROTO_CLIENT="1"
ZXFLAGS = +zx
channels_CFLAGS = -m -zorg=25000 -pragma-redirect=CRT_FONT_64=_fake_CRT_FONT_64 -pragma-include:zpragma.inc -L/usr/local/share/z88dk/lib/clibs $(COMPILER_LIBS) $(COMMON_LIBS) -create-app $(SUBTYPE)
endif

ifeq ($(NETWORK_ENGINE), "spectranet")
CFLAGS += -D__SPECTRANET="1"
endif

SOURCES := $(wildcard src/*.c gui/*.c netlog/*.c ../proto/channels_proto.c ../proto/proto_objects.c)
OBJECTS = $(SOURCES:.c=.o)

all: bin/channels.tap

libs:
	@mkdir -p libs

include/spectranet:
	@mkdir -p include/spectranet

libs/libsocket.lib: libs include/spectranet
	make DESTLIBS=$(ROOT_DIR)/libs DESTINCLUDE=$(ROOT_DIR)/include/spectranet -C spectranet/socklib plib install

libs/libspectranet.lib: libs include/spectranet
	make DESTLIBS=$(ROOT_DIR)/libs DESTINCLUDE=$(ROOT_DIR)/include/spectranet -C spectranet/libspectranet plib install

spectranet-libraries: libs/libsocket.lib libs/libspectranet.lib

bin/channels.tap: spectranet-libraries bin $(OBJECTS)
	$(CC) $(ZXFLAGS) $(CFLAGS) -o $(channels_OUT) $(channels_CFLAGS) $(OBJECTS)

bin/channels__.bin: spectranet-libraries bin $(OBJECTS)
	$(CC) $(ZXFLAGS) $(CFLAGS) -o $(channels_OUT) $(channels_CFLAGS) $(OBJECTS) -subtype=bin

%.o: %.c
	$(CC) $(ZXFLAGS) $(CFLAGS) -c -o $@ $<

bin:
	@mkdir -p bin

channels-bin: bin/channels__.bin

boot-zx: channels-bin
	cp boot/boot.zx bin/boot.zx

deploy: channels-bin boot-zx
	cp bin/channels__.bin bin/channels
ifeq ($(CHANNELS_DEPLOY_TO),)
	@echo "Please define CHANNELS_DEPLOY_TO"
else
	scp bin/channels__.bin $(CHANNELS_DEPLOY_TO):~/tnfsd/channels
	scp bin/boot.zx $(CHANNELS_DEPLOY_TO):~/tnfsd/boot.zx
endif

tnfsd: channels-bin
	tnfsd $(ROOT_DIR)/bin

run: bin/channels.tap
	open bin/channels.tap

listen-for-logs:
	@nc -kluvw 0 9468

get-size:
	@cat bin/channels.map | sed -n "s/^\\([a-zA-Z0-9_]*\\).*= .\([A-Z0-9]*\) ; addr.*/\2,\1/p" | sort | python3 tools/symbol_sizes.py

clean:
	make -C spectranet/socklib clean
	make -C spectranet/libspectranet clean
	@rm -f bin/channels*
	@rm -f src/*.o
	@rm -rf include/spectranet
	@rm -f libs/*.lib
	@rm -f gui/*.o
	@rm -f log/*.o
	@rm -f channels_proto/*.o

ifeq ($(PHONY_OBJS),yes)
.PHONY: $(SOURCES)
endif

.PHONY: channels clean run deploy listen-for-logs get-size
