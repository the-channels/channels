/*
;The MIT License
;
;Copyright (c) 2008 Dylan Smith
; Other contributors: Miguel Angel Rodríguez Jódar
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

  A simple "send file by TCP" program.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>

#ifdef UNIX
#include <sys/socket.h>
#include <netdb.h>
#else
#include <windows.h>
#define SHUT_RDWR	2
#endif

int main(int argc, char **argv);

int main(int argc, char **argv)
{
	FILE *stream;
	struct sockaddr_in	remoteaddr;
	struct hostent		*he;
	int sockfd;
	int bytes;
	int sent;
	unsigned int addr=32768;	/* default start address */
	struct stat filestat;
	char buf[1025];
#ifdef WIN32
WSADATA wsaData;
#endif
	

	if(argc < 3 || argc > 4)
	{
		printf("Usage: %s <host> <file> [startaddr]\n", argv[0]);
		return(255);
	}

	if(argc == 4)
	{
		addr=strtol(argv[3], (char **)NULL, 10);
		if(errno)
		{
			perror("strtol");
			return(255);
		}
		else if(addr > 65535 || addr < 0)
		{
			fprintf(stderr, "Start address must be < 65535\n");
			return(255);
		}
	}

#ifdef WIN32
	if(WSAStartup(MAKEWORD(2,0), &wsaData) != 0)
   {
		fprintf(stderr, "Winsock failed to initialize\n");
		return(255);
   }
#endif
	

	he=gethostbyname(argv[1]);
	if(!he)
	{
		perror("gethostbyname");
		return(255);
	}

	stream=fopen(argv[2], "rb");
	if(!stream)
	{
		perror("fopen");
		return(255);
	}

	if(stat(argv[2], &filestat) < 0)
	{
		perror("stat");
		return(255);
	}

	if(filestat.st_size > 65536 || filestat.st_size < 1)
	{
		printf("File size is out of range\n");
		return(255);
	}

	sockfd=socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0)
	{
		perror("socket");
		return(255);
	}

	printf("Connecting...\n");
	memset(&remoteaddr, 0, sizeof(remoteaddr));
	remoteaddr.sin_family=AF_INET;
	remoteaddr.sin_port=htons(2000);
	memcpy(&(remoteaddr.sin_addr), he->h_addr, he->h_length);
	if(connect(sockfd, (struct sockaddr *)&remoteaddr, sizeof(remoteaddr))
		< 0)
	{
		perror("connect");
		return(255);
	}

	printf("Sending ");

	/* portable way of making up the little-endian start and size
 	   header */
	buf[0]=(unsigned char)(addr % 256);
	buf[1]=(unsigned char)(addr / 256);
	buf[2]=(unsigned char)(filestat.st_size % 256);
	buf[3]=(unsigned char)(filestat.st_size / 256);
	sent=send(sockfd, buf, 4, 0);
	if(sent != 4)
	{
		printf("Oops. Failed to send size block.\n");
		return(255);
	}

	while((bytes=fread(buf, 1, 1024, stream)) > 0)
	{
		printf(".");
		while((sent=send(sockfd, buf, bytes, 0)) < bytes)
		{
			bytes-=sent;
		}
		/* spectranet is a bit too slow */
		usleep(50000);
	}
	printf("\n");

	shutdown(sockfd, SHUT_RDWR);
	close(sockfd);
	fclose(stream);
	return 0;
}

