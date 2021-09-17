/* The MIT License
 *
 * Copyright (c) 2010 Dylan Smith
 * Other contributors: Edward Cree
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * TNFS daemon datagram handler
 *
 * */

#include <dirent.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

#include "session.h"
#include "log.h"
#include "tnfs.h"
#include "directory.h"
#include "datagram.h"
#include "errortable.h"
#include "bsdcompat.h"

/* List of sessions */
Session *slist[MAX_CLIENTS];
char *DEFAULT_ROOT = "/";

void tnfs_init()
{
	int i;
	for (i = 0; i < MAX_CLIENTS; i++)
		slist[i] = NULL;

#ifdef BSD
	/* initialize prng */
	srandomdev();
#endif
}

/* TODO: This is the "simple" TNFS server that won't do authentication.
 * So it ignores the user/pass fields of the tnfs_mount request. It is
 * intended at some stage that there is a server that can use the underlying
 * OS to perform authentication (and uses the authorization features of
 * the OS) but that's a job for later since it needs different stuff
 * for each OS supported. The intention at this stage is to make a simple
 * daemon that can share a directory tree with an 8 bit machine */
int tnfs_mount(Header *hdr, unsigned char *buf, int bufsz)
{
	int mplen;
	int sindex;
	Session *s;
	unsigned char repbuf[4];
	char *cliroot;
	uint16_t recycledSid = 0;

#ifdef DEBUG
	TNFSMSGLOG(hdr, "TNFS_MOUNT");
#endif
	/* Mount packet looks like:
	 * Header + mountpoint + user + pass.
	 * Check that there is at least one null terminator so we
	 * won't create an invalid string ever*/
	if (*(buf + bufsz - 1) != 0)
	{
		TNFSMSGLOG(hdr, "Unterminated MOUNT operation");
		return -1;
	}

	/* deallocate old, if necessary */
	if ((s = tnfs_findsession_ipaddr(hdr->ipaddr, &sindex)) != NULL)
	{
		recycledSid = s->sid;
		TNFSMSGLOG(hdr, "Freeing existing session");
		tnfs_freesession(s, sindex);
	}

	/* allocate a new session */
	s = tnfs_allocsession(&sindex, recycledSid);
	if (!s)
	{
		TNFSMSGLOG(hdr, "Failed to allocate session");
		return -1;
	}

	/* find out how much to allocate for the mount point */
	cliroot = ((char *)buf) + 2;
	mplen = strlen(cliroot);
	if (mplen < 1)
	{
		mplen = 1;
		cliroot = DEFAULT_ROOT;
	}

	if ((s->root = (char *)malloc(mplen + 1)) != NULL)
	{
		strlcpy((char *)s->root, cliroot, mplen + 1);
	}
	else
	{
		TNFSMSGLOG(hdr, "Failed to allocate session root");
		return -1;
	}

	s->last_contact = time(NULL);
	s->ipaddr = hdr->ipaddr;

	/* set up the proto version/timeout in the reply buffer */
	repbuf[0] = PROTOVERSION_LSB;
	repbuf[1] = PROTOVERSION_MSB;
	repbuf[2] = TIMEOUT_LSB;
	repbuf[3] = TIMEOUT_MSB;

	/* verify that the root path is valid */
	if (validate_dir(s, "") == 0)
	{
		/* all OK - send a response */
		hdr->status = 0;
		hdr->sid = s->sid;
		tnfs_send(s, hdr, repbuf, 4);
#ifdef DEBUG
		TNFSMSGLOG(hdr, "Mounted %s OK, SID=%x", s->root, s->sid);
#endif
#ifdef USAGELOG
		USGLOG(hdr, "Session started at: %s", s->root);
		s->lastpath[0] = 0;
#endif
	}
	else
	{
		hdr->status = tnfs_error(errno);
		hdr->sid = 0;

		/* only need to send the version in an error reply */
		tnfs_send(s, hdr, repbuf, 2);

		/* free the session */
		tnfs_freesession(s, sindex);
#ifdef DEBUG
		TNFSMSGLOG(hdr, "Failed to mount %s", s->root);
#endif
	}

	return 0;
}

/* Unmount a filesystem */
void tnfs_umount(Header *hdr, Session *s, int sindex)
{
#ifdef DEBUG
	TNFSMSGLOG(hdr, "Unmounting");
#endif
#ifdef USAGELOG
	USGLOG(hdr, "Unmounting session");
#endif
	/* the response must be sent before we deallocate the
	 * session */
	hdr->status = 0;
	tnfs_send(s, hdr, NULL, 0);

	tnfs_freesession(s, sindex);
}

/* Create a new session */
Session *tnfs_allocsession(int *sindex, uint16_t withSid)
{
	Session *s;

	LOG("Allocating new session for 0x%02x\n", withSid);
	for (*sindex = 0; (*sindex) < MAX_CLIENTS; (*sindex)++)
	{
		if (slist[*sindex] == NULL)
		{
			/* free session entry has been found */
			s = (Session *)malloc(sizeof(Session));
			if (s)
			{
				memset(s, 0, sizeof(Session));
				if (withSid > 0)
				{
					s->sid = withSid;
				}
				else
				{
					s->sid = tnfs_newsid();
				}
				LOG("Allocated new session for 0x%02x\n", s->sid);
				slist[*sindex] = s;
			}
			return s;
		}
	}

	/* reached MAX_CLIENTS */
	return NULL;
}

/* Free a session */
void tnfs_freesession(Session *s, int sindex)
{
	LOG("Freeing session ID index %d\n", sindex);	
	int i;
	if (s->root)
		free(s->root);

	/* close open fds, directories etc. */
	for (i = 0; i < MAX_FD_PER_CONN; i++)
	{
		if (s->fd[i])
			close(s->fd[i]);
	}
	for (i = 0; i < MAX_DHND_PER_CONN; i++)
	{
		if (s->dhandles[i].handle)
			closedir(s->dhandles[i].handle);
		dirlist_free(s->dhandles[i].entry_list);
		s->dhandles[i].entry_count = 0;
	}
	free(s);
	slist[sindex] = NULL;
}

/* Find a session by its SID. Return NULL if not found */
Session *tnfs_findsession_sid(uint16_t sid, int *sindex)
{
	int i;
	Session *s;
	for (i = 0; i < MAX_CLIENTS; i++)
	{
		if (slist[i])
		{
			s = slist[i];
			if (s->sid == sid)
			{
				*sindex = i;
				return s;
			}
		}
	}
	return NULL;
}

/* Find a session by IP address. Return NULL if not found.
	Up to MAX_CLIENTS_PER_IP are allowed from the same IP address,
	so we'll return NULL if we haven't yet reached that number
	even if there are existing matching connections.
	If we reach  MAX_CLIENTS_PER_IP then the first entry in the
	session list with a matching IP is returned.
*/
Session *tnfs_findsession_ipaddr(in_addr_t ipaddr, int *sindex)
{
	int i;
	Session *s;
	time_t currenttime;

	Session *first_match_sess;
	int first_match_idx;

	int count = 0;

#ifdef DEBUG
	unsigned char *ip = (unsigned char *)&ipaddr;
	LOG("Looking for existing sessions with IP %d.%d.%d.%d\n", ip[0], ip[1], ip[2], ip[3]);
#endif

	currenttime = time(NULL);

	for (i = 0; i < MAX_CLIENTS; i++)
	{
		if (slist[i])
		{
			s = slist[i];

			/* Remove expired sessions while we're looking at them all */
			if(SESSION_TIMEOUT > 0 && (currenttime - s->last_contact >= SESSION_TIMEOUT))
			{
				LOG("Deleting expired session 0x%02x\n", s->sid);
				tnfs_freesession(s, i);
				continue;
			}
			
			if (s->ipaddr == ipaddr)
			{
				// If we've reached the max for this IP, return the first match
				if ((count + 1) >= MAX_CLIENTS_PER_IP)
				{
					LOG("Found we already %d sessions for this IP - returning oldest entry\n", MAX_CLIENTS_PER_IP);
					*sindex = first_match_idx;
					return first_match_sess;
				}
				if (count == 0)
				{
					first_match_idx = i;
					first_match_sess = s;
				}
				count++;
			}
		}
	}
	return NULL;
}

/* Creates a new unique SID */
uint16_t tnfs_newsid()
{
	uint16_t newsid;
	int sindex;
	int tries;

	for (tries = 0; tries < 255; tries++)
	{
#ifdef BSD
		newsid = random() & 0xFFFF;
#else
		newsid = rand() & 0xFFFF;
#endif
		if (!tnfs_findsession_sid(newsid, &sindex))
			return newsid;
	}
	die("Tried to find a new SID 256 times. (Broken PRNG)");
	return 0;
}
