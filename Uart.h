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

#include <fcntl.h>

#define UART_DEVICE_NAME    "/dev/ttymxc1"
#define UART_BANDRATE       115200
#define UART_DATA_BITS      8
#define UART_STOP_BITS      1
#define UART_PARITY         'N'

int UartInit(char *device, int bandrate);
int OpenDevice(char *dev);
int SetParity(int fd, int dataBits, int stopBits, int parity);
void SetSpeed(int fd, int speed);
void PrintUartUsage(FILE *stream, int exitCode);

#endif
