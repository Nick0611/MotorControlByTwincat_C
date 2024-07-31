#pragma once
#include <iostream> //��׼�����������ͷ�ļ�
#include <windows.h> //windowsAPI�ĺ��������ͺ�
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
	long      m_nPort;	//����˿ڱ���
	AmsAddr   m_Addr;		//����AMS��ַ����
	PAmsAddr  m_pAddr = &m_Addr;//����˿ڵ�ַ����
};

AdsExpand::AdsExpand(PAmsAddr pAddr) :m_pAddr(pAddr)
{
	m_nPort = AdsPortOpenEx();		//��ADSͨѶ�˿�
}

AdsExpand::~AdsExpand()
{
	AdsPortCloseEx(m_nPort);
}

template<typename T>
inline long AdsExpand::WriteArray(char* szVar, T* array_data, int array_data_length)
{
	unsigned long lHdlVar;       // �������
	unsigned long pcbReturn;

	// ���ƴ���������Ա����޸�ԭʼ����
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

	// д����������
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

	// ��ȡ��������
	T* myArray = new T[array_data_length];
	nErr = AdsSyncReadReqEx2(m_nPort, m_pAddr, ADSIGRP_SYM_VALBYHND, lHdlVar, sizeof(T) * array_data_length, &myArray[0], &pcbReturn); //��������ǹ̶���С��������
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
