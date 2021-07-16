#ifndef __NETDB_H__
#define __NETDB_H__

/*
 * netdb.h
 *
 * Routines for querying DNS.
 *
 * 2008-05-08 Dylan Smith
 */

/* netdb structures: note they are a little bit(!) divergent from the
 * typical BSD library.
 * Note, like BSD, the struct hostent is just statically allocated
 * and gets overwritten on every call. At present, only one h_addr
 * gets returned, but this may change in the future so the structure
 * is defined in a mostly BSD compatible way */
struct hostent
{
	char *hostname;			/* official hostname */
	unsigned long *h_addr_list;	/* list of addresses, null terminated */
};

/* netdb functions */
extern struct hostent __LIB__ __FASTCALL__ *gethostbyname(char *name);

extern int h_errno;
#define h_addr h_addr_list[0]
#endif

