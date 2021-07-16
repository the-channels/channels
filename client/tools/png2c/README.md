png2c.py
========

This is a simple tool to convert a 24 or 32 bit PNG image to a
include file suitable to be used with Z88DK and the ZX Spectrum
version of the SP1 sprite library that is distributed with it.

`png2c.py` generates:

 * a C array with the sprites encoded (eg, tiles)
 * some useful defines to call `sp1_TileEntry` (eg, TILES_BASE)
 * an optional array to use with `sp1_PrintString` (eg, ptiles)

See `-h` flag for a list of options.

For example:
```
$ png2c.py image.png > image.h
```

This will use the default variable name "tiles", so it can be
used with:

```C
	uchar i;
    uchar *pt = tiles;

    for (i = 0; i < TILES_LEN; i++, pt += 8)
        sp1_TileEntry(TILES_BASE + i, pt);
```

`TILES_BASE` starts in 128 by default (can be changed with `-b`
flag).

Then the print string can be used:

```C
	// print the string starting in 0,0
    sp1_SetPrintPos(&ps0, 0, 0);
    sp1_PrintString(&ps0, ptiles);
```


Requirements
------------

The tool requires `PIL` library (or `Pillow`) and `argparse` if
you're using Python 2 < 2.7.

It should work both in Python 2 and Python 3.


License
-------

The tool is distribute under MIT license (read the begining of
the script for a copy of the license).

