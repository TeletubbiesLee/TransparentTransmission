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

#ifndef _PTHREAD_H_
#define _PTHREAD_H_

#define TCP_CLIENT			0
#define TCP_SERVER			1
#define UDP_PROTOCOL		2
#define RUN_NET_PROTOCOL 	TCP_SERVER		//该项为配置项

int TCP_Client2Uart(void);
int TCP_Server2Uart(void);
int UDP2Uart(void);


#endif
