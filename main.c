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
// #include <netinet/in.h>
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
		goto CLOSE;
	}

	ret = pthread_create(&uart2NetPid, NULL, (void*)Uart2Net, fdArray);
	if(0 != ret)
	{
		printf("pthread Uart2Net create error!\n");
		goto CLOSE;
	}

CLOSE:
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
		goto CLOSE;
	}

	ret = pthread_create(&uart2NetPid, NULL, (void*)Uart2Net, fdArray);
	if(0 != ret)
	{
		printf("pthread Uart2Net create error!\n");
		goto CLOSE;
	}

CLOSE:
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
	int recvbytes;
	struct sockaddr_in remoteAddr;
	char bufReceive[MAX_DATA_SIZE];
	char bufSend[MAX_DATA_SIZE];
	socklen_t sin_size;
	int nread;			/* Read the counts of data */

	remoteAddr.sin_family = AF_INET;
	remoteAddr.sin_port = htons(SERVER_PORT);
	remoteAddr.sin_addr.s_addr = inet_addr(REMOTE_IP_ADDRESS);

	sockfd = UDP_NetConnect(SERVER_PORT, REMOTE_IP_ADDRESS);		//连接网口
	uartfd = UartInit(UART_DEVICE_NAME, UART_BANDRATE);		//打开串口
	SetNetNonBlock(sockfd);			//设置网口非阻塞

	while(1)
	{
		recvbytes = recvfrom(sockfd, bufReceive, sizeof(bufReceive)-1, 0, (struct sockaddr *)&remote_addr, &sin_size);
		if(recvbytes > 0)
		{
			bufReceive[recvbytes] = '\0';
			if (write(uartfd, bufReceive, strlen(bufReceive)) == -1)
			{
				printf("write error！\r\n");
				exit(1);
			}
			printf("sockfd receivr, usart send: %s\r\n", bufReceive);

		}

		nread = read(uartfd, bufSend, sizeof(bufSend));
		if (nread > 0)
		{
			bufSend[nread] = '\0';
			if (sendto(sockfd, bufSend, strlen(bufSend), 0, (struct sockaddr *)&remote_addr, sizeof(remote_addr)) == -1)
			{
				printf("send error！\r\n");
				exit(1);
			}
			printf("usart receivr, sockfd send: %s\r\n", bufSend);
		}
	}

	close(sockfd);
	close(uartfd);

	return 0;
}






