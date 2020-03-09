/**
 * @file Interface_S2J.c
 * @brief 实现struct2json函数对外接口
 * @copyright Copyright (c) 2020 Beijing SOJO Electric CO., LTD.
 * @company SOJO
 * @date 2020.02.18
 *
 * @author Lei.L
 * @version ver 1.0
 */
 
#include "Interface_S2J.h"

/**
 * @fn ConfigFile_StructToJson
 * @brief 实配置文件struct转json
 * @return 返回指向转换后的json指针
 *
 */
cJSON *ConfigFile_StructToJson(void* struct_obj)
{
    ConfigFile *struct_Config = (ConfigFile *)struct_obj;
	/* create Student JSON object */
	s2j_create_json_obj(json_obj);

	/* serialize data to Student JSON object. */
	s2j_json_set_basic_element(json_obj, struct_Config, string, dataName);		//名称
	s2j_json_set_basic_element(json_obj, struct_Config, string, configString);	//配置字符串
	s2j_json_set_basic_element(json_obj, struct_Config, int, configData);		//配置数据
	s2j_json_set_basic_element(json_obj, struct_Config, int, number);			//序号

	/* return Student JSON object pointer */
	return json_obj;
}

/**
 * @fn ConfigFile_JsonToStruct
 * @brief 配置文件json转换为结构体
 * @param json_obj 指向要转换的json文件
 * @return 返回指向获取到的json指针
 * 
 */
//#include <stdio.h>
ConfigFile *ConfigFile_JsonToStruct(cJSON* json_obj)
{
    /* create ConfigFile structure object */
    s2j_create_struct_obj(struct_Config, ConfigFile);

    /* deserialize data to ConfigFile structure object. */
    s2j_struct_get_basic_element(struct_Config, json_obj, string, dataName);		//名称
    s2j_struct_get_basic_element(struct_Config, json_obj, string, configString);	//配置字符串
	s2j_struct_get_basic_element(struct_Config, json_obj, int, configData);			//配置数据
	s2j_struct_get_basic_element(struct_Config, json_obj, int, number);				//序号


    /* return Config structure object pointer */
    return struct_Config;
}

/**
 * @fn vPort_Get_cJSON
 * @brief 获取json，重新封装cJSON_Parse函数
 * @param value 指向要转换为json结构的字符串
 * @return 返回指向获取到的json指针
 * 
 */
inline cJSON* vPort_Get_cJSON(const char *value)
{
    return cJSON_Parse(value);
}


/**
 * @fn vPort_Print_cJSON
 * @brief 打印json，重新封装cJSON_Print函数
 * @param item 指向要打印的json结构体的指针
 * @return 返回指向打印的json结构体生成的字符串指针
 * 
 */
inline char* vPort_Print_cJSON(cJSON *item)
{
	char *str = cJSON_Print(item);
	if(item != NULL)
	{
		s2j_delete_json_obj(item);
	}
    return str;
}


