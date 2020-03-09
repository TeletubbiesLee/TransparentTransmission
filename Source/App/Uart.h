/**
 * @file Uart.h
 * @brief Linux系统中串口设备相关操作函数文件
 * @copyright Copyright (c) 2020 Beijing SOJO Electric CO., LTD.
 * @company  SOJO
 * @date 2020.02.17
 *
 * @author Lei.L
 * @version ver 1.0
 */

#ifndef _UART_H_
#define _UART_H_


int UartInit(char *device, int bandrate);
int OpenDevice(char *dev);
int SetParity(int fd, int dataBits, int stopBits, int parity);
void SetSpeed(int fd, int speed);
void PrintUartUsage();

#endif
