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
#include "minizip/zip.h"

typedef struct {
    int clientFd;
} task;

const int maxRetry = 3;

void zipFileOrDir(const char *filePath, zipFile *zf) {
    struct stat fileStat;
    stat(filePath,&fileStat);
    if(S_ISDIR(fileStat.st_mode)) {
        DIR *pDir = opendir(filePath);
        if (!pDir) {
            printf("open dir failed");
            return;
        }
        struct dirent *pDirEnt = NULL;
        while (pDirEnt = readdir(pDir)) {
            if (pDirEnt == NULL) {
                printf("read dir failed");
                return;
            }
            if (strcmp(pDirEnt->d_name, ".") == 0 || strcmp(pDirEnt->d_name, "..") == 0) {
                continue;
            }
            char newFilePath[strlen(filePath) + strlen(pDirEnt->d_name) + 1];
            strcpy(newFilePath, filePath);
            strcat(newFilePath + strlen(filePath), "/");
            strcat(newFilePath + strlen(filePath) + 1, pDirEnt->d_name);
            zipFileOrDir(newFilePath, zf);
        }
    } else {
        zip_fileinfo zipFileinfo = {0};
        int ret = zipOpenNewFileInZip(zf,filePath,&zipFileinfo,NULL,0,NULL,0,NULL,0,0);
        if(ret!=ZIP_OK) {
            printf("zip file %s failed",filePath);
            return;
        }
    }
}

void exec(void * tsk) {
    //接收文件名
    task* t = tsk;
    char* recvBuf = malloc(4096);
    int recved = 0;
    int hasRecv = 0;
    while ((recved = recv(t->clientFd,recvBuf+hasRecv,4096-hasRecv,0))!=0) {
        hasRecv+=recved;
    }
    recvBuf[hasRecv] = '\0';
    //调用压缩方法压缩
    zipFile zf = zipOpen("temp.gz",APPEND_STATUS_CREATE);
    zipFileOrDir(hasRecv, &zf);
    zipClose(zf,NULL);
    int zippedFile = open("temp.gz",O_RDONLY);
    //发送到客户端，关闭套接字
    char sendBuf[4096];
    char seqRecvBuf[4];
    int readed = 0;
    while (1) {
        next:
        readed = read(zippedFile,sendBuf,4096);
        if(readed == 0) {
            break;
        }
        send(t->clientFd,sendBuf,readed,0);
        int retry = 0;
        while (retry<maxRetry) {
            recv(t->clientFd, seqRecvBuf, 3, 0);
            seqRecvBuf[3] = '\0';
            if(strcmp("ACK",seqRecvBuf)==0) {
                memset(seqRecvBuf,'\0',4);
                goto next;
            }
        }
        printf("has not recieved client's ack!");
        exit(1);
    }
    close(t->clientFd);
    close(zippedFile);
    free(recvBuf);
}

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
    listen(socket_fd,10);
    while (1) {
        struct sockaddr_in clientAddr;
        socklen_t len = sizeof(clientAddr);
        task t;
        //接收客户端连接，新开线程处理
        t.clientFd = accept(socket_fd, (struct sockaddr*)&clientAddr,&len);
        pthread_t id = rand();
        pthread_create(&id,NULL,exec,&t);
    }
}
