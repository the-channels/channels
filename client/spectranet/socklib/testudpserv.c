#include <stdio.h>
#include <input.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <strings.h>
#include <sockpoll.h>
#include "testsocks.h"

void udpserv_bale(char *msg, int rc);

/* A simple UDP server. */
void testudpserver()
{
	int sockfd, rc, addrsz;
	struct sockaddr_in my_addr;
	struct sockaddr_in their_addr;
	struct pollfd p;
	char sendbuf[128];
	char recvbuf[128];
	unsigned char *dispip;

	sockfd=socket(AF_INET, SOCK_DGRAM, 0);
	if(sockfd < 0)
	{
		udpserv_bale("socket() failed", rc);
		return;
	}

	printf("Socket opened: fd=%d. Binding to port 2000\n", sockfd);

	my_addr.sin_family=AF_INET;
	my_addr.sin_port=2000;
	rc=bind(sockfd, &my_addr, sizeof(my_addr));
	if(rc < 0)
	{
		udpserv_bale("bind() died", rc);
		return;
	}

	printf("Polling...'x' exits\n");
	while(1)
	{
		if(in_Inkey() == 'x')
			break;
		
		rc=pollall(&p);
		if(rc == sockfd)
		{
			printf("POLL: %x\n", rc);
			printf("Data received:\n");
			memset(recvbuf, 0, sizeof(recvbuf));
			rc=recvfrom(sockfd, recvbuf, sizeof(recvbuf), 0,
				&their_addr, &addrsz);
			if(rc < 0)
			{
				udpserv_bale("recv died", rc);
				sockclose(sockfd);
				return;
			}
			dispip=&their_addr.sin_addr.s_addr;
			printf("%d bytes from %d.%d.%d.%d:%ld\n", 
				rc, 
				dispip[0], 
				dispip[1], 
				dispip[2], 
				dispip[3],
				(unsigned long)their_addr.sin_port);
			printf("message: %s\n", recvbuf);

			sprintf(sendbuf, "some data for you\r\n");
			rc=sendto(sockfd, sendbuf, strlen(sendbuf), 0,
				&their_addr, addrsz);
			if(rc < 0)
			{
				udpserv_bale("send died", rc);
				sockclose(sockfd);	
				return;
			}
		}

	}
	sockclose(sockfd);
}

void udpserv_bale(char *msg, int rc)
{
	printf("FAIL: %s - rc = %d\n", msg, rc);
	while(!in_Inkey());
}

