# Makefile to build the Spectranet C library.

SRCGEN = ../scripts/makesources.pl
PLIBNAME = libspectranet
NPLIBNAME = libspectranet_np
PASMFILES = libspectranet.lst
NPASMFILES = libspectranet_np.lst
TESTOUT = testlibsn.bin
BASOUT = testbas.bin
CFLAGS = +zx -vn -I./include -O2 -preserve
COBJS = testlibsn.o
BASOBJS = testbasext.o
CLIBS = -lndos -llibspectranet

all:	plib nplib

plib:	$(PASMFILES)
	$(SRCGEN)
	$(ASSEMBLER) $(ASMFLAGS) -x$(PLIBNAME) @$(PASMFILES)

nplib:	$(NPASMFILES)
	$(SRCGEN)
	$(ASSEMBLER) $(ASMFLAGS) -x$(NPLIBNAME) @$(NPASMFILES)

test:	$(COBJS)
	$(CC) $(CFLAGS) -o $(TESTOUT) $(COBJS) $(CLIBS)

basext:	$(BASOBJS)
	$(CC) $(CFLAGS) -o $(BASOUT) $(BASOBJS) $(CLIBS)
	
	
install:
	$(CP) *.lib $(DESTLIBS)/
	$(CP) -r include/* $(DESTINCLUDE)

include ../make.inc
