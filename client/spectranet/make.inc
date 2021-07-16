# make.inc file for z88dk libraries
CC = zcc
LIBLINKER = z80asm
ASSEMBLER = z80asm
RM = rm -f
LIBLDFLAGS = -d
ASMFLAGS = -d
CP = cp

.PHONY:	clean

clean:
	$(RM) *.o *.lib *.def *_p.asm *_np.asm *.sym

