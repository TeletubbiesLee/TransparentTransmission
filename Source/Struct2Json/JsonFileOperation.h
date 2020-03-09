/**
 * @file JsonFileOperation.c
 * @brief 实现Json文件操作
 * @copyright Copyright (c) 2020 Beijing SOJO Electric CO., LTD.
 * @company SOJO
 * @date 2020.02.18
 *
 * @author Lei.L
 * @version ver 1.0
 */
 
#ifndef __JSON_FILE_OPERATION_
#define __JSON_FILE_OPERATION_

#include "stdint.h"


extern uint8_t Struct2JsonInit(void);  //初始化struct2json
extern uint8_t CreateJsonFile(void);
extern uint8_t GetJsonFile(void);

#endif

