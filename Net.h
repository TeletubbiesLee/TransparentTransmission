/**
 * @file Net.h
 * @brief Linux系统中网络设备相关操作函数文件
 * @copyright Copyright (c) 2020 Beijing SOJO Electric CO., LTD.
 * @company  SOJO
 * @date 2020.02.17
 *
 * @author Lei.L
 * @version ver 1.0
 */

#ifndef _NET_H_
#define _NET_H_

#define SERVER_PORT 3333
#define REMOTE_IP_ADDRESS "192.168.1.1"

int TCP_NetConnect(int serverPort, char *ipAddress);
int TCP_NetListen(int serverPort);
int TCP_NetAccept(int sockfd);
int UDP_NetConnect(int serverPort);
void SetNetNonBlock(int sockfd);

#endif
