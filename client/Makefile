ROOT_DIR:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))

JUST_PRINT := $(findstring n,$(MAKEFLAGS))
TARGET := sccz80
ARCH := zx

PROTO_PATH = $(ROOT_DIR)/../proto
CHANNELS_LIBS = -lgui
BIN_NAME = channels__.bin
LIBS_SUFFIX = .lib

channels_OUT = bin/channels

ifeq ($(ARCH),zx)

ifneq (,$(JUST_PRINT))
	PHONY_OBJS := yes
	CC = gcc
	LIB_LD = ar
else
	LIB_LD = zcc
	CC = zcc
	LIB_LDFLAGS += +zx -x -o
	ASMFLAGS = +zx
	CFLAGS += +zx
	CHANNELS_CFLAGS_PREFIX = +zx
	CHANNELS_CFLAGS = -m -zorg=25000 -L$(ROOT_DIR)/libs -pragma-redirect=CRT_FONT_64=_fake_CRT_FONT_64 -pragma-include:zpragma.inc \
	 -L/usr/local/share/z88dk/lib/clibs $(COMPILER_LIBS) -llibsocket_np -llibspectranet_np $(CHANNELS_LIBS) -create-app $(SUBTYPE)
	GUI_CFLAGS = -L$(ROOT_DIR)/libs -L/usr/local/share/z88dk/lib/clibs
	CHANNELS_INSTALLER_CFLAGS = -m -L/usr/local/share/z88dk/lib/clibs -llibspectranet_np -lndos
endif

INCLUDE_PATH = -I/usr/local/share/z88dk/include -I/usr/local/share/z88dk/include/arch/zx
ARCH_TARGETS += spectranet-libs
CHANNELS_TARGETS += channels-zx-bin channels-zx-tap
CFLAGS += -D__SPECTRUM="1" -I$(ROOT_DIR)/include/spectranet $(INCLUDE_PATH)
ARCH_C_SOURCES = $(wildcard arch/zx/*.c)
ARCH_ASM_SOURCES = $(wildcard arch/zx/*.asm)

else
CFLAGS += -D__CALLEE__="" -D__SCCZ80="1" -D__Z80="1" -D__naked="" -D__z88dk_callee="" -D__z88dk_fastcall="" -D__stdc="" -D__preserves_regs="" -D__LIB__="" -D__no_z88dk_declspec="1"
endif

ifeq ($(ARCH),desktop)
CFLAGS += -D__DESKTOP="1"
CHANNELS_TARGETS += channels-desktop-bin
CC = gcc
LIB_LD = ar
LIB_LDFLAGS = rvs
ARCH_C_SOURCES = $(wildcard arch/desktop/*.c)
CHANNELS_CFLAGS = -L$(ROOT_DIR)/libs $(CHANNELS_LIBS)
LIBS_SUFFIX = .a
LIBS_PREFIX = lib
endif

ifneq (,$(JUST_PRINT))
CFLAGS += -I$(ROOT_DIR)/include -I$(ROOT_DIR)/src -I$(ROOT_DIR)/netlog -I$(PROTO_PATH) -D__FASTCALL__="" \
	-DCHANNELS_PROTO_CLIENT="1" -D__CALLEE__="" -D__SCCZ80="1" -D__Z80="1" -D__naked="" -D__z88dk_callee="" -D__z88dk_fastcall="" -D__stdc="" -D__preserves_regs="" -D__LIB__="" -D__no_z88dk_declspec="1"
else
CFLAGS += $(DFLAGS) -I$(ROOT_DIR)/src -O3 -I$(ROOT_DIR)/include -I$(ROOT_DIR)/netlog -I$(PROTO_PATH) -DCHANNELS_PROTO_CLIENT="1"
endif

ifeq ($(DEBUG), 1)
ifeq ($(ARCH),zx)
CFLAGS += -debug
else
CFLAGS += -g -O0
endif
else
CFLAGS += -DNDEBUG="1"
endif

ARCH_OBJECTS = $(ARCH_C_SOURCES:.c=.o) $(ARCH_ASM_SOURCES:.asm=.o)

SOURCES := $(wildcard src/*.c netlog/*.c $(PROTO_PATH)/channels_proto.c $(PROTO_PATH)/proto_objects.c)
OBJECTS = $(SOURCES:.c=.o)

GUI_LIB_SOURCES := $(wildcard gui/*.c)
GUI_LIB_OBJECTS = $(GUI_LIB_SOURCES:.c=.o)

all: channels

libs/$(LIBS_PREFIX)gui$(LIBS_SUFFIX): $(GUI_LIB_OBJECTS)
	$(LIB_LD) $(LIB_LDFLAGS) $@ $(GUI_CFLAGS) $(GUI_LIB_OBJECTS)

gui-lib: libs/$(LIBS_PREFIX)gui$(LIBS_SUFFIX)

gui-lib-clean:
	@rm -f gui/*.o

include/spectranet:
	@mkdir -p include/spectranet

libs/libsocket_np.lib: include/spectranet
	make DESTLIBS=$(ROOT_DIR)/libs DESTINCLUDE=$(ROOT_DIR)/include/spectranet SRCGEN=$(ROOT_DIR)/spectranet/scripts/makesources.pl -C spectranet/socklib nplib install

libs/libspectranet_np.lib: include/spectranet
	make DESTLIBS=$(ROOT_DIR)/libs DESTINCLUDE=$(ROOT_DIR)/include/spectranet SRCGEN=$(ROOT_DIR)/spectranet/scripts/makesources.pl -C spectranet/libspectranet nplib install

spectranet-libs: libs/libsocket_np.lib libs/libspectranet_np.lib

bin/channels.tap: $(ARCH_TARGETS) gui-lib bin $(OBJECTS) $(ARCH_OBJECTS)
	$(CC) $(CHANNELS_CFLAGS_PREFIX) -o $(channels_OUT) $(OBJECTS) $(ARCH_OBJECTS) $(CHANNELS_CFLAGS)
	@cat bin/channels.map | sed -n "s/^\\([a-zA-Z0-9_]*\\).*= .\([A-Z0-9]*\) ; addr.*/\2,\1/p" | sort > bin/channels.map.sorted

bin/channels__.bin: $(ARCH_TARGETS) gui-lib bin $(OBJECTS) $(ARCH_OBJECTS)
	$(CC) $(CHANNELS_CFLAGS_PREFIX) -o $(channels_OUT) $(OBJECTS) $(ARCH_OBJECTS) $(CHANNELS_CFLAGS) -subtype=bin

bin/channels: $(ARCH_TARGETS) gui-lib bin $(OBJECTS) $(ARCH_OBJECTS)
	$(CC) $(CHANNELS_CFLAGS_PREFIX) -o $(channels_OUT) $(OBJECTS) $(ARCH_OBJECTS) $(CHANNELS_CFLAGS)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.o: %.asm
	$(CC) $(ASMFLAGS) -c -o $@ $<

bin:
	@mkdir -p bin

channels: $(CHANNELS_TARGETS)

channels-zx-bin: bin/channels__.bin
channels-zx-tap: bin/channels.tap
channels-desktop-bin: bin/channels

boot-zx: channels-zx-bin
	cp boot/boot.zx bin/boot.zx

run: bin/channels.tap
	open bin/channels.tap

listen-for-logs:
	@nc -kluvw 0 9468

get-size:
	@cat bin/channels.map | sed -n "s/^\\([a-zA-Z0-9_]*\\).*= .\([A-Z0-9]*\) ;.*/\2,\1/p" | sort | python3 tools/symbol_sizes.py

clean: gui-lib-clean
	make -C spectranet/socklib clean
	make -C spectranet/libspectranet clean
	@rm -f bin/channels*
	@rm -f src/*.o
	@rm -f src/*.o.asm
	@rm -f src/*.c.sym
	@rm -rf include/spectranet
	@rm -f libs/*.lib
	@rm -f libs/*.a
	@rm -f netlog/*.o
	@rm -f netlog/*.o.asm
	@rm -f netlog/*.c.sym
	@rm -f $(PROTO_PATH)/*.o
	@rm -f log/*.o
	@rm -f log/*.o.asm
	@rm -f log/*.c.sym
	@rm -f channels_proto/*.o
	@rm -f channels_proto/*.c.sym
	@rm -f arch/*/*.o

ifeq ($(PHONY_OBJS),yes)
.PHONY: $(SOURCES)
.PHONY: $(GUI_LIB_SOURCES)
.PHONY: $(ARCH_C_SOURCES)
.PHONY: $(ARCH_ASM_SOURCES)
endif

.PHONY: channels modules modules-clean channels-installer-clean clean libs run deploy listen-for-logs get-size
