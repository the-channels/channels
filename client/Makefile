ROOT_DIR:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))

JUST_PRINT := $(findstring n,$(MAKEFLAGS))
TARGET := sccz80
NETWORK_ENGINE:="spectranet"

PROTO_PATH = $(ROOT_DIR)/../proto
CHANNELS_LIBS = -llibsocket_np -llibspectranet_np -lfzx -lgui
INCLUDE_PATH = /usr/local/share/z88dk/include
BIN_NAME = channels__.bin

ifeq ($(DEBUG), 1)
CFLAGS += -debug
else
CFLAGS += -DNDEBUG="1"
endif

channels_OUT = bin/channels
ifneq (,$(JUST_PRINT))
PHONY_OBJS := yes
CC = gcc
CFLAGS += -I$(ROOT_DIR)/include/spectranet -I$(INCLUDE_PATH) -I$(ROOT_DIR)/include -I$(ROOT_DIR)/src -I$(ROOT_DIR)/netlog -I$(PROTO_PATH) -D__FASTCALL__="" \
	-DCHANNELS_PROTO_CLIENT="1" -D__CALLEE__="" -D__SCCZ80="1" -D__SPECTRUM="1" -D__Z80="1" -D__naked="" -D__z88dk_callee="" -D__z88dk_fastcall="" -D__stdc="" -D__preserves_regs="" -D__LIB__="" -D__no_z88dk_declspec="1"
else
CC = zcc
CFLAGS += $(DFLAGS) -I$(ROOT_DIR)/include/spectranet -L$(ROOT_DIR)/libs -I$(ROOT_DIR)/src -I$(INCLUDE_PATH) -O3 -I$(ROOT_DIR)/include -I$(ROOT_DIR)/netlog -I$(PROTO_PATH) -DCHANNELS_PROTO_CLIENT="1"
ASMFLAGS =
ZXFLAGS = +zx
channels_CFLAGS = -m -zorg=25000 -pragma-redirect=CRT_FONT_64=_fake_CRT_FONT_64 -pragma-include:zpragma.inc \
 -L/usr/local/share/z88dk/lib/clibs $(COMPILER_LIBS) $(CHANNELS_LIBS) -create-app $(SUBTYPE)
FZX_CFLAGS = -L/usr/local/share/z88dk/lib/clibs
GUI_CFLAGS = -L/usr/local/share/z88dk/lib/clibs
MODULES_CFLAGS = -L/usr/local/share/z88dk/lib/clibs
CHANNELS_INSTALLER_CFLAGS = -m -L/usr/local/share/z88dk/lib/clibs -llibspectranet_np -lndos
endif

ifeq ($(NETWORK_ENGINE), "spectranet")
CFLAGS += -D__SPECTRANET="1"
endif

SOURCES := $(wildcard src/*.c netlog/*.c $(PROTO_PATH)/channels_proto.c $(PROTO_PATH)/proto_objects.c)
OBJECTS = $(SOURCES:.c=.o)

FZX_LIB_SOURCES := $(wildcard fzx/c/*.asm fzx/z80/*.asm fzx/zx_c/*.asm fzx/zx_z80/*.asm fzx/font/*.asm)
FZX_LIB_OBJECTS = $(FZX_LIB_SOURCES:.asm=.o)

GUI_LIB_SOURCES := $(wildcard gui/*.c)
GUI_LIB_OBJECTS = $(GUI_LIB_SOURCES:.c=.o)

all: channels

libs/fzx.lib: $(FZX_LIB_OBJECTS)
	$(CC) $(ZXFLAGS) $(CFLAGS) -x -o $@ $(FZX_CFLAGS) $(FZX_LIB_OBJECTS)

fzx-lib: libs/fzx.lib

fzx-lib-clean:
	@rm -f fzx/c/*.o
	@rm -f fzx/z80/*.o
	@rm -f fzx/zx_c/*.o
	@rm -f fzx/zx_z80/*.o
	@rm -f fzx/font/*.o

libs/gui.lib: $(GUI_LIB_OBJECTS)
	$(CC) $(ZXFLAGS) $(CFLAGS) -x -o $@ $(GUI_CFLAGS) $(GUI_LIB_OBJECTS)

gui-lib: libs/gui.lib

gui-lib-clean:
	@rm -f gui/*.o

include/spectranet:
	@mkdir -p include/spectranet

libs/libsocket_np.lib: include/spectranet
	make DESTLIBS=$(ROOT_DIR)/libs DESTINCLUDE=$(ROOT_DIR)/include/spectranet SRCGEN=$(ROOT_DIR)/spectranet/scripts/makesources.pl -C spectranet/socklib nplib install

libs/libspectranet_np.lib: include/spectranet
	make DESTLIBS=$(ROOT_DIR)/libs DESTINCLUDE=$(ROOT_DIR)/include/spectranet SRCGEN=$(ROOT_DIR)/spectranet/scripts/makesources.pl -C spectranet/libspectranet nplib install

spectranet-libs: libs/libsocket_np.lib libs/libspectranet_np.lib

bin/channels.tap: spectranet-libs fzx-lib gui-lib bin $(OBJECTS)
	$(CC) $(ZXFLAGS) $(CFLAGS) -o $(channels_OUT) $(channels_CFLAGS) $(OBJECTS)
	@cat bin/channels.map | sed -n "s/^\\([a-zA-Z0-9_]*\\).*= .\([A-Z0-9]*\) ; addr.*/\2,\1/p" | sort > bin/channels.map.sorted

bin/channels__.bin: spectranet-libs fzx-lib gui-lib bin $(OBJECTS)
	$(CC) $(ZXFLAGS) $(CFLAGS) -o $(channels_OUT) $(channels_CFLAGS) $(OBJECTS) -subtype=bin

%.o: %.c
	$(CC) $(ZXFLAGS) $(CFLAGS) -c -o $@ $<

%.o: %.asm
	$(CC) $(ZXFLAGS) $(ASMFLAGS) -c -o $@ $<

bin:
	@mkdir -p bin

channels: channels-bin channels-tap
channels-bin: bin/channels__.bin
channels-tap: bin/channels.tap

boot-zx: channels-bin
	cp boot/boot.zx bin/boot.zx

run: bin/channels.tap
	open bin/channels.tap

listen-for-logs:
	@nc -kluvw 0 9468

get-size:
	@cat bin/channels.map | sed -n "s/^\\([a-zA-Z0-9_]*\\).*= .\([A-Z0-9]*\) ; addr.*/\2,\1/p" | sort | python3 tools/symbol_sizes.py

clean: fzx-lib-clean gui-lib-clean
	make -C spectranet/socklib clean
	make -C spectranet/libspectranet clean
	@rm -f bin/channels*
	@rm -f src/*.o
	@rm -f src/*.o.asm
	@rm -f src/*.c.sym
	@rm -rf include/spectranet
	@rm -f libs/*.lib
	@rm -f netlog/*.o
	@rm -f netlog/*.o.asm
	@rm -f netlog/*.c.sym
	@rm -f $(PROTO_PATH)/*.o
	@rm -f log/*.o
	@rm -f log/*.o.asm
	@rm -f log/*.c.sym
	@rm -f channels_proto/*.o
	@rm -f channels_proto/*.c.sym

ifeq ($(PHONY_OBJS),yes)
.PHONY: $(SOURCES)
endif

.PHONY: fzx-lib-clean fzx-lib-clean
.PHONY: channels modules modules-clean channels-installer-clean clean libs run deploy listen-for-logs get-size
