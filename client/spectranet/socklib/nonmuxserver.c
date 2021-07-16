#include <stdio.h>
#include <input.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <strings.h>
#include "testsocks.h"

void nonmux_bale(int rc);

/* Non-multiplexed server: accepts just one connection. */
void testnonmuxedserver()
{
	int connfd, sockfd, rc, addrsz;
	struct sockaddr_in my_addr;
	struct sockaddr_in their_addr;
	unsigned char *addrch;
	char sendbuf[128];
	char recvbuf[128];
	memset(recvbuf,0,sizeof(recvbuf));

	sockfd=socket(AF_INET, SOCK_STREAM, 0);
	if(sockfd < 0)
	{
		nonmux_bale(sockfd);
		return;
	}
	printf("Socket opened: fd=%d\n", sockfd);

	my_addr.sin_family=AF_INET;
	my_addr.sin_port=2000;		/* listen on port 2000 */


	printf("Binding to port 2000\n");	
	rc=bind(sockfd, &my_addr, sizeof(my_addr));
	if(rc < 0)
	{
		nonmux_bale(sockfd);
		return;
	}

	printf("Listening...\n");
	rc=listen(sockfd, 1);
	if(rc < 0)
	{
		nonmux_bale(sockfd);
		return;
	}
	
	printf("Waiting for connection...\n");
	connfd=accept(sockfd, &their_addr, &addrsz);
	if(connfd < 0)
	{
		nonmux_bale(connfd);
		return;
	}

	addrch=(unsigned char *)&their_addr.sin_addr.s_addr;
	printf("Got connection:\nfd  =%d\nAddr=%d.%d.%d.%d\nPort=%ld\nReceiving a message\n", 
		connfd, 
		addrch[0], addrch[1], addrch[2], addrch[3],
		(unsigned long)their_addr.sin_port);

	rc=recv(connfd, recvbuf, sizeof(recvbuf), 0);
	if(rc < 0)
	{
		nonmux_bale(rc);
		return;
	}
	printf("Received a message: %s\nSending a message.\n", recvbuf);

	sprintf(sendbuf, "Hello,world!\r\n");
	rc=send(connfd, sendbuf, strlen(sendbuf), 0);
	if(rc < 0)
	{
		nonmux_bale(rc);
		return;
	}

	printf("Closing accepted socket...\n");
	sockclose(connfd);

	printf("Closing listening socket...\n");
	sockclose(sockfd);

	printf("Done. Press any key.\n");
	while(!in_Inkey());
}

void nonmux_bale(int rc)
{
	printf("Server function died with rc=%d\nPress any key to exit.",rc);
	while(!in_Inkey());
}

