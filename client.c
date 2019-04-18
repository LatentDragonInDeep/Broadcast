#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>

int main(int argc,char** argv) {
    char* ip = NULL;
    int port = 0;
    char* fileName = NULL;
    //获取本机ip
    for (int i = 1; i < argc; ++i) {
        if(strcmp(argv[i],"-h") == 0) {
            ip = argv[++i];
            continue;
        }
        if(strcmp(argv[i],"-p") == 0) {
            port = atoi(argv[++i]);
            continue;
        }
        if(strcmp(argv[i],"-f") == 0) {
            fileName = argv[++i];
            continue;
        }
    }
    //初始化连接
    int sockFd = socket(AF_INET, SOCK_STREAM|SOCK_NONBLOCK, IPPROTO_TCP);
    if(sockFd == -1) {
        printf("create sock failed");
        exit(1);
    }
    struct sockaddr_in serAddr;
    serAddr.sin_family = AF_INET;
    serAddr.sin_port = htons((u_int16_t)port);
    serAddr.sin_addr.s_addr = inet_addr(ip);
    int connectFd;
    //建立连接
    if((connectFd = connect(sockFd, (struct sockaddr*)&serAddr, sizeof(serAddr)))<0) {
        printf("connect to server failed");
        exit(1);
    }
    //发送要传输的文件路径
    int len = strlen(fileName);
    int hasSend = 0;
    int sended = 0;
    while (hasSend<len) {
        sended = send(connectFd,fileName+hasSend,len-hasSend,0);
        hasSend+=sended;
    }
    //接收文件
    char* recvBuf = malloc(4096);
    int recvFileFd = open("temp.gz",O_CLOEXEC|O_CREAT|O_WRONLY);
    int recved = 0;
    while ((recved = recv(connectFd,recvBuf,4096,0))!=0) {
        write(recvFileFd,recvBuf,recved);
    }
    free(recvBuf);
}