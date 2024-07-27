#include "CPSHandler.h"
#include "HypersenManager.h"
#include "Logger.h"

void Handler::OnMsg(uint32_t from_id, uint32_t msg_type, const char* data, uint32_t msg_len)
{
	switch (msg_type)
	{
	case MSG_HYPERSEN_GET_SENSOR_INFO:
	{
		m_mng->ReadSensorDevID();
		std::this_thread::sleep_for(std::chrono::milliseconds(10));
		m_mng->ReadVersion();
	}
		break;
	case MSG_HYPERSEN_START_READ:
		m_mng->StartRead();
		break;
	case MSG_HYPERSEN_STOP_READ:
		m_mng->StopRead();
		break;
	case MSG_HYPERSEN_RESET_ZERO:
		m_mng->ResetZero();
		break;
	case MSG_HYPERSEN_READ_STATUS:
	{
		ST_HypersenSensorStatus status = m_mng->GetSensorStatus();
		m_api->SendDeviceMsg(from_id, MSG_HYPERSEN_STATUS, (const char*)&status, sizeof(ST_HypersenSensorStatus));
	}
		break;
	default: 
		LOG_ERROR("Unrecognized msg type: %d", msg_type);
		break;
	}
}

void Handler::SendSensorInfo(uint32_t app_id, ST_HypersenSensorInfo* data)
{
	if (m_api)
	{
		m_api->SendDeviceMsg(app_id, MSG_HYPERSEN_SENSOR_INFO, (const char*)data, sizeof(ST_HypersenSensorInfo));
	}
}

void Handler::SendSensorData(uint32_t app_id, ST_HypersenSensorData* data)
{
	if (m_api)
	{
		m_api->SendDeviceMsg(app_id, MSG_HYPERSEN_SENSOR_DATA, (const char*)data, sizeof(ST_HypersenSensorData));
	}
}