#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc,char** argv) {
    //初始化连接
    int sockFd = socket(AF_INET, SOCK_DGRAM|SOCK_NONBLOCK, IPPROTO_TCP);
    if(sockFd == -1) {
        printf("create sock failed");
        exit(1);
    }
    struct sockaddr_in clientAddr;
    struct sockaddr_in servAddr;
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_port = htons((u_int16_t)12345);
    clientAddr.sin_addr.s_addr = inet_addr(INADDR_ANY);
    if (bind(sockFd, (struct sockaddr *)&clientAddr, sizeof(clientAddr)) == -1) {
        printf("Bind error.");
        exit(1);
    }
    char recvBuf[4];
    int len = sizeof(servAddr);
    while (1) {
        int recved = recvfrom(sockFd,recvBuf,4,0,(struct sockaddr *)&servAddr,&len);
        if (recved < 0){
            printf("recvfrom error\n");
            exit(1);
        }
        unsigned now = 0;
        for (int i = 0; i <4; ++i) {
            now |= (((unsigned)recvBuf[i])<<(24-i*8));
        }
        printf("current tiem is %d",now);
    }
}