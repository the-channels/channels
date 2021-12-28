#include "netlog.h"
#include <string.h>
#include <netdb.h>
#include "system.h"

static struct sockaddr_in netlog_remoteaddr;
static int netlog_socket = -1;

int netlog_init(int port)
{
    if (netlog_socket >= 0)
    {
        return netlog_socket;
    }

    struct hostent *he;
    int rc;

    he = gethostbyname("127.0.0.1");
    if(!he) return -1;

    netlog_remoteaddr.sin_family= AF_INET;
    netlog_remoteaddr.sin_port = htons(port);
    netlog_remoteaddr.sin_addr.s_addr = (in_addr_t)he->h_addr;

    netlog_socket = socket(AF_INET, SOCK_DGRAM, 0);
    return netlog_socket;
}

void netlog(const char* data)
{
    if (netlog_socket == -1) return;

    sendto(netlog_socket, (void*)data, strlen(data), 0,
        (const struct sockaddr *)&netlog_remoteaddr, sizeof(netlog_remoteaddr));
}

extern void netlog_1(const char *a)
{
    char buff[128];
    strcpy(buff, a);
    strcat(buff, "\n");
    netlog(buff);
}

extern void netlog_2(const char *a, const char *b)
{
    char buff[128];
    strcpy(buff, a);
    strcat(buff, b);
    strcat(buff, "\n");
    netlog(buff);
}

extern void netlog_3(const char *a, const char *b, const char *c)
{
    char buff[128];
    strcpy(buff, a);
    strcat(buff, b);
    strcat(buff, ", ");
    strcat(buff, c);
    strcat(buff, "\n");
    netlog(buff);
}
