#include "SensorManager.h"
#include <boost/algorithm/string.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/asio.hpp>
#include <vector>

SensorManager::SensorManager(const SensorCfg& sc):m_sc(sc)
{
}

SensorManager::~SensorManager()
{
	StopService();
}

bool SensorManager::LaunchService()
{
	m_exit_flag = false;
	m_thread_handle = std::move(std::thread(&SensorManager::ReadThreadFunc, this));
	return true;
}

void SensorManager::StopService()
{
	m_exit_flag = true;
	if (m_thread_handle.joinable())
	{
		m_thread_handle.join();
	}
	DestroyComConnection();
}

bool SensorManager::IsSensorOK()
{
	return m_is_sensor_ok;
}

ST_SensorData SensorManager::GetCurrentSensorData()
{
	std::lock_guard<std::mutex> lock(m_sensor_lock);
	if (!m_sensor_data_strs.empty())
	{
		const std::string & line = m_sensor_data_strs.back();
		DecodeStr(line, &m_cur_sensor_data);
	}
	return m_cur_sensor_data;
}

bool SensorManager::DecodeStr(const std::string& line, ST_SensorData* data)
{
	std::vector<std::string> nums;
	boost::split(nums, line, boost::is_any_of(","));
	if (nums.size() == m_sc.sensor_num)
	{
		try
		{
			data->sensor_num = nums.size();
			for (size_t i = 0; i < nums.size() && i < MAX_SENSOR_DATA_NUM; i++)
			{
				data->data[i] = boost::lexical_cast<float>(nums[i]);
			}
			return true;
		}
		catch (const std::exception& e)
		{
			printf("parse %s failed: %s\n", line.c_str(), e.what());
		}
	}
	return false;
}

void SensorManager::RetrieveAllData(std::vector<std::string>& data)
{
	std::lock_guard<std::mutex> lock(m_sensor_lock);
	m_sensor_data_strs.swap(data);
}

void SensorManager::ReadThreadFunc()
{
	while (!m_exit_flag)
	{
		if (!CheckComConnection())
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
			continue;
		}
		std::string line = m_com->readStringUntil(m_sc.delim);
		if (!line.empty())
		{
			{
				std::lock_guard<std::mutex> lock(m_sensor_lock);
				m_sensor_data_strs.push_back(line);
			}
			if (m_sc.is_print_raw_string)
			{
				printf("%s\n", line.c_str());
			}
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
}

void SensorManager::DestroyComConnection()
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

bool SensorManager::CheckComConnection()
{
	try
	{
		if (!m_com)
		{
			m_com = new BufferedAsyncSerial();
		}
		if (!m_com->isOpen())
		{
			m_com->open(m_sc.com_name, m_sc.baud_rate);
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
