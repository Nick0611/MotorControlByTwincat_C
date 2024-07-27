#pragma once
#include <CPSAPI/CPSDef.h>


#pragma pack(push, 4)

// 计算服务计算结果
#define RRI_SEG_NUM		120
#define RIB_NUM 5
#define RRI_MSG_CALC_RESULT		0xD07
typedef struct
{
	int	req_id;            // 计算请求的ID
	//目前只适用前三个量，即接触位置、方向和力大小
	double contact_position[3]; //接触位置
	double contact_direction[3]; //接触方向
	double contact_force;       //接触力大小
	double q[RRI_SEG_NUM];     // 弹性板当前关节角度
	double rxix[RRI_SEG_NUM];   // 弹性板形状--等效关节x坐标
	double rxiy[RRI_SEG_NUM];   // 弹性板形状--等效关节y坐标
	double ribx[RIB_NUM*2];   // 肋板位置--x坐标
	double riby[RIB_NUM*2];   // 肋板位置--y坐标
	char t1[64];
	char t2[64];
	char t3[64];
	char t4[64];
	char t5[64];
	char t6[64];
	char t7[64];
	ST_MsgRsp info;          // 结果信息，如果出错，给error_code赋非0值
}ST_RRI_Calc_Result;

#pragma pack(pop)

