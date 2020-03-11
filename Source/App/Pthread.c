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
static void UartSendPthread(void *param);
static void UartReceivePthread(void *param);
static void UDPSendPthread(void *param);
static void UDPReceivePthread(void *param);


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
	int uart2UdpPipe[2] = {0}, udp2UartPipe[2] = {0};		//串口转UDP的管道识别号，UDP转串口的管道识别号
	int paramArray[4][3] = {{0}, {0}, {0}, {0}};			//用于给线程传递参数
	pthread_t udpSendtPid, uartSendPid, udpReceive, uartReceive;		//网口与串口转换的线程ID号
	struct sockaddr_in remoteAddr;
	int ret = -1;
	void *status;

	/* 建立管道pipe */
	if(pipe(uart2UdpPipe) < 0)
	{
		printf("uart to udp pipe error!\n");
	}
	if(pipe(udp2UartPipe) < 0)
	{
		printf("udp to uart pipe error!\n");
	}

	/* 连接UDP并建立socket，打开串口，设置远端配置的IP和端口号 */
	sockfd = UDP_NetConnect(g_ConfigFile[LOCAL_PORT_NUM].configData);
	uartfd = UartInit(g_ConfigFile[UART_DEVICE_NAME_NUM].configString, g_ConfigFile[UART_BANDRATE_NUM].configData);
	SetRemoteAddress(&remoteAddr);


	/* 网口与串口透传的线程 */
	paramArray[0][0] = uartfd;
	paramArray[0][1] = uart2UdpPipe[1];
	ret = pthread_create(&uartReceive, NULL, (void*)UartReceivePthread, paramArray[0]);
	if(0 != ret)
	{
		printf("pthread Uart Receive create error!\n");
		goto UDP_CLOSE;
	}

	paramArray[1][0] = sockfd;
	paramArray[1][1] = udp2UartPipe[1];
	paramArray[1][2] = (int)&remoteAddr;
	ret = pthread_create(&udpReceive, NULL, (void*)UDPReceivePthread, paramArray[1]);
	if(0 != ret)
	{
		printf("pthread UDP Receive create error!\n");
		goto UDP_CLOSE;
	}

	paramArray[2][0] = uartfd;
	paramArray[2][1] = udp2UartPipe[0];
	ret = pthread_create(&uartSendPid, NULL, (void*)UartSendPthread, paramArray[2]);
	if(0 != ret)
	{
		printf("pthread Uart Send create error!\n");
		goto UDP_CLOSE;
	}

	paramArray[3][0] = sockfd;
	paramArray[3][1] = uart2UdpPipe[0];
	paramArray[3][2] = (int)&remoteAddr;
	ret = pthread_create(&udpSendtPid, NULL, (void*)UDPSendPthread, paramArray[3]);
	if(0 != ret)
	{
		printf("pthread UDP Send create error!\n");
		goto UDP_CLOSE;
	}

UDP_CLOSE:
	pthread_join(uartReceive, &status);
	pthread_join(udpReceive, &status);
	pthread_join(uartSendPid, &status);
	pthread_join(udpSendtPid, &status);
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
 * @breif 串口发送的线程程序
 * @param param 整型数组，第一个数存放串口描述符，第二个数存放读管道号
 * @return void
 */
static void UartSendPthread(void *param)
{
    int uartfd, pipefd;
    int dataBytes;		//数据字节数
	char dataBuffer[MAX_DATA_SIZE] = {0};		//数据缓存区

    uartfd = ((int*)param)[0];
    pipefd = ((int*)param)[1];

    while(1)
    {
    	/* 从管道中读取数据 */
    	dataBytes = read(pipefd, dataBuffer, sizeof(dataBuffer));
    	if(dataBytes > 0)
    	{
			if (write(uartfd, dataBuffer, dataBytes) == -1)
			{
				printf("write error！\r\n");
				continue;
			}
			memset(dataBuffer, 0, MAX_DATA_SIZE);
    	}
    }
}


/**
 * @breif 串口接收的线程程序
 * @param param 整型数组，第一个数存放串口描述符，第二个数存放写管道号
 * @return void
 */
static void UartReceivePthread(void *param)
{
    int uartfd, pipefd;
    char dataBuffer[MAX_DATA_SIZE] = {0};		//数据缓存区
	int dataBytes = 0;			//数据字节数

    uartfd = ((int*)param)[0];
    pipefd = ((int*)param)[1];

    while(1)
	{
    	dataBytes = read(uartfd, dataBuffer, sizeof(dataBuffer));
		if (dataBytes > 0)
		{
			/* 将接收到的数据写入管道 */
			write(pipefd, dataBuffer, dataBytes);
			memset(dataBuffer, 0, MAX_DATA_SIZE);
		}
	}
}


/**
 * @breif UDP发送的线程程序
 * @param param 整型数组，第一个数存放网络socket描述符，第二个数存放读管道号，第三个存放远端信息结构体的指针
 * @return void
 */
static void UDPSendPthread(void *param)
{
    int sockfd, pipefd;
    char dataBuffer[MAX_DATA_SIZE] = {0};		//数据缓存区
	int dataBytes = 0;			//数据字节数
    struct sockaddr_in *remoteAddr = NULL;
    socklen_t sinSize = sizeof(struct sockaddr_in);

    sockfd = ((int*)param)[0];
    pipefd = ((int*)param)[1];
    remoteAddr = (struct sockaddr_in *)((int*)param)[2];

    while(1)
	{
		/* 从管道中读取数据 */
    	dataBytes = read(pipefd, dataBuffer, sizeof(dataBuffer));
    	if(dataBytes > 0)
    	{
			if (sendto(sockfd, dataBuffer, dataBytes, 0, (struct sockaddr *)remoteAddr, sinSize) == -1)
			{
				printf("send error！\r\n");
				continue;
			}
			memset(dataBuffer, 0, MAX_DATA_SIZE);
    	}
	}
}


/**
 * @breif UDP接收的线程程序
 * @param param 整型数组，第一个数存放网络socket描述符，第二个数存放写管道号，第三个存放远端信息结构体的指针
 * @return void
 */
static void UDPReceivePthread(void *param)
{
    int sockfd, pipefd;
    int recvBytes;		//数据字节数
    struct sockaddr_in *remoteAddr = NULL;
	char dataBuffer[MAX_DATA_SIZE] = {0};		//数据缓存区
    socklen_t sinSize = sizeof(struct sockaddr_in);

    sockfd = ((int*)param)[0];
    pipefd = ((int*)param)[1];
    remoteAddr = (struct sockaddr_in *)((int*)param)[2];

    while(1)
    {
        recvBytes = recvfrom(sockfd, dataBuffer, sizeof(dataBuffer)-1, 0, (struct sockaddr *)remoteAddr, &sinSize);
        if(recvBytes > 0)
        {
            /* 将接收到的数据写入管道 */
        	write(pipefd, dataBuffer, recvBytes);
            memset(dataBuffer, 0, MAX_DATA_SIZE);
        }
    }

}



