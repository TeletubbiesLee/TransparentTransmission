/**
 * @file Interface_S2J.h
 * @brief 实现struct2json函数对外接口
 * @copyright Copyright (c) 2020 Beijing SOJO Electric CO., LTD.
 * @company SOJO
 * @date 2020.02.18
 *
 * @author Lei.L
 * @version ver 1.0
 */

#ifndef __INTER_FACE_S2J_
#define __INTER_FACE_S2J_

#include "s2j.h"
#include "ConfigFile.h"


extern cJSON *ConfigFile_StructToJson(void* struct_obj);         //实现固有参数结构体的struct转json
extern ConfigFile *ConfigFile_JsonToStruct(cJSON* json_obj);         //实现固有参数结构体的struct转json

extern cJSON* vPort_Get_cJSON(const char *value);  //从字符串中获得json
extern char* vPort_Print_cJSON(cJSON *item);       //将json打印到字符串中

#endif
