#pragma once
#include <iostream> //标准的输入输出流头文件
#include <windows.h> //windowsAPI的函数声明和宏
#include "TcAdsDef.h"
#include "TcAdsAPI.h"

class AdsExpand
{
public:
	AdsExpand(PAmsAddr pAddr);
	~AdsExpand();
	template<typename T>
	long WriteArray(char* szVar, T* array_data, int array_data_length);
	template<typename T>
	long ReadArray(char* szVar, T* array_data, int array_data_length);
protected:
private:
	long      m_nPort;	//定义端口变量
	AmsAddr   m_Addr;		//定义AMS地址变量
	PAmsAddr  m_pAddr = &m_Addr;//定义端口地址变量
};

AdsExpand::AdsExpand(PAmsAddr pAddr) :m_pAddr(pAddr)
{
	m_nPort = AdsPortOpenEx();		//打开ADS通讯端口
}

AdsExpand::~AdsExpand()
{
	AdsPortCloseEx(m_nPort);
}

template<typename T>
inline long AdsExpand::WriteArray(char* szVar, T* array_data, int array_data_length)
{
	unsigned long lHdlVar;       // 创建句柄
	unsigned long pcbReturn;

	// 复制传入的数组以避免修改原始数据
	T* myArray = new T[array_data_length];
	for (int i = 0; i < array_data_length; ++i)
	{
		myArray[i] = array_data[i];
	}

	long nErr = AdsSyncReadWriteReqEx2(m_nPort, m_pAddr, ADSIGRP_SYM_HNDBYNAME, 0x0, sizeof(lHdlVar), &lHdlVar, strlen(szVar), (void*)szVar, &pcbReturn);
	if (nErr) {
		delete[] myArray;
		return nErr;
	}

	// 写入数组数据
	nErr = AdsSyncWriteReqEx(m_nPort, m_pAddr, ADSIGRP_SYM_VALBYHND, lHdlVar, sizeof(T) * array_data_length, &myArray[0]);
	if (nErr) {
		delete[] myArray;
		return nErr;
	}

	delete[] myArray;
	return 0;
}

template<typename T>
long AdsExpand::ReadArray(char* szVar, T* array_data, int array_data_length)
{
	unsigned long lHdlVar;
	unsigned long pcbReturn;

	long nErr = AdsSyncReadWriteReqEx2(m_nPort, m_pAddr, ADSIGRP_SYM_HNDBYNAME, 0x0, sizeof(lHdlVar), &lHdlVar, strlen(szVar), (void*)szVar, &pcbReturn);
	if (nErr)
	{
		return nErr;
	}

	// 读取数组数据
	T* myArray = new T[array_data_length];
	nErr = AdsSyncReadReqEx2(m_nPort, m_pAddr, ADSIGRP_SYM_VALBYHND, lHdlVar, sizeof(T) * array_data_length, &myArray[0], &pcbReturn); //这个读的是固定大小的数组哇
	if (nErr)
	{
		delete[] myArray;
		return nErr;
	}
	else
	{
		for (int j = 0; j < array_data_length; ++j)
		{
			array_data[j] = myArray[j];
		}
	}

	delete[] myArray;
	return 0;
}
