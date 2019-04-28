#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <pthread.h>
#include <zlib.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/sendfile.h>

int main() {
    //获取本机ip，初始化连接
    char * selfIp = NULL;
    char hostName[256];
    gethostname(hostName,256);
    struct hostent *hostEnt = gethostbyname(hostName);
    struct in_addr addr;
    for(int i= 0;;i++)
    {
        char *p = hostEnt->h_addr_list[i];
        if(p == NULL) {
            break;
        }
        memcpy(&addr.s_addr,p,hostEnt->h_length);
        selfIp = inet_ntoa(addr);
    }
    struct sockaddr_in servaddr;
    servaddr.sin_family = PF_INET;
    servaddr.sin_addr.s_addr = htonl(atoi(selfIp));
    servaddr.sin_port = htons(12345);
    int socket_fd = socket(AF_INET,SOCK_DGRAM,0);
    int yes = 1;
    setsockopt(socket_fd,SOL_SOCKET,SO_BROADCAST,&yes, sizeof(yes));
    int len = sizeof(servaddr);
    while (1) {
        time_t now = time(NULL);
        char sendBuf[4];
        char * dataLenPtr = (char*)&now;
        for (int i = 0; i < 4; ++i) {
            sendBuf[i] = dataLenPtr[3-i];
        }
        sendto(socket_fd,sendBuf,4,0,(struct sockaddr*)&servaddr,&len);
        sleep(1);
    }
}
