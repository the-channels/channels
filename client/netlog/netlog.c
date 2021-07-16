#include "netlog.h"
#include <string.h>
#include <netdb.h>
#include "system.h"

static struct sockaddr_in netlog_remoteaddr;
static int netlog_socket = -1;

int netlog_init(const char* reporting_address, int port)
{
    if (netlog_socket >= 0)
    {
        return netlog_socket;
    }

    struct hostent *he;
    int rc;

    he = gethostbyname((char*)reporting_address);
    if(!he) return -1;

    netlog_remoteaddr.sin_family= AF_INET;
    netlog_remoteaddr.sin_port = htons(port);
    netlog_remoteaddr.sin_addr.s_addr = he->h_addr;

    netlog_socket = socket(AF_INET, SOCK_DGRAM, 0);
    return netlog_socket;
}

static char netlog_buffer_[128];
char* netlog_buffer = netlog_buffer_;

void netlog_str(const char* data)
{
    if (netlog_socket == -1) return;

    sendto(netlog_socket, (void*)data, strlen(data), 0, &netlog_remoteaddr, sizeof(netlog_remoteaddr));
}
