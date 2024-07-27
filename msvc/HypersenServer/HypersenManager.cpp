#include "HypersenManager.h"
#include "Logger.h"

enum class SENSOR_CMD
{
	CMD_NONE = 0x0,
	CMD_READ_DEV_ID,
	CMD_CONTINUOUS_READ,
	CMD_STOP_READ,
	CMD_SINGLE_READ,
	CMD_SET_FREQ,
	CMD_SET_BAUDRATE,
	CMD_RESTORE_USER_SETTINGS,
	CMD_RESTORE_FACTORY_SETTINGS,
	CMD_STORE_USER_SETTINGS,
	CMD_GET_VERSION,
	CMD_RESET_ZERO,
	CMD_RESET_DEVICE_ADDR = 0xD,
	CMD_SET_DEVICE_ADDR = 0xE,
	CMD_SET_LOW_FILTER_PARAM = 0x18,
	CMD_GET_OVERFLOW_NUM = 0xD4,
	CMD_GET_OVERFLOW_VALUE = 0xD8
};


HypersenManager::HypersenManager(CCPSAPI* cps_api, const HypersenCfg& cfg): m_sc(cfg), m_cps_api(cps_api)
{

}

HypersenManager::~HypersenManager()
{
	StopService();
}

bool HypersenManager::LaunchService()
{
	m_exit_flag = false;
	m_thread_handle = std::move(std::thread(&HypersenManager::ReadThreadFunc, this));
	printf("��ȡ�߳�������\n");
	return true;
}

void HypersenManager::StopService()
{
	m_exit_flag = true;
	if (m_thread_handle.joinable())
	{
		m_thread_handle.join();
	}
	DestroyComConnection();
}

bool HypersenManager::IsSensorOK()
{
	return m_is_sensor_ok;
}

bool HypersenManager::IsReading()
{
	return m_sensor_status.status;
	//return m_sensor_status.status;
	//return true;
}

const ST_HypersenSensorData& HypersenManager::GetCurrentSensorData()
{
	std::lock_guard<std::mutex> lock(m_sensor_lock);
	return m_cur_sensor_data;
}

const ST_HypersenSensorInfo& HypersenManager::GetSensorInfo()
{
	return m_sensor_info;
}

const ST_HypersenSensorStatus& HypersenManager::GetSensorStatus()
{
	return m_sensor_status;
}

void HypersenManager::ReadSensorDevID()
{
	char cmd[10] = { 0xF6, 0x6F, 0x03, 0x00, 0x00, 0x01, 0xBD, 0xDC, 0x6F, 0xF6 };
	m_com->write(cmd, sizeof(cmd));
	/*std::this_thread::sleep_for(std::chrono::milliseconds(500));
	std::vector<char> rsp = m_com->read();
	if (rsp.size() != 12)
	{
		LOG_ERROR("failed to read sensor id.");
		return 0;
	}
	union {
		uint16_t id;
		char bytes[2];
	} id;
	id.bytes[0] = rsp[6];
	id.bytes[1] = rsp[7];
	return id.id;*/
}

void HypersenManager::ReadVersion()
{
	char cmd[10] = { 0xF6, 0x6F, 0x03, 0x00, 0x00, 0x0A, 0xD6, 0x6D, 0x6F, 0xF6 };
	m_com->write(cmd, sizeof(cmd));
}

void HypersenManager::StartRead()
{
	/*���������ϵ����ҪԤ��һ��ʱ�����ȶ��������������ߵĲ���Ƶ�ʹ��� 10~20 ���ӣ�ʹ�ڲ�����
	����ƽ�������㸴λ�����λ��ɺ��ٿ�ʼʹ�á�
	*/
	char cmd[10] = { 0xF6, 0x6F, 0x03, 0x00, 0x00, 0x02, 0xDE, 0xEC, 0x6F, 0xF6 };
	m_com->write(cmd, sizeof(cmd));
	printf("���Ϳ�ʼ�������");
}

void HypersenManager::StopRead()
{
	/*
	ע�⣺
	�����������ǰ������������ģʽ���ڷ���ֹͣ��������ǰ����Ҫ�ȷ��� 50 ���ֽڵ� 0x00 ռ�� RS-485
	���ߣ��������յ���Щ�ֽں����ʱ�ͷ� RS-485 ���ߡ������ڷ����� 50 ���ֽڵ� 0x00 ���ݺ���ʱԼ 200ms
	����ֹͣ�����������ȷֹͣ�������������������ڽ��յ�ֹͣ��������󣬻᷵��һ��Ӧ�����ݰ�����
	ʾ�Ѿ���ȷֹͣ�˲�������������ڷ����� 50 ���ֽڵ� 0x00 ���ݺ� 250ms �ڻ�δ����ֹͣ���������
	�������Զ������������������������������������������ģʽ�£��ڷ��������κ�����ǰ��������ֹͣ����
	����������������ȷֹͣ�����������ȷ�������������
	*/
	char pre_cmd[50] = { 0 };
	m_com->write(pre_cmd, sizeof(pre_cmd));
	std::this_thread::sleep_for(std::chrono::milliseconds(200));
	char cmd[10] = { 0xF6, 0x6F, 0x03, 0x00, 0x00, 0x03, 0xFF, 0xFC, 0x6F, 0xF6 };
	m_com->write(cmd, sizeof(cmd));
}

void HypersenManager::ResetZero()
{
	char cmd[10] = { 0xF6, 0x6F, 0x03, 0x00, 0x00, 0x0B, 0xF7, 0x7D, 0x6F, 0xF6 };
	m_com->write(cmd, sizeof(cmd));
}

void HypersenManager::ReadThreadFunc()
{
	constexpr size_t SENSOR_HEADER_LEN = 5;
	constexpr size_t SENSOR_DATA_MAX_LEN = 64;
	while (!m_exit_flag)
	{
		if (!CheckComConnection())
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			printf("δ���ӣ�\n");
			continue;
		}
		// read head
		size_t buf_len = m_com->bytesRead();
		if (buf_len < SENSOR_HEADER_LEN)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			//printf("read head... buf_len=%d\n", buf_len);
			continue;
		}
		char head_buf[SENSOR_HEADER_LEN] = { 0 };
		m_com->read(head_buf, SENSOR_HEADER_LEN);

		size_t body_len = head_buf[2] + 2;
		// read body
		while (!m_exit_flag)
		{
			if (m_com->bytesRead() < body_len)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
				continue;
			}
			else
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(1));
				break;
			}
		}
		char body_buf[SENSOR_DATA_MAX_LEN] = { 0 };
		m_com->read(body_buf, body_len);

		char ccmd = body_buf[0];
		SENSOR_CMD cmd = static_cast<SENSOR_CMD>(ccmd);		
		switch (cmd)
		{
			case SENSOR_CMD::CMD_READ_DEV_ID:
			{
				m_sensor_info.dev_id = (uint16_t(body_buf[2]) << 1) + body_buf[1];
				printf("���豸��Ϣ...\n");
			}
			break;
			case SENSOR_CMD::CMD_CONTINUOUS_READ:
			{
				std::lock_guard<std::mutex> lock(m_sensor_lock);
				//printf("������ȡ��...\n");
				m_cur_sensor_data.code = head_buf[4];
				//printf("\n=====%#X======\n", (int)head_buf[1]);
				//printf("=====%#X======\n", ((int)head_buf[2]) << 0);
				//printf("=====%#X======\n", ((int)head_buf[3]) << 0);
				//printf("=====%#X======\n", ((int)head_buf[4]) << 0);
				//printf("+++++++%#X+++++\n", (int)head_buf[1] + (((int)head_buf[2]) << 8) + (((int)head_buf[3]) << 16) + (((int)head_buf[4]) << 24));
				for (unsigned int i = 0; i < HYPERSEN_SENSOR_DOF; i++)
				{
					//printf("=====%#X====== size= %d\n", (unsigned char)body_buf[i * 4 + 1],sizeof(body_buf[i * 4 + 1]));
					//printf("=====%#X======\n", (unsigned char)body_buf[i * 4 + 2]);
					//printf("=====%#X======\n", (unsigned char)body_buf[i * 4 + 3]);
					//printf("=====%#X======\n", (unsigned char)body_buf[i * 4 + 4]);
					//printf("siez= %d\n", sizeof(body_buf[i * 4 + 4]));
					//printf("=====��%#X======\n", (unsigned char)body_buf[i * 4 + 1] + (((unsigned char)body_buf[i * 4 + 2]) << 8) + (((unsigned char)body_buf[i * 4 + 3]) << 16) + (((unsigned char)body_buf[i * 4 + 4]) << 24));
					m_cur_sensor_data.data[i] = (float)((unsigned char)body_buf[i * 4 + 1] + (((unsigned char)body_buf[i * 4 + 2]) << 8) + (((unsigned char)body_buf[i * 4 + 3]) << 16) + (((unsigned char)body_buf[i * 4 + 4]) << 24)) / 1000.0f;
				}
				if (m_cur_sensor_data.code == 0)
				{
					m_sensor_status.status = 1; // reading
				}
				else
				{
					m_sensor_status.status = 2; // error
				}
			}
			break;
			case SENSOR_CMD::CMD_STOP_READ:
			{
				char stop_flag = body_buf[1];
				if (stop_flag == 0x01)
				{
					m_sensor_status.status = 0; // stopped
					LOG_INFO("stop success!");
				}
				else
				{
					m_sensor_status.status = 2; // error
					LOG_ERROR("stop failed!");
				}
				//printf("ֹͣ��ȡ...\n");
			}
			break;
			case SENSOR_CMD::CMD_GET_VERSION:
			{
				memcpy(m_sensor_info.version, body_buf + 1, 6);

				m_cps_api->SendDeviceMsg(-1, MSG_HYPERSEN_SENSOR_INFO, (const char*)&m_sensor_info, sizeof(ST_HypersenSensorInfo));
				//printf("��ȡ�汾��...\n");
			}
			break;
			case SENSOR_CMD::CMD_RESET_ZERO:
			{
				char reset_zero_flag = body_buf[1];
				if (reset_zero_flag == 0x01)
				{
					LOG_INFO("reset zero success!");
				}
				else
				{
					LOG_ERROR("reset zero failed!");
				}
				//printf("��������...\n");
			}
			break;
			default:
			{
				LOG_ERROR("unprocessed response for cmd=%c", ccmd);
				//printf("Ĭ��...\n");
			}
			break;
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}

bool HypersenManager::CheckComConnection()
{
	try
	{
		if (!m_com)
		{
			m_com = new BufferedAsyncSerial();
		}
		if (!m_com->isOpen())
		{
			m_com->open(m_sc.com_name, m_sc.baud_rate, boost::asio::serial_port_base::parity(boost::asio::serial_port_base::parity::even));
			printf("�򿪴���\n");
		}
		if (m_com->errorStatus())
		{
			DestroyComConnection();
			m_is_sensor_ok = false;
			return false;
		}
		m_is_sensor_ok = true;
		return true;
	}
	catch (const std::exception& e)
	{
		printf("open com: %s failed, %s\n", m_sc.com_name, e.what());
	}
	m_is_sensor_ok = false;
	return false; 	
}

void HypersenManager::DestroyComConnection()
{
	if (m_com)
	{
		try
		{
			m_com->close();
		}
		catch (...)
		{
		}
		delete m_com;
		m_com = nullptr;
	}
}
