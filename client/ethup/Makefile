# devtools makefile
CC = gcc
ETHUPOBJS = ethup.o
ifeq ($(OS),Windows_NT)
ETHUPLIBS = -lwsock32
CFLAGS = -DWIN32
else
ETHUPLIBS =
CFLAGS = -DUNIX
endif
ETHUPOUT = ethup

ethup:	$(ETHUPOBJS)
	$(CC) $(CFLAGS) -o $(ETHUPOUT) $(ETHUPOBJS) $(ETHUPLIBS)
clean:
	$(RM) *.o $(ETHUPOUT)

