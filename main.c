// #include <stdio.h>
// #include <stdlib.h>
// #include <errno.h>
// #include <string.h>
// #include <netdb.h>
// #include <unistd.h>
// #include <fcntl.h>
// #include <sys/types.h>
// #include <sys/stat.h>
// #include <sys/ioctl.h>
// 
// #include <arpa/inet.h>
// #include <sys/socket.h>
// #include <getopt.h>
// #include <termios.h>

#include "Uart.h"
#include "Net.h"
#include "Pthread.h"
#include <pthread.h>


#define BACKLOG 10 /* max client */


/**
 *	TCP客户端
 */
int TCP_Client2Uart(char *ipAddress)
{
	int uartfd, sockfd;		//串口和网卡设备的文件描述符
	int fdArray[2] = {0};	//存放网络socket和串口的描述符
	pthread_t net2UartPid, uart2NetPid;		//网口与串口转换的线程ID号
	int ret = -1;
	
	sockfd = TCP_NetConnect(SERVER_PORT, ipAddress);		//连接网口
	uartfd = UartInit(UART_DEVICE_NAME, UART_BANDRATE);		//打开串口

	fdArray[0] = sockfd;
	fdArray[1] = uartfd;

	/* 常见网口与串口透传的线程 */
	ret = pthread_create(&net2UartPid, NULL, (void*)Net2Uart, fdArray);
	if(0 != ret)
	{
		printf("pthread Net2Uart create error!\n");
		goto TCP_CLIENT_CLOSE;
	}

	ret = pthread_create(&uart2NetPid, NULL, (void*)Uart2Net, fdArray);
	if(0 != ret)
	{
		printf("pthread Uart2Net create error!\n");
		goto TCP_CLIENT_CLOSE;
	}

TCP_CLIENT_CLOSE:
	pthread_join(net2UartPid);
	pthread_join(uart2NetPid);
	close(sockfd);
	close(uartfd);

	return 0;
}


/**
 *	TCP服务器
 */
int TCP_Server2Uart(void)
{
	int uartfd, sockfd, clientfd;
	int fdArray[2] = {0};	//存放网络socket和串口的描述符
	pthread_t net2UartPid, uart2NetPid;		//网口与串口转换的线程ID号
	int ret = -1;

	sockfd = TCP_NetListen(SERVER_PORT);
	uartfd = UartInit(UART_DEVICE_NAME, UART_BANDRATE);		//打开串口
	clientfd = TCP_NetAccept(sockfd);
	
	fdArray[0] = clientfd;
	fdArray[1] = uartfd;

	/* 常见网口与串口透传的线程 */
	ret = pthread_create(&net2UartPid, NULL, (void*)Net2Uart, fdArray);
	if(0 != ret)
	{
		printf("pthread Net2Uart create error!\n");
		goto TCP_SERVER_CLOSE;
	}

	ret = pthread_create(&uart2NetPid, NULL, (void*)Uart2Net, fdArray);
	if(0 != ret)
	{
		printf("pthread Uart2Net create error!\n");
		goto TCP_SERVER_CLOSE;
	}

TCP_SERVER_CLOSE:
	pthread_join(net2UartPid);
	pthread_join(uart2NetPid);
	close(clientfd);
	close(sockfd);
	close(uartfd);

	return 0;
}


/**
 *	UDP
 */
int UDP2Uart(void)
{
	int uartfd, sockfd;
	int fdArray[2] = {0};	//存放网络socket和串口的描述符
	pthread_t udp2UartPid, uart2UdpPid;		//网口与串口转换的线程ID号
	int ret = -1;

	sockfd = UDP_NetConnect(SERVER_PORT, REMOTE_IP_ADDRESS);		//连接网口
	uartfd = UartInit(UART_DEVICE_NAME, UART_BANDRATE);		//打开串口

	fdArray[0] = sockfd;
	fdArray[1] = uartfd;

	/* 常见网口与串口透传的线程 */
	ret = pthread_create(&udp2UartPid, NULL, (void*)UDP2Uart, fdArray);
	if(0 != ret)
	{
		printf("pthread UDP2Uart create error!\n");
		goto UDP_CLOSE;
	}

	ret = pthread_create(&uart2UdpPid, NULL, (void*)Uart2UDP, fdArray);
	if(0 != ret)
	{
		printf("pthread Uart2UDP create error!\n");
		goto UDP_CLOSE;
	}

UDP_CLOSE:
	pthread_join(udp2UartPid);
	pthread_join(uart2UdpPid);
	close(sockfd);
	close(uartfd);

	return 0;
}






