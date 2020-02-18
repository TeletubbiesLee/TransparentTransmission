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

#include <netdb.h>
#include "Net.h"


/**
 * @breif 打开网口设备
 * @param serverPort 服务端串口号
 * @param ipAddress IP地址，格式："192.168.1.1"
 * @return 设备文件描述符或-1
 */
int TCP_NetConnect(int serverPort, char *ipAddress)
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
	if (connect(sockfd, (struct sockaddr *)&serverAddr, sizeof(struct sockaddr)) == -1)
	{
		printf("connect error!");
		return -1;
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
		SetNetNonBlock(clientfd);
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
 * @return 设备文件描述符或-1
 */
void SetNetNonBlock(int sockfd)
{
    int flags;
    
    flags = fcntl(sockfd, F_GETFL, 0);				//获取原始sockfd属性
	fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);		//添加非阻塞
}







