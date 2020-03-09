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
#include <Pthread.h>
#include "Pthread.h"
#include "Net.h"
#include "Uart.h"
#include "../Struct2Json/ConfigFile.h"


static void Net2UartPthread(void *param);
static void Uart2NetPthread(void *param);
static void UDP2UartPthread(void *param);
static void Uart2UDPPthread(void *param);


/**
 * @breif TCP客户端转串口，外接TCP服务器端和串口
 * @param ipAddress IP地址，格式："192.168.1.1"
 * @return 成功0或失败-1
 */
int TCP_Client2Uart(char *ipAddress)
{
	int uartfd, sockfd;		//串口和网卡设备的文件描述符
	int fdArray[2] = {0};	//存放网络socket和串口的描述符
	pthread_t net2UartPid, uart2NetPid;		//网口与串口转换的线程ID号
	int ret = -1;
	void *status;

	sockfd = TCP_NetConnect(g_ConfigFile[LOCAL_PORT_NUM].configData, ipAddress);		//连接网口
	uartfd = UartInit(g_ConfigFile[UART_DEVICE_NAME_NUM].configString, g_ConfigFile[UART_BANDRATE_NUM].configData);	//打开串口

	fdArray[0] = sockfd;
	fdArray[1] = uartfd;

	/* 常见网口与串口透传的线程 */
	ret = pthread_create(&net2UartPid, NULL, (void*)Net2UartPthread, fdArray);
	if(0 != ret)
	{
		printf("pthread Net2Uart create error!\n");
		goto TCP_CLIENT_CLOSE;
	}

	ret = pthread_create(&uart2NetPid, NULL, (void*)Uart2NetPthread, fdArray);
	if(0 != ret)
	{
		printf("pthread Uart2Net create error!\n");
		goto TCP_CLIENT_CLOSE;
	}

TCP_CLIENT_CLOSE:
	pthread_join(net2UartPid, &status);
	pthread_join(uart2NetPid, &status);
	close(sockfd);
	close(uartfd);
	printf("TCP CLIENT CLOSE!\n");

	return 0;
}


/**
 * @breif TCP服务端转串口，外接TCP客户端和串口
 * @return 成功0或失败-1
 */
int TCP_Server2Uart(void)
{
	int uartfd, sockfd, clientfd;
	int fdArray[2] = {0};	//存放网络socket和串口的描述符
	pthread_t net2UartPid, uart2NetPid;		//网口与串口转换的线程ID号
	int ret = -1;
	void *status;

	sockfd = TCP_NetListen(g_ConfigFile[LOCAL_PORT_NUM].configData);
	uartfd = UartInit(g_ConfigFile[UART_DEVICE_NAME_NUM].configString, g_ConfigFile[UART_BANDRATE_NUM].configData);	//打开串口
	clientfd = TCP_NetAccept(sockfd);

	fdArray[0] = clientfd;
	fdArray[1] = uartfd;

	/* 常见网口与串口透传的线程 */
	ret = pthread_create(&net2UartPid, NULL, (void*)Net2UartPthread, fdArray);
	if(0 != ret)
	{
		printf("pthread Net2Uart create error!\n");
		goto TCP_SERVER_CLOSE;
	}

	ret = pthread_create(&uart2NetPid, NULL, (void*)Uart2NetPthread, fdArray);
	if(0 != ret)
	{
		printf("pthread Uart2Net create error!\n");
		goto TCP_SERVER_CLOSE;
	}

TCP_SERVER_CLOSE:
	pthread_join(net2UartPid, &status);
	pthread_join(uart2NetPid, &status);
	close(clientfd);
	close(sockfd);
	close(uartfd);
	printf("TCP SERVER CLOSE!\n");

	return 0;
}


/**
 * @breif UDP转串口，外接UDP和串口
 * @return 成功0或失败-1
 */
int UDP2Uart(void)
{
	int uartfd, sockfd;
	int fdArray[3] = {0};	//存放网络socket和串口的描述符
	pthread_t udp2UartPid, uart2UdpPid;		//网口与串口转换的线程ID号
	struct sockaddr_in remoteAddr;
	int ret = -1;
	void *status;

	sockfd = UDP_NetConnect(g_ConfigFile[LOCAL_PORT_NUM].configData);		//连接网口
	uartfd = UartInit(g_ConfigFile[UART_DEVICE_NAME_NUM].configString, g_ConfigFile[UART_BANDRATE_NUM].configData);	//打开串口
	SetRemoteAddress(&remoteAddr);

	fdArray[0] = sockfd;
	fdArray[1] = uartfd;
	fdArray[2] = (int)&remoteAddr;

	/* 常见网口与串口透传的线程 */
	ret = pthread_create(&udp2UartPid, NULL, (void*)UDP2UartPthread, fdArray);
	if(0 != ret)
	{
		printf("pthread UDP2Uart create error!\n");
		goto UDP_CLOSE;
	}

	ret = pthread_create(&uart2UdpPid, NULL, (void*)Uart2UDPPthread, fdArray);
	if(0 != ret)
	{
		printf("pthread Uart2UDP create error!\n");
		goto UDP_CLOSE;
	}

UDP_CLOSE:
	pthread_join(udp2UartPid, &status);
	pthread_join(uart2UdpPid, &status);
	close(sockfd);
	close(uartfd);
	printf("UDP CLOSE!\n");

	return 0;
}


/**
 * @breif 网口接口转发到串口的线程程序
 * @param param 整型数组，第一个数存放网络socket描述符，第二个数存放串口描述符
 * @return 设备文件描述符或-1
 */
static void Net2UartPthread(void *param)
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
            if(write(uartfd, bufReceive, recvBytes) == -1)
            {
                printf("write error！\r\n");
                continue;
            }
            memset(bufReceive, 0, MAX_DATA_SIZE);
        }
    } 
}


/**
 * @breif 串口接口转发到网口的线程程序
 * @param param 整型数组，第一个数存放网络socket描述符，第二个数存放串口描述符
 * @return 设备文件描述符或-1
 */
static void Uart2NetPthread(void *param)
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
            if (send(sockfd, bufSend, nread, 0) == -1)
            {
                printf("send error！\r\n");
                continue;
            }
            memset(bufSend, 0, MAX_DATA_SIZE);
        }
    }
}


/**
 * @breif 网口UDP转发到串口的线程程序
 * @param param 整型数组，第一个数存放网络socket描述符，第二个数存放串口描述符
 * @return 设备文件描述符或-1
 */
static void UDP2UartPthread(void *param)
{
    int uartfd, sockfd;
    int recvBytes;
    struct sockaddr_in *remoteAddr = NULL;
	char bufReceive[MAX_DATA_SIZE] = {0};
    socklen_t sinSize;

    sockfd = ((int*)param)[0];
    uartfd = ((int*)param)[1];
    remoteAddr = (struct sockaddr_in *)((int*)param)[2];

    while(1)
    {
        recvBytes = recvfrom(sockfd, bufReceive, sizeof(bufReceive)-1, 0, (struct sockaddr *)remoteAddr, &sinSize);
        if(recvBytes > 0)
        {
            if (write(uartfd, bufReceive, recvBytes) == -1)
            {
                printf("write error！\r\n");
                continue;
            }
            memset(bufReceive, 0, MAX_DATA_SIZE);
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
    struct sockaddr_in *remoteAddr = NULL;

    sockfd = ((int*)param)[0];
    uartfd = ((int*)param)[1];
    remoteAddr = (struct sockaddr_in *)((int*)param)[2];

    while(1)
	{
		nread = read(uartfd, bufSend, sizeof(bufSend));
		if (nread > 0)
		{
			if (sendto(sockfd, bufSend, nread, 0, (struct sockaddr *)remoteAddr, sizeof(struct sockaddr_in)) == -1)
			{
				printf("send error！\r\n");
				continue;
			}
			memset(bufSend, 0, MAX_DATA_SIZE);
		}
	}
}
