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
 
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include "s2j.h"
#include "JsonFileOperation.h"
#include "Interface_S2J.h"
#include "ConfigFile.h"
	
static int FilePositon;

/**
 * @fn rt_s2j_init
 * @brief 初始化struct2json
 * @return  RT_EOK    初始化成功
 *          其他      失败
 */
uint8_t Struct2JsonInit(void)
{
    s2jHook.malloc_fn = malloc;  //初始化内存申请函数
    s2jHook.free_fn = free;

    s2j_init(&s2jHook);   //初始化struct2json的内存申请

    return 0;
}

/**
 * @fn Create_JsonFile
 * @brief 结构体转换为json并写入到文件
 * @param fileName    要写入的文件名称
 * @param length    转换的长度
 * @param name    需要转换的结构体
 * 
 */
uint8_t CreateJsonFile(void)
{

    char* string;
    ssize_t writeNum = 0;
    uint16_t i;

    uint16_t length = GetConfigFileLength();    //获取长度

    FilePositon = open(JSON_FILENAME, O_RDWR | O_CREAT | O_TRUNC); //在根目录下创建一个可读写文件
    if(FilePositon < 0)
	{
		return 1;
	}
    
    writeNum = write(FilePositon, "{\"JsonConfigFile\":[ \n", strlen("{\"JsonConfigFile\":[ \n"));    //依照标准格式进行写入
    
    for(i = 0; i < length; i++)
    {
        cJSON* struct_json = ConfigFile_StructToJson(&g_JsonFile[i]);
        
        string = vPort_Print_cJSON(struct_json);

        writeNum = write(FilePositon, string, strlen(string));    //依照标准格式进行写入
        
		if(string != NULL)
		{
			memset(string, 0, (strlen(string) + 1));
			s2jHook.free_fn(string);
		}
        if(i < (length - 1))
        {
        	writeNum = write(FilePositon, ",\n", 2);    //依照标准格式进行写入
        }
        else
        {
        	writeNum = write(FilePositon, "\n", 1);    //依照标准格式进行写入
        }
    }

    writeNum = write(FilePositon, "]\n", 2);    //依照标准格式进行写入

    writeNum = write(FilePositon, "}\n", 2);    //依照标准格式进行写入

    close(FilePositon);
    
    return 0;
}


/**
 * @fn Get_JsonFile
 * @brief 获取json文件，获得配置信息
 * @return 0    成功获得json文件
 *         1    目录中无json文件，使用默认配置
 *         2    其他错误
 */
uint8_t GetJsonFile(void)
{
    //TERMINAL_PRODUCT_SERIAL_NUMBER
    char* string = NULL;
    struct stat fileStat;
	off_t fileSize = 0;
	size_t stringLenth = 0;
    cJSON * item;
    cJSON *readJson;
	cJSON * _item;
	int i, j;

	FilePositon = open(JSON_FILENAME, O_RDONLY); //在sojo目录下创建一个可读写文件
	if(FilePositon < 0)
    {
        goto JSON_RES;
    }
    //获取json文件
	fstat(FilePositon, &fileStat);
	fileSize = fileStat.st_size;	//获取文件大小
    string  = malloc(fileSize);   //申请内存
    if(NULL == string)
    {
    	goto JSON_RES;
    }

    stringLenth = read(FilePositon, string, fileSize);
    if(stringLenth == -1)
    {
        goto JSON_RES;
    }

    readJson = vPort_Get_cJSON(string);
    if(NULL == readJson)
    {
        goto JSON_RES;
    }

	
	for (i = 0; i < cJSON_GetArraySize(readJson); i++)   //遍历最外层json键值对
	{
		item = cJSON_GetArrayItem(readJson, i);
		if (cJSON_Object != item->type)		//值不为json对象则查找child是否为空，为空即不包含json
		{
			if (item->child != NULL)
			{
				for (j = 0; j < cJSON_GetArraySize(item); j++)   //遍历外层json键值对
				{
					_item = cJSON_GetArrayItem(item, j);
					if (cJSON_Object == _item->type)
					{
						/* deserialize Student structure object */
						g_JsonFile[j] = * ConfigFile_JsonToStruct(_item);
					}
				}
			}
		}
	}
    if(readJson != NULL)
	{
		s2j_delete_json_obj(readJson);
	}

    
JSON_RES:
	close(FilePositon);
	if(NULL == string)
	{
		free(string);  //释放内存
	}
    return 0;
}


