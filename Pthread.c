/**
 * @file Pthread.c
 * @brief 线程相关的程序文件
 * @copyright Copyright (c) 2020 Beijing SOJO Electric CO., LTD.
 * @company  SOJO
 * @date 2020.02.17
 *
 * @author Lei.L
 * @version ver 1.0
 */


#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "Pthread.h"
#include "Net.h"


/**
 * @breif 网口接口转发到串口的线程程序
 * @param param 整型数组，第一个数存放网络socket描述符，第二个数存放串口描述符
 * @return 设备文件描述符或-1
 */
void Net2UartPthread(void *param)
{
    int sockfd, uartfd;
    int recvBytes = 0;      //接收到的字节数
    char bufReceive[MAX_DATA_SIZE] = {0};       //接收缓存区

    sockfd = ((int*)param)[0];
    uartfd = ((int*)param)[1];
    while(1)
    {
        recvBytes = recv(sockfd, bufReceive, MAX_DATA_SIZE, 0);
        if(recvBytes > 0)
        {
            if(write(uartfd, bufReceive, strlen(bufReceive)) == -1)
            {
                printf("write error！\r\n");
                continue;
            }
            printf("socket receivr, usart send: %s\r\n", bufReceive);
        }
        sleep(1);
    } 
}


/**
 * @breif 串口接口转发到网口的线程程序
 * @param param 整型数组，第一个数存放网络socket描述符，第二个数存放串口描述符
 * @return 设备文件描述符或-1
 */
void Uart2NetPthread(void *param)
{
    int sockfd, uartfd;
    int nread = 0;
    char bufSend[MAX_DATA_SIZE] = {0};

    sockfd = ((int*)param)[0];
    uartfd = ((int*)param)[1];
    while(1)
    {
        nread = read(uartfd, bufSend, sizeof(bufSend));
        if (nread > 0)
        {
            if (send(sockfd, bufSend, strlen(bufSend), 0) == -1)
            {
                printf("send error！\r\n");
                continue;
            }
            printf("usart receivr, socket send: %s\r\n", bufSend);
        }
        sleep(1);
    }
}


/**
 * @breif 网口UDP转发到串口的线程程序
 * @param param 整型数组，第一个数存放网络socket描述符，第二个数存放串口描述符
 * @return 设备文件描述符或-1
 */
void UDP2UartPthread(void *param)
{
    int uartfd, sockfd;
    int recvBytes;
    struct sockaddr_in remoteAddr;
	char bufReceive[MAX_DATA_SIZE] = {0};
    socklen_t sinSize;

    sockfd = ((int*)param)[0];
    uartfd = ((int*)param)[1];

    remoteAddr.sin_family = AF_INET;
	remoteAddr.sin_port = htons(SERVER_PORT);
	remoteAddr.sin_addr.s_addr = inet_addr(REMOTE_IP_ADDRESS);

    while(1)
    {
        recvBytes = recvfrom(sockfd, bufReceive, sizeof(bufReceive)-1, 0, (struct sockaddr *)&remoteAddr, &sinSize);
        if(recvBytes > 0)
        {
            if (write(uartfd, bufReceive, strlen(bufReceive)) == -1)
            {
                printf("write error！\r\n");
                continue;
            }
            printf("sockfd receivr, usart send: %s\r\n", bufReceive);
        sleep(1);
        }
    }
    
}


/**
 * @breif 串口接口转发到网口UDP的线程程序
 * @param param 整型数组，第一个数存放网络socket描述符，第二个数存放串口描述符
 * @return 设备文件描述符或-1
 */
void Uart2UDPPthread(void *param)
{
    int uartfd, sockfd;
    char bufSend[MAX_DATA_SIZE] = {0};
	int nread = 0;
    struct sockaddr_in remoteAddr;

    sockfd = ((int*)param)[0];
    uartfd = ((int*)param)[1];

    remoteAddr.sin_family = AF_INET;
	remoteAddr.sin_port = htons(SERVER_PORT);
	remoteAddr.sin_addr.s_addr = inet_addr(REMOTE_IP_ADDRESS);

    while(1)
	{
		nread = read(uartfd, bufSend, sizeof(bufSend));
		if (nread > 0)
		{
			if (sendto(sockfd, bufSend, strlen(bufSend), 0, (struct sockaddr *)&remoteAddr, sizeof(remoteAddr)) == -1)
			{
				printf("send error！\r\n");
				continue;
			}
			printf("usart receivr, sockfd send: %s\r\n", bufSend);
		}
        sleep(1);
	}
}
