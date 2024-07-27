#pragma once
#include <CPSAPI/CPSDef.h>

#pragma pack(push, 4)
////////////////////////////////HypersenServerDef服务消息////////////////////////////////////////

#define HYPERSEN_SENSOR_DOF									6

#define MSG_HYPERSEN_GET_SENSOR_INFO						0xB01

#define MSG_HYPERSEN_SENSOR_INFO							0xB02
struct ST_HypersenSensorInfo
{
	uint16_t dev_id; // 设备ID
	char version[7]; // 固件生成日期(年，月，日)，固件版本（主版本，小版本，修订号）
};

#define MSG_HYPERSEN_START_READ								0xB03
#define MSG_HYPERSEN_STOP_READ								0xB04

#define	MSG_HYPERSEN_SENSOR_DATA					         0xB05
typedef struct
{
	char			code; // 异常数据指示，0 表示数据正常、0xFF 表示数据出现异常
	float			data[HYPERSEN_SENSOR_DOF]; // FxFyFz(单位：N),MxMyMz(单位：Nm)
}ST_HypersenSensorData;


#define MSG_HYPERSEN_RESET_ZERO								0xB06

#define MSG_HYPERSEN_READ_STATUS							0xB07
#define MSG_HYPERSEN_STATUS									0xB08

typedef struct
{
	char status;   // 0=stopped, 1=reading, 2=error
}ST_HypersenSensorStatus;

#pragma pack(pop)

