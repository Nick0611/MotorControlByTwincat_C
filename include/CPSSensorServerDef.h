#pragma once
#include <CPSAPI/CPSDef.h>

#pragma pack(push, 4)
////////////////////////////////SensorServer服务消息////////////////////////////////////////

#define MAX_SENSOR_DATA_NUM						20   // 最大20个，如果不够，可以修改
#define	MSG_SENSOR_DATA					         0xA01
typedef struct
{
	unsigned int	sensor_num;
	float			data[MAX_SENSOR_DATA_NUM];
	char t1[64];
	char t2[64];
	char t3[64];
}ST_SensorData;

#pragma pack(pop)

