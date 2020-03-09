/**
 * @file ConfigFile.h
 * @brief 程序部分参数配置文件
 * @copyright Copyright (c) 2020 Beijing SOJO Electric CO., LTD.
 * @company SOJO
 * @date 2020.02.18
 *
 * @author Lei.L
 * @version ver 1.0
 */
 

#ifndef __CONFIG_FILE_H
#define	__CONFIG_FILE_H

#include <stdint.h>

/****************************宏定义****************************/
#define DIS_ENABLE  0               //禁止
#define ENABLE      !DIS_ENABLE     //使能

#define PARAM_ERROR  -1   //参数错误

#define CHECK_PARAM(parameter) {parameter > 1 ? PARAM_ERROR : 1}    //检测参数

#define CONFIG_STRUCT_MAX 30
#define DATA_NAME_LENTH_MAX 20
#define CONFIG_STRING_LENTH_MAX DATA_NAME_LENTH_MAX

enum ConfigNumber
{
	UART_DEVICE_NAME_NUM = 0,
	UART_BANDRATE_NUM,
	UART_DATA_BITS_NUM,
	UART_STOP_BITS_NUM,
	UART_PARITY_NUM,
	LOCAL_IP_ADDRESS_NUM,
	LOCAL_PORT_NUM,
	REMOTE_IP_ADDRESS_NUM,
	REMOTE_PORT_NUM,
};


/****************************结构体定义****************************/
typedef struct tagConfiguiation
{
    char dataName[DATA_NAME_LENTH_MAX];				//名称
    char configString[CONFIG_STRING_LENTH_MAX];		//配置字符串
    int configData;		//配置数据
    int number;			//序号
}ConfigFile;


/****************************全局变量定义****************************/
extern ConfigFile g_ConfigFile[];


/****************************函数定义****************************/
extern uint16_t GetConfigFileLength(void);


/**********************************END**********************************/


#endif


