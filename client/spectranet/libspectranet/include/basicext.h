#ifndef __BASICEXT_H__
#define __BASICEXT_H__

/* Functions for calling the ZX ROM when writing BASIC extensions.
;The MIT License
;
;Copyright (c) 2008 Dylan Smith
;
;Permission is hereby granted, free of charge, to any person obtaining a copy
;of this software and associated documentation files (the "Software"), to deal
;in the Software without restriction, including without limitation the rights
;to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
;copies of the Software, and to permit persons to whom the Software is
;furnished to do so, subject to the following conditions:
;
;The above copyright notice and this permission notice shall be included in
;all copies or substantial portions of the Software.
;
;THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
;IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
;FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
;AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
;LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
;OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
;THE SOFTWARE.
*/

/*--------------------------------------------------*/
/* Retrieve values from BASIC using ZX ROM routines */

/* Get a 16 bit int from the current position in the BASIC line */
extern unsigned int __LIB__ find_int2();

/* Get an 8 bit int */
extern unsigned int __LIB__ find_int1();

/* Get a string from the BASIC line. This function makes a C string, and
 * returns the string length */
extern unsigned int __LIB__ string_fetch(char *buf, int bufsz);

/* Syntax checking routines */
/*--------------------------*/

/* Checks for the presence of 1 number */
extern unsigned char __LIB__ expectNum();

/* Checks for the presence of 2 numbers, comma separated */
extern unsigned char __LIB__ expect2Num();

/* Check for a single string expression - uses EXPT-EXP */
extern unsigned char __LIB__ expectStringExp();

/* Routines that may be needed either in syntax checking or at run time */
/*----------------------------------------------------------------------*/
extern unsigned char __LIB__ next_char();

/* CALLEE linkage */
extern unsigned int __LIB__ __CALLEE__ string_fetch_callee(char *buf, int bufsz);

/* Make CALLEE the default */
#define string_fetch(a,b)	string_fetch_callee(a,b)

#endif

