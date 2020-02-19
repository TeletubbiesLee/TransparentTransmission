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

#include "Pthread.h"
#include "Net.h"


/**
 * @breif 网口接口转发到串口的线程程序
 * @param param 整型数组，第一个数存放网络socket描述符，第二个数存放串口描述符
 * @return 设备文件描述符或-1
 */
void Net2Uart(void *param)
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
                exit(1);
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
void Uart2Net(void *param)
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
                exit(1);
            }
            printf("usart receivr, socket send: %s\r\n", bufSend);
        }
        sleep(1);
    }
}
