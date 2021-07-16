#include <stdio.h>
#include <input.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <strings.h>
#include <sockpoll.h>
#include "testsocks.h"

void muxed_bale(int rc);

/* Non-multiplexed server: accepts just one connection. */
void testmuxedserver()
{
	int connfd, sockfd, polled, rc, addrsz;
	struct sockaddr_in my_addr;
	struct sockaddr_in their_addr;
	struct pollfd p;
	char sendbuf[128];
	char recvbuf[128];

	sockfd=socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0)
	{
		muxed_bale(sockfd);
		return;
	}
	printf("Socket opened: fd=%d\n", sockfd);

	my_addr.sin_family=AF_INET;
	my_addr.sin_port=2000;		/* listen on port 2000 */


	printf("Binding to port 2000\n");	
	rc=bind(sockfd, &my_addr, sizeof(my_addr));
	if(rc < 0)
	{
		muxed_bale(sockfd);
		return;
	}

	printf("Listening...\n");
	rc=listen(sockfd, 1);
	if(rc < 0)
	{
		muxed_bale(sockfd);
		return;
	}
	
	printf("Polling...\n");
	while(1)
	{
		if(in_Inkey())	/* stop on keypress */
			break;

		polled=pollall(&p);
		if(polled == sockfd)
		{
			printf("Accepting new connection...\n");
			connfd=accept(sockfd, NULL, NULL);
		}

		else if(polled > 0)
		{
			if(p.revents & POLLHUP)
			{
				printf("Remote host disconnected.\n");
				sockclose(polled);
			}
			else
			{
				printf("Got some data:\n");
				memset(recvbuf, 0, sizeof(recvbuf));
				rc=recv(polled, recvbuf, sizeof(recvbuf), 0);
				if(rc < 0)
				{
					muxed_bale(rc);
					return;
				}
				printf("bytes = %d data = %s\n", rc, recvbuf);

				sprintf(sendbuf, "You are fd %d\n", polled);
				rc=send(polled, sendbuf, strlen(sendbuf), 0);
				if(rc < 0)
				{
					muxed_bale(rc);
					return;
				}
			}
		}
	}
	sockclose(sockfd);
}

void muxed_bale(int rc)
{
	printf("Server function died with rc=%d\nPress any key to exit.",rc);
	while(!in_Inkey());
}

