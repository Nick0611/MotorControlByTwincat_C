#pragma once
#include <CPSAPI/CPSAPI.h>
#include "CPSHypersenServerDef.h"

class HypersenManager;

class Handler : public CCPSEventHandler
{
public:
	Handler(CCPSAPI* api, HypersenManager * mng) :m_api(api), m_mng(mng)
	{}
	// Connect event
	virtual void OnConnected()
	{
		m_api->RegisterDevice();
	}
	// Disconnect event
	virtual void OnDisconnected()
	{
	}
	// Message event
	virtual void OnMsg(uint32_t from_id, uint32_t msg_type, const char* data, uint32_t msg_len);

	void SendSensorInfo(uint32_t app_id, ST_HypersenSensorInfo* data);
	void SendSensorData(uint32_t app_id, ST_HypersenSensorData* data);
private:
	CCPSAPI* m_api;
	HypersenManager* m_mng;
};
