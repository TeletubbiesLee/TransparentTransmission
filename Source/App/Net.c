/**
 * @file Net.c
 * @brief Linux系统中网络设备相关操作函数文件
 * @copyright Copyright (c) 2020 Beijing SOJO Electric CO., LTD.
 * @company  SOJO
 * @date 2020.02.17
 *
 * @author Lei.L
 * @version ver 1.0
 */

#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <strings.h>
#include "Net.h"
#include "../Struct2Json/ConfigFile.h"


/**
 * @breif 打开网口设备
 * @param ipAddress IP地址，格式："192.168.1.1"
 * @param serverPort 服务端串口号
 * @return 设备文件描述符或-1
 */
int TCP_NetConnect(char *ipAddress, int serverPort)
{
    int sockfd = -1;
    struct hostent *host;
    struct sockaddr_in serverAddr;

    if((host = gethostbyname(ipAddress)) == NULL)
	{
		printf("gethostbyname error！");
		return -1;
	}

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		printf("socket create error!");
		return -1;
	}

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(serverPort);
	serverAddr.sin_addr = *((struct in_addr *)host->h_addr);
	bzero(&(serverAddr.sin_zero), 8);
	while (connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(struct sockaddr)) == -1)
	{
		printf("TCP_NetConnect:connect error!\n");
		sleep(1);
	}

    return sockfd;
}


/**
 * @breif 网口设备侦听
 * @param serverPort 服务端串口号
 * @return 设备文件描述符或-1
 */
int TCP_NetListen(int serverPort)
{
    int sockfd = -1;
    struct sockaddr_in loaclAddr;     /* loacl */

    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		printf("socket fail!");
        return -1;
	}

	loaclAddr.sin_family = AF_INET;
	loaclAddr.sin_port = htons(serverPort);
	loaclAddr.sin_addr.s_addr = INADDR_ANY;
	bzero(&(loaclAddr.sin_zero),8);

	if (bind(sockfd, (struct sockaddr *)&loaclAddr, sizeof(struct sockaddr)) == -1)
	{
		printf("bind error!");
		return -1;
	}
	if (listen(sockfd, BACKLOG) == -1)
    {
		printf("listen error!");
		return -1;
	}

    return sockfd;
}


/**
 * @breif 网口设备连接
 * @param sockfd 服务端socket文件描述符
 * @return 设备文件描述符或-1
 */
int TCP_NetAccept(int sockfd)
{
    int clientfd = -1;
    struct sockaddr_in remoteAddr;
    socklen_t sinSize;

    sinSize = sizeof(struct sockaddr_in);
	clientfd = accept(sockfd, (struct sockaddr *)&remoteAddr, &sinSize);
	if (clientfd == -1)
	{
		printf("accept error\n");
        return -1;
	}
	else
	{
		printf("REC FROM： %s\n", inet_ntoa(remoteAddr.sin_addr));
	}

    return clientfd;
}


/**
 * @breif 打开网口设备
 * @param serverPort 服务端串口号
 * @return 设备文件描述符或-1
 */
int UDP_NetConnect(int serverPort)
{
    int sockfd = -1;
    struct sockaddr_in localAddr;         /* loacl */

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
	{
		printf("socket fail!");
        return -1;
	}

	localAddr.sin_family = AF_INET;
	localAddr.sin_port = htons(serverPort);
	localAddr.sin_addr.s_addr = INADDR_ANY;

	bzero(&(localAddr.sin_zero), 8);

	if (bind(sockfd, (struct sockaddr *)&localAddr, sizeof(struct sockaddr)) == -1)
	{
		printf("bind error!");
		return -1;
	}

    return sockfd;
}


/**
 * @breif 打开串口设备
 * @param sockfd 网口文件描述符
 * @return void
 */
void SetNetNonBlock(int sockfd)
{
    int flags;
    
    flags = fcntl(sockfd, F_GETFL, 0);				//获取原始sockfd属性
	fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);		//添加非阻塞
}


/**
 * @breif 打开串口设备
 * @param remoteAddr 远端IP地址和端口号设置
 * @return void
 */
void SetRemoteAddress(struct sockaddr_in *remoteAddr)
{
	remoteAddr->sin_family = AF_INET;
	remoteAddr->sin_port = htons(g_ConfigFile[REMOTE_PORT_NUM].configData);
	remoteAddr->sin_addr.s_addr = inet_addr(g_ConfigFile[REMOTE_IP_ADDRESS_NUM].configString);
}







