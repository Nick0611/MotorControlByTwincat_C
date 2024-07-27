#pragma once
#include <CPSAPI/CPSAPI.h>
#include "CPSSensorServerDef.h"

class Handler : public CCPSEventHandler
{
public:
	Handler(CCPSAPI* api) :m_api(api)
	{}
	// Connect event
	virtual void OnConnected()
	{
		m_api->RegisterDevice();
		printf("OnConnected\n");
	}
	// Disconnect event
	virtual void OnDisconnected()
	{
		printf("OnDisconnected\n");
	}
	// Message event
	virtual void OnMsg(uint32_t from_id, uint32_t msg_type, const char* data, uint32_t msg_len)
	{
	}

	void SendSensorData(uint32_t app_id, ST_SensorData* data)
	{
		if (m_api)
		{
			m_api->SendDeviceMsg(app_id, MSG_SENSOR_DATA, (const char*)data, sizeof(ST_SensorData));
		}
	}
private:
	CCPSAPI* m_api;
};
