ROOT_DIR:=$(shell dirname $(realpath $(firstword $(MAKEFILE_LIST))))
JUST_PRINT := $(findstring n,$(MAKEFLAGS))

ifneq (,$(JUST_PRINT))
PHONY_OBJS := yes
CC = gcc
CFLAGS += -I$(INCLUDE_PATH) -D__FASTCALL__="" \
	-D__CALLEE__="" -D__SCCZ80="1" -D__SPECTRUM="1" -D__Z80="1" -D__naked="" -D__z88dk_callee="" -D__z88dk_fastcall="" -D__preserves_regs="" -D__LIB__="" -D__no_z88dk_declspec="1"
else
CC = zcc
CFLAGS += -I$(INCLUDE_PATH) -O3
ZXFLAGS = +zx
FZX_CFLAGS = -L/usr/local/share/z88dk/lib/clibs
endif

all: fzx

%.o: %.asm
	$(CC) $(ZXFLAGS) $(CFLAGS) -c -o $@ $<

fzx: $(FZX_OBJECTS)
	$(CC) $(ZXFLAGS) $(CFLAGS) -x -o $@ $(FZX_CFLAGS) $(FZX_OBJECTS)

clean:
	@rm -f c/*.o
	@rm -f z80/*.o
	@rm -f zx_c/*.o
	@rm -f zx_z80/*.o
	@rm -f *.lib

ifeq ($(PHONY_OBJS),yes)
.PHONY: $(FZX_SOURCES)
endif

.PHONY: clean
