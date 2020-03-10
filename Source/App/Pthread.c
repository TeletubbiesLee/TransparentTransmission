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



static void Client2UartPthread(void *param);
static void Server2UartPthread(void *param);
static void Uart2ClientPthread(void *param);
static void Uart2ServerPthread(void *param);
static void UDP2UartPthread(void *param);
static void Uart2UDPPthread(void *param);


/**
 * @breif TCP客户端转串口，外接TCP服务器端和串口
 * @return 成功0或失败-1
 */
int TCP_Client2Uart(void)
{
	int uartfd, sockfd;		//串口和网卡设备的文件描述符
	int fdArray[2] = {0};	//存放网络socket和串口的描述符
	pthread_t net2UartPid, uart2NetPid;		//网口与串口转换的线程ID号
	int ret = -1;
	void *status;

	/* 建立连接服务器端的socket，打开串口 */
	sockfd = TCP_NetConnect(g_ConfigFile[REMOTE_IP_ADDRESS_NUM].configString, g_ConfigFile[REMOTE_PORT_NUM].configData);
	uartfd = UartInit(g_ConfigFile[UART_DEVICE_NAME_NUM].configString, g_ConfigFile[UART_BANDRATE_NUM].configData);

	/* 将串口文件描述符以及socket号的指针作为参数，传给透传线程 */
	fdArray[0] = uartfd;
	fdArray[1] = (int)&sockfd;

	/* 网口与串口透传的线程 */
	ret = pthread_create(&net2UartPid, NULL, (void*)Client2UartPthread, fdArray);
	if(0 != ret)
	{
		printf("pthread Net2Uart create error!\n");
		goto TCP_CLIENT_CLOSE;
	}

	ret = pthread_create(&uart2NetPid, NULL, (void*)Uart2ClientPthread, fdArray);
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

	return 0;
}


/**
 * @breif TCP服务端转串口，外接TCP客户端和串口
 * @return 成功0或失败-1
 */
int TCP_Server2Uart(void)
{
	int uartfd, sockfd, clientfd;
	int fdArray[3] = {0};	//存放网络socket和串口的描述符
	pthread_t net2UartPid, uart2NetPid;		//网口与串口转换的线程ID号
	int ret = -1;
	void *status;

	/* 侦听客户端并且连接，建立socket，打开串口 */
	sockfd = TCP_NetListen(g_ConfigFile[LOCAL_PORT_NUM].configData);
	uartfd = UartInit(g_ConfigFile[UART_DEVICE_NAME_NUM].configString, g_ConfigFile[UART_BANDRATE_NUM].configData);
	clientfd = TCP_NetAccept(sockfd);

	/* 将串口描述符、socket号以及client描述符的指针作为参数，传给透传线程 */
	fdArray[0] = uartfd;
	fdArray[1] = sockfd;
	fdArray[2] = (int)&clientfd;

	/* 网口与串口透传的线程 */
	ret = pthread_create(&net2UartPid, NULL, (void*)Server2UartPthread, fdArray);
	if(0 != ret)
	{
		printf("pthread Net2Uart create error!\n");
		goto TCP_SERVER_CLOSE;
	}

	ret = pthread_create(&uart2NetPid, NULL, (void*)Uart2ServerPthread, fdArray);
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

	/* 连接UDP并建立socket，打开串口，设置远端配置的IP和端口号 */
	sockfd = UDP_NetConnect(g_ConfigFile[LOCAL_PORT_NUM].configData);
	uartfd = UartInit(g_ConfigFile[UART_DEVICE_NAME_NUM].configString, g_ConfigFile[UART_BANDRATE_NUM].configData);
	SetRemoteAddress(&remoteAddr);

	/* 将串口描述符，socket号以及远端配置信息结构体指针作为参数传给透传线程 */
	fdArray[0] = uartfd;
	fdArray[1] = sockfd;
	fdArray[2] = (int)&remoteAddr;

	/* 网口与串口透传的线程 */
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

	return 0;
}


/**
 * @breif TCP客户端接口转发到串口的线程程序
 * @param param 整型数组，第一个数存放串口描述符，第二个数存放网络socket描述符指针
 * @return void
 */
static void Client2UartPthread(void *param)
{
    int *socketfd, uartfd;
    int recvBytes = 0;      //接收到的字节数
    char bufReceive[MAX_DATA_SIZE] = {0};       //接收缓存区

    uartfd = ((int*)param)[0];
    socketfd = (int*)((int*)param)[1];

    while(1)
    {
        recvBytes = recv(*socketfd, bufReceive, MAX_DATA_SIZE, 0);
        if(recvBytes > 0)
        {
            if(write(uartfd, bufReceive, recvBytes) == -1)
            {
                printf("write error！\r\n");
                continue;
            }
            memset(bufReceive, 0, MAX_DATA_SIZE);
        }
        else if(recvBytes == 0)		//远端断开连接
        {
        	TCP_CloseConnect(socketfd);		//关闭连接

			*socketfd = TCP_NetConnect(g_ConfigFile[REMOTE_IP_ADDRESS_NUM].configString,
						g_ConfigFile[REMOTE_PORT_NUM].configData);		//重新连接服务端
			if(*socketfd == -1)
				return;
        }
    } 
}


/**
 * @breif TCP服务端接口转发到串口的线程程序
 * @param param 整型数组，第一个数存放串口描述符，第二个数存放网络socket描述符，第三个数存放网络client描述符的指针
 * @return void
 */
static void Server2UartPthread(void *param)
{
    int socketfd, uartfd, *clientfd;
    int recvBytes = 0;      //接收到的字节数
    char bufReceive[MAX_DATA_SIZE] = {0};       //接收缓存区

    uartfd = ((int*)param)[0];
    socketfd = ((int*)param)[1];
    clientfd = (int*)((int*)param)[2];

    while(1)
    {
        recvBytes = recv(*clientfd, bufReceive, MAX_DATA_SIZE, 0);
        if(recvBytes > 0)
        {
            if(write(uartfd, bufReceive, recvBytes) == -1)
            {
                printf("write error！\r\n");
                continue;
            }
            memset(bufReceive, 0, MAX_DATA_SIZE);
        }
        else if(recvBytes == 0)			//远端断开连接
        {
        	TCP_CloseConnect(clientfd);		//关闭连接

			*clientfd = TCP_NetAccept(socketfd);		//重新连接服务端
			if(*clientfd == -1)
				return;
        }
    }
}


/**
 * @breif 串口接口转发到TCP客户端的线程程序
 * @param param 整型数组，第一个数存放串口描述符，第二个数存放网络socket描述符指针
 * @return void
 */
static void Uart2ClientPthread(void *param)
{
    int *socketfd, uartfd;
    int nread = 0;			//发送的字节数
    char bufSend[MAX_DATA_SIZE] = {0};		//发送缓存区

    uartfd = ((int*)param)[0];
    socketfd = (int*)((int*)param)[1];

    while(1)
    {
        nread = read(uartfd, bufSend, sizeof(bufSend));
        if (nread > 0)
        {
            if (send(*socketfd, bufSend, nread, 0) == -1)
            {
                printf("send error！\r\n");
                continue;
            }
            memset(bufSend, 0, MAX_DATA_SIZE);
        }
    }
}


/**
 * @breif 串口接口转发到TCP服务器端的线程程序
 * @param param 整型数组，第一个数存放串口描述符，第二个数存放网络socket描述符，第三个数存放网络client描述符的指针
 * @return void
 */
static void Uart2ServerPthread(void *param)
{
    int *clientfd, uartfd;
    int nread = 0;				//发送的字节数
    char bufSend[MAX_DATA_SIZE] = {0};		//发送缓存区

    uartfd = ((int*)param)[0];
    clientfd = (int*)((int*)param)[2];

    while(1)
    {
        nread = read(uartfd, bufSend, sizeof(bufSend));
        if (nread > 0)
        {
            if (send(*clientfd, bufSend, nread, 0) == -1)
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
 * @param param 整型数组，第一个数存放串口描述符，第二个数存放网络socket描述符，第三个存放远端信息结构体的指针
 * @return void
 */
static void UDP2UartPthread(void *param)
{
    int uartfd, sockfd;
    int recvBytes;		//接收字节数
    struct sockaddr_in *remoteAddr = NULL;
	char bufReceive[MAX_DATA_SIZE] = {0};		//接收缓存区
    socklen_t sinSize = sizeof(struct sockaddr_in);

    uartfd = ((int*)param)[0];
    sockfd = ((int*)param)[1];
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
 * @param param 整型数组，第一个数存放串口描述符，第二个数存放网络socket描述符，第三个存放远端信息结构体的指针
 * @return void
 */
void Uart2UDPPthread(void *param)
{
    int uartfd, sockfd;
    char bufSend[MAX_DATA_SIZE] = {0};		//发送缓存区
	int nread = 0;			//发送字节数
    struct sockaddr_in *remoteAddr = NULL;
    socklen_t sinSize = sizeof(struct sockaddr_in);

    uartfd = ((int*)param)[0];
    sockfd = ((int*)param)[1];
    remoteAddr = (struct sockaddr_in *)((int*)param)[2];

    while(1)
	{
		nread = read(uartfd, bufSend, sizeof(bufSend));
		if (nread > 0)
		{
			if (sendto(sockfd, bufSend, nread, 0, (struct sockaddr *)remoteAddr, sinSize) == -1)
			{
				printf("send error！\r\n");
				continue;
			}
			memset(bufSend, 0, MAX_DATA_SIZE);
		}
	}
}

