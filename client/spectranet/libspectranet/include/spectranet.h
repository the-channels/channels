#ifndef __SPECTRANET_H__
#define __SPECTRANET_H__

/*
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

 spectranet.h:

 Definitions and function prototypes for miscellaneous Spectranet functions
 not directly related to the socket library.

*/

/* TODO: Move to sys/types.h */
#ifndef in_addr_t
#define in_addr_t	unsigned long
#endif

/* structures */
struct basic_cmd
{
	unsigned char errorcode;
	char *command;
	unsigned char rompage;
	void *function;
};

/* defines */
#define TRAP_NONSENSE	0x0b	/* trap C Nonsense in BASIC */

/* detect (absense of) spectranet hardware */
extern int  __LIB__ __FASTCALL__	spectranet_detect(void); /* returns 0 if hardware present */

/* Control of settings - hardware address, IP address, gateway etc. */
/* Set and get the hardware address */
extern int  __LIB__ __FASTCALL__	sethwaddr(char *hwaddr);
extern void __LIB__ __FASTCALL__	gethwaddr(char *hwaddr);

/* Convert hw address to and from a string */
extern void __LIB__ 			mac2string(char *mac, char *str);
extern void __LIB__			string2mac(char *str, char *mac);
extern void __LIB__ __CALLEE__		mac2string_callee(char *m, char *s);
extern void __LIB__ __CALLEE__		string2mac(char *s, char *m);

/* Configure inet address, netmask, gateway */
extern void __LIB__ __FASTCALL__	ifconfig_inet(in_addr_t *addr);
extern void __LIB__ __FASTCALL__	get_ifconfig_inet(in_addr_t *addr);
extern void __LIB__ __FASTCALL__	ifconfig_netmask(in_addr_t *addr);
extern void __LIB__ __FASTCALL__	get_ifconfig_netmask(in_addr_t *addr);
extern void __LIB__ __FASTCALL__	ifconfig_gw(in_addr_t *addr);
extern void __LIB__ __FASTCALL__	get_ifconfig_gw(in_addr_t *addr);

/* De-configure inet settings (reset to 0.0.0.0 for addr, gw, netmask) */
extern void __LIB__			deconfig();

/* Memory management */
extern void __LIB__ __FASTCALL__	setpagea(unsigned char page);
extern void __LIB__ __FASTCALL__	setpageb(unsigned char page);
extern void __LIB__			pagein();
extern void __LIB__			pageout();

/* Utility functions */
extern void __LIB__			clear42();
extern void __LIB__ __FASTCALL__	putchar42(char ch);
extern void __LIB__			inputstring(char *str, int len);
extern void __LIB__ __CALLEE__		inputstring_callee(char *str, int len);
extern unsigned int __LIB__		rand16();

/* Conversion functions */
extern void __LIB__		long2ipstring(in_addr_t *addr, char *str);
extern int  __LIB__		ipstring2long(char *str, in_addr_t *addr);
extern void __LIB__ __CALLEE__	long2ipstring_callee(in_addr_t *addr, char *str);
extern int  __LIB__ __CALLEE__	ipstring2long_callee(char *str, in_addr_t *addr);

extern int __LIB__ __FASTCALL__	addbasicext(struct basic_cmd *cmd);
extern void __LIB__		statement_end();
extern void __LIB__		exit_success();

/* Make CALLEE the default */
#define mac2string(a,b)			mac2string_callee(a,b)
#define string2mac(a,b)			string2mac_callee(a,b)
#define inputstring(a,b)		inputstring_callee(a,b)
#define long2ipstring(a,b)		long2ipstring_callee(a,b)
#define ipstring2long(a,b)		ipstring2long_callee(a,b)

#endif

