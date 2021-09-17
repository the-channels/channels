Building the tnfs daemon
========================

Use the command 'make OS=osname'.
The following is valid:

make OS=LINUX       All versions of Linux
make OS=BSD         Use this also for macOS. Has been tested on OpenBSD.
make OS=Windows_NT  All versions of Windows (with MinGW)

If using Windows with cygwin, it's probable you'll need to use
make OS=LINUX instead since Cygwin looks more like Linux than Windows.
You might have to remove the -DENABLE_CHROOT from the Makefile, though
since I'm not sure chrooting is supported under Cygwin.

To make a debug version, use 'make OS=osname DEBUG=yes'. This will add
some extra debugging messages and add the -g flag to the compilation 
options.

To output basic usage log on stdout, use 'make OS=osname USAGELOG=yes'.

