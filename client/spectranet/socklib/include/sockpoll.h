#ifndef __SOCKPOLL_H__
#define __SOCKPOLL_H__
/*
 * sockpoll.h
 *
 * Socket polling routines.
 *
 * The file is named 'sockpoll.h' rather than 'poll.h' to differentiate it
 * from the POSIX poll() functions. The functions here can only poll sockets.
 *
 * Three functions are provided:
 * pollfd - Polls a single socket file descriptor.
 * pollall - Polls all open socket file descriptors.
 * pollarray - Polls an array of file descriptors.
 *
 * The poll functions are sort of the INKEY$ of sockets, and return
 * immediately whether a file descriptor is ready or not. A ready state
 * is indicated for the following conditions:
 *  - a listening socket gets connected
 *  - a socket receives some data
 *  - a SOCK_STREAM socket gets disconnected by the remote host
 * The pollall and pollarray functions take a struct pollfd (which is the
 * same as the POSIX structure). The pollfd function simply returns
 * an event bitfield.
 */

/* Bitfield mask values for event/revent members */
#define POLLCON		1	/* remote host connected (not POSIX) */
#define POLLHUP		2	/* foreign host hung up */
#define POLLIN		4	/* data available to read */
#define POLLNVAL	128	/* error occurred */

struct pollfd
{
	int fd;			/* file descriptor to poll */
	unsigned char events;	/* events to poll for */
	unsigned char revents;	/* returned events */
};

/* functions */
/* pollfd polls a single file descriptor and returns the event in the same
 * way that revent is set for the multiple socket poll functions */
extern unsigned char __LIB__ __FASTCALL__ poll_fd(int sockfd);

/* pollall polls all open sockets. When it finds one that needs action,
 * it fills the passed pollfd structure, and also returns the file descriptor.
 * If none are ready, it returns zero and the pollfd struct is unmodified */
extern int __LIB__ __FASTCALL__ pollall(struct pollfd *p); 

#endif
