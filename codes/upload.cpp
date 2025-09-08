#pragma comment(lib,"ws2_32.lib")
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>

#define BUFFER_SIZE 512
#define WRQ 2
#define DATA 3
#define ACK 4
#define ERROR 5
#define TIMEOUT 5
#define RETRY 3

#define RECV_LOOP_COUNT 100
int recv_intime(SOCKET sockfd, char* buf, int buf_n, sockaddr* addr, int* addrlen)
{
    //正常收到ACK
    struct timeval tv;
    fd_set readfds;
    int i = 0;
    unsigned int n = 0;

    FD_ZERO(&readfds);
    FD_SET(sockfd, &readfds);
    tv.tv_sec = 3;
    tv.tv_usec = 5000;
    select(sockfd + 1, &readfds, NULL, NULL, &tv);
    if (FD_ISSET(sockfd, &readfds))
    {
        if ((n = recvfrom(sockfd, buf, buf_n, 0, addr, addrlen)) >= 0)
        {
            return n;
        }
    }

    return -1;
}

void send_wrq(SOCKET sockfd, char* filename, char* mode, SOCKADDR_IN serverAddr) {
    char request[516];
    request[0] = 0;
    request[1] = WRQ;
    strcpy(request + 2, filename);
    strcpy(request + 2 + strlen(filename) + 1, mode);
    sendto(sockfd, request, 2 + strlen(filename) + 1 + strlen(mode) + 1, 0, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
}

void send_data(SOCKET sockfd, char* data, int dataLen, int blocks, SOCKADDR_IN serverAddr) {
    char dataPacket[516];
    dataPacket[0] = 0;
    dataPacket[1] = DATA;
    dataPacket[2] = blocks >> 8;
    dataPacket[3] = blocks & 0xFF;
    memcpy(dataPacket + 4, data, dataLen);
    sendto(sockfd, dataPacket, dataLen + 4, 0, (SOCKADDR*)&serverAddr, sizeof(serverAddr));
}


int upload(char* server_ip, char* filename, char* mode)
{
    SOCKET sockfd;
    SOCKADDR_IN server_addr;
    int server_addr_len;
    static int server_port;
    FILE* fp;
    char buffer[BUFFER_SIZE];
    int len;
    unsigned short block = 0;

    // Check the number of command line arguments (should be 4)

    /*scanf("%s", server_ip);*/
    server_port = atoi("69");
    //scanf("%s", filename);


    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        printf("WSAStartup");
        exit(-1);
    }

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd == INVALID_SOCKET)
    {
        printf("socket");
        exit(-1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    //初始化
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(server_port);
    server_addr.sin_addr.s_addr = inet_addr(server_ip);
    server_addr_len = sizeof(server_addr);

    
    const int block_size = 512;
    unsigned int blocks = 1;

    send_wrq(sockfd, filename, mode, server_addr);
    char ackPacket[4];
    int addrLen = sizeof(server_addr);
    for (int cnt = 0; recv_intime(sockfd, ackPacket, sizeof(ackPacket), (struct sockaddr*)&server_addr, &addrLen) <= 0 && cnt < RETRY; cnt++)
    {
        printf("TIMEOUT. TRY%d\n", cnt+1);
        send_wrq(sockfd, filename, mode, server_addr);
    }

    if (ackPacket[1] != ACK) {
        fprintf(stderr, "Error: Invalid ACK received.\n");
        return 1;
    }
    char data[block_size];
    FILE* file = fopen(filename, "rb");

    if (file == NULL) {
        perror("Error opening file");
        return -1;
    }

    struct timeval timeout;
    timeout.tv_sec = 1;  // 设置超时时间为1秒
    timeout.tv_usec = 0;

    while (1) {
        size_t bytesRead = fread(data, 1, block_size, file);
        size_t last_bytesRead;
        if (bytesRead <= 0) {   //有问题，如果文件大小刚好能被512字节整除，则会多发一个空包，不能直接break
            break;
        }

        int retry = 0;
        while (retry < 3) {  // 最多重传3次
            send_data(sockfd, data, bytesRead, blocks, server_addr);

            char ackPacket[4];
            int addrLen = sizeof(server_addr);

            if (recv_intime(sockfd, ackPacket, sizeof(ackPacket), (struct sockaddr*)&server_addr, &addrLen) > 0);
            {
                if (ackPacket[1] == ACK) {
                    int received_block_number = ((u_short(ackPacket[2]) % 256) << 8) | (u_short(ackPacket[3]) % 256);
                    if (received_block_number == blocks) {
                        break;  // 收到正确的ACK，继续下一块数据
                    }
                }
                else {
                    // 超时，重传数据包
                    retry++;
                    printf("TIMEOUT. TRY%d\n",retry);
                }
            }
        }

        if (retry == 3) {
            printf("Maximum retries reached. Transfer aborted.\n");
            exit(1);
        }

        if (blocks == 65535)
            blocks = 0;
        else
            blocks++;

    }
    fclose(file);
    closesocket(sockfd);

    printf("File %s uploaded successfully.\n", filename);

    WSACleanup();

    return 0;
}


