/**
 * @file ConfigFile.c
 * @brief 程序部分参数配置文件
 * @copyright Copyright (c) 2020 Beijing SOJO Electric CO., LTD.
 * @company SOJO
 * @date 2020.02.18
 *
 * @author Lei.L
 * @version ver 1.0
 */
 

#include "ConfigFile.h"
#include <string.h>
#include <stdio.h>


/*************************************************全局变量*************************************************/
ConfigFile g_ConfigFile[] =
{
	{
		.dataName = "UartDeviceName",
		.configString = "/dev/ttymxc1",
		.configData = 1,
		.number = UART_DEVICE_NAME_NUM
	},
	{
		.dataName = "UartBandrate",
		.configString = "115200",
		.configData = 115200,
		.number = UART_BANDRATE_NUM
	},
	{
		.dataName = "UartDataBits",
		.configString = "8",
		.configData = 8,
		.number = UART_DATA_BITS_NUM
	},
	{
		.dataName = "UartStopBits",
		.configString = "1",
		.configData = 1,
		.number = UART_STOP_BITS_NUM
	},
	{
		.dataName = "UartParity",
		.configString = "N",
		.configData = 'N',
		.number = UART_PARITY_NUM
	},
	{
		.dataName = "LocalIpAddress",
		.configString = "192.168.10.10",
		.configData = 0xC0A80A0A,
		.number = LOCAL_IP_ADDRESS_NUM
	},
	{
		.dataName = "LocalPort",
		.configString = "5555",
		.configData = 5555,
		.number = LOCAL_PORT_NUM
	},
	{
		.dataName = "RemoteIpAddress",
		.configString = "192.168.10.11",
		.configData = 0xC0A80A0B,
		.number = REMOTE_IP_ADDRESS_NUM
	},
	{
		.dataName = "RemotePort",
		.configString = "3333",
		.configData = 3333,
		.number = REMOTE_PORT_NUM
	}
};

ConfigFile *g_JsonFile = g_ConfigFile;

/*************************************************END*************************************************/

/*************************************************公有函数实现*************************************************/
/**
 * @fn GetConfigFileLength
 * @brief 获取配置文件结构体数组长度
 * @return 返回数组长度
 */
inline int GetConfigFileLength(void)
{
    return (sizeof(g_ConfigFile) / sizeof(g_ConfigFile[0]));    //配置文件结构体数组长度;
}



/*************************************************END*************************************************/









