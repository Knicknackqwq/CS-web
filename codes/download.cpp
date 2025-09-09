#pragma comment(lib,"ws2_32.lib")
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <winsock2.h>
#include <WS2tcpip.h>
#include <time.h>

#define TFTP_RRQ   1
#define TFTP_DATA  3
#define TFTP_ACK   4
#define TIMES      3
#define LOG "logs.log"
//没有接受的包，重复请求次数



void send_rrq(SOCKET sockfd, SOCKADDR_IN* servaddr, char* filename,char *mode,FILE*f,char* ip) {
    char buffer[516];
    int a;
    int text_len;
    //构造RRQ报文
    buffer[0] = 0;
    buffer[1] = TFTP_RRQ;
    strcpy(buffer + 2, filename);
    buffer[strlen(filename) + 2] = 0;
    text_len = 3 + strlen(filename);
    strcpy(buffer + text_len, mode);
    text_len += strlen(mode);
    buffer[text_len] = 0;
    a = sendto(sockfd, buffer, text_len + 1, 0, (SOCKADDR*)servaddr, sizeof(*servaddr));
    if (a > 0)
    {
        printf("Request is sent!\n");   //应该加上丢包重传机制（RRQ包没有送到后也应该重传至多RETRY次）
        //计入log日志
        time_t a = time(NULL);
        struct tm* b = localtime(&a);
        char e[30];
        strcpy(e, asctime(b));
        int p;
        p = strlen(e) - 1;
        e[p] = 0;
        fprintf(f, "[%s] DOWNLOAD INFO %s %s Request is sent!\n", e, ip, filename);
    }
    else
    {
        printf("Request Send failed!\n");
        //计入log日志
        time_t a = time(NULL);
        struct tm* b = localtime(&a);
        char e[30];
        strcpy(e, asctime(b));
        int p;
        p = strlen(e) - 1;
        e[p] = 0;
        fprintf(f, "[%s] DOWNLOAD ERROR %s %s Request Send failed!\n", e, ip, filename);
    }
}

void recv_data_and_send_ack(SOCKET sockfd, SOCKADDR_IN* servaddr, char* filename,FILE*file) {
    int times = 0;
    char buffer[516];
    int n;
    bool q = TRUE;
    int len = sizeof(*servaddr);
    FILE* f;
    f = fopen(filename, "wb");
    while (q) 
    {
        if ((n = recvfrom(sockfd, buffer, sizeof(buffer), 0, (SOCKADDR*)servaddr, &len)) > 0)
        {
            //写入f文件
            if (buffer[1] == TFTP_DATA)
            {
                fwrite(buffer + 4, 1, n - 4, f);
                //构造ACK报文,就直接使用返回报文头
                buffer[1] = TFTP_ACK;
                sendto(sockfd, buffer, 4, 0, (SOCKADDR*)servaddr, sizeof(*servaddr));
                if (n < sizeof(buffer))
                    //结束
                    q = FALSE;
                times = 0;//times计数置零
            }
            else
            {
                time_t a = time(NULL);
                struct tm* b = localtime(&a);
                char e[30];
                strcpy(e, asctime(b));
                int p;
                p = strlen(e) - 1;
                e[p] = 0;
                fprintf(file, "[%s] DOWNLOAD ERROR %s\n", e,buffer+4);
                printf("Recv Wrong!\n");
                q = FALSE;
                
            }
        }
        else
        {   
            times++;
            if (times < TIMES)
            {
                sendto(sockfd, buffer, 4, 0, (SOCKADDR*)servaddr, sizeof(*servaddr));
                time_t a = time(NULL);
                struct tm* b = localtime(&a);
                char e[30];
                strcpy(e, asctime(b));
                int p;
                p = strlen(e) - 1;
                e[p] = 0;
                fprintf(file, "[%s] DOWNLOAD INFO Retry\n", e);
                printf("Retry %d\n", times);
            }
            else
            {
                //data报文接受错误
                printf("Time out!\n");
                time_t a = time(NULL);
                struct tm* b = localtime(&a);
                char e[30];
                strcpy(e, asctime(b));
                int p;
                p = strlen(e) - 1;
                e[p] = 0;
                fprintf(file, "[%s] DOWNLOAD ERROR TIMEOUT\n", e);
                fclose(f);
                break;
            }
        }
    }
    fclose(f);
}

int download(char*ip,char*filename,char *mode) 
{
    FILE* f;
    f = fopen(LOG, "a+");
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printf("WSAStartup failed.\n");
        time_t a = time(NULL);
        struct tm* b = localtime(&a);
        char e[30];
        strcpy(e, asctime(b));
        int p;
        p = strlen(e) - 1;
        e[p] = 0;
        fprintf(f, "[%s] DOWNLOAD ERROR %s %s WSAStartup failed.\n",e, ip,filename);
        fclose(f);
        exit(1);
    }

    SOCKET sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    SOCKADDR_IN servaddr;
    ZeroMemory(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(69);
    inet_pton(AF_INET, ip, &servaddr.sin_addr);

    send_rrq(sockfd, &servaddr, filename,mode,f,ip);
    recv_data_and_send_ack(sockfd, &servaddr, filename,f);
    fclose(f);
    closesocket(sockfd);
    WSACleanup();
    return 1;
}
