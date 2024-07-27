#include "EPOS2.h"
#include <stdio.h>
#include <fstream>
#include <sstream>
#include <iostream>
#include "Logger.h"

using namespace Device;

EPOS2::EPOS2(unsigned short NodeId): m_Mode(0), m_ErrorCode(0),m_KeyHandle(0),m_NodeId(NodeId),
m_BaudRate(1000000), m_Timeout_ms(500), m_IsIPMStop(FALSE), m_IPMThread(nullptr), m_IsRecorderStop(FALSE)
{
	memset(&m_SafeParam, 0, sizeof(EPOS2SafetyParam));
	memset(&m_PositionProfileParam, 0, sizeof(PositionProfileParam));
	memset(&m_VelocityProfileParam, 0, sizeof(VelocityProfileParam));
	memset(&m_HomeParam, 0, sizeof(HomingParam));
	memset(&m_IPMParam, 0, sizeof(IPMParam));
	memset(&m_RecorderParam, 0, sizeof(RecorderParam));
	
	// read sdk version
	DWORD err_code = 0;
	VCS_GetDriverInfo(m_libname, sizeof(m_libname), m_libversion, sizeof(m_libversion), &err_code);
}

EPOS2::~EPOS2()
{
	CloseDevice();
}

int EPOS2::OpenDevice()
{
	if (m_KeyHandle)
	{
		LOG_INFO("Device[%d] already open!", m_NodeId);
		return TRUE;
	}
	void * hNewKeyHandle = NULL;
#ifdef _WIN32
	char device_name[] = "EPOS2";
	char protocol_name[] = "MAXON SERIAL V2";
	char interface_name[] = "USB";
	char port_name[] = "USB0";
	hNewKeyHandle = VCS_OpenDevice(device_name, protocol_name, interface_name, port_name, &m_ErrorCode);
	//hNewKeyHandle = VCS_OpenDevice("EPOS2", "CANopen", "IXXAT_USB-to-CAN V2 compact 0", "CAN0", &m_ErrorCode);
	//hNewKeyHandle = VCS_OpenDeviceDlg(&m_ErrorCode);
#else
	hNewKeyHandle = VCS_OpenDevice("EPOS2", "MAXON SERIAL V2", "USB", "USB0", &m_ErrorCode);
#endif
	if(hNewKeyHandle)
	{
		// Close Previous Device
		if(m_KeyHandle) VCS_CloseDevice(m_KeyHandle, &m_ErrorCode);
		m_KeyHandle = hNewKeyHandle;
		// Set protocol
		if (!VCS_SetProtocolStackSettings(m_KeyHandle, m_BaudRate, m_Timeout_ms, &m_ErrorCode))
		{
			ShowErrorInformation(m_ErrorCode);
			return FALSE;
		}
		// Show node version
		ReadNodeVersion();
		// Read params
		ReadNodeParams();

		LOG_INFO("Open device[%d] success!", m_NodeId);
	}
	else
	{
		LOG_ERROR("Can't open device[%d]!", m_NodeId);
		ShowErrorInformation(m_ErrorCode);
		return FALSE;
	}
	return TRUE;
}

int EPOS2::ReadNodeParams()
{
	if (!m_KeyHandle) return FALSE;
	// Safety param
	if (!VCS_GetMaxAcceleration(m_KeyHandle, m_NodeId, &(m_SafeParam.MaxAcceleration), &m_ErrorCode))
	{
		ShowErrorInformation(m_ErrorCode);
	}
	if (!VCS_GetMaxProfileVelocity(m_KeyHandle, m_NodeId, &(m_SafeParam.MaxProfileVelocity), &m_ErrorCode))
	{
		ShowErrorInformation(m_ErrorCode);
	}
	if (!VCS_GetMaxFollowingError(m_KeyHandle, m_NodeId, &(m_SafeParam.MaxFollowingError), &m_ErrorCode))
	{
		ShowErrorInformation(m_ErrorCode);
	}
	// Profile param
	if (!VCS_GetVelocityProfile(m_KeyHandle, m_NodeId, &(m_VelocityProfileParam.ProfileAcceleration),
		&(m_VelocityProfileParam.ProfileDeceleration), &m_ErrorCode))
	{
		ShowErrorInformation(m_ErrorCode);
	}
	if (!VCS_GetPositionProfile(m_KeyHandle, m_NodeId, &(m_PositionProfileParam.ProfileVelocity),
		&(m_PositionProfileParam.ProfileAcceleration), &(m_PositionProfileParam.ProfileDeceleration), &m_ErrorCode))
	{
		ShowErrorInformation(m_ErrorCode);
	}
	// Homing param
	if (!VCS_GetHomingParameter(m_KeyHandle, m_NodeId, &(m_HomeParam.HomingAcceleration), &(m_HomeParam.SpeedSwitch),
		&(m_HomeParam.SpeedIndex), &(m_HomeParam.HomeOffset), &(m_HomeParam.CurrentTreshold), &(m_HomeParam.HomePosition), &m_ErrorCode))
	{
		ShowErrorInformation(m_ErrorCode);
	}
	// IPM param
	if (!VCS_GetIpmBufferParameter(m_KeyHandle, m_NodeId, &(m_IPMParam.UnderFlowWarningLimit),
		&(m_IPMParam.OverFlowWarningLimit), &(m_IPMParam.MaxBufferSize), &m_ErrorCode))
	{
		ShowErrorInformation(m_ErrorCode);
	}
	return TRUE;
}

int EPOS2::ClearFault()
{
	int oFault = FALSE;
	if(!m_KeyHandle) return FALSE;

	if(!VCS_GetFaultState(m_KeyHandle, m_NodeId, &oFault, &m_ErrorCode))
	{
		ShowErrorInformation(m_ErrorCode);
		return FALSE;
	}

	if(oFault)
	{
		if(!VCS_ClearFault(m_KeyHandle, m_NodeId, &m_ErrorCode))
		{
			ShowErrorInformation(m_ErrorCode);
			return FALSE;
		}
	}
	return TRUE;
}

int EPOS2::CloseDevice()
{
	// Stop Recorder
	StopRecording();
	// Stop IPM
	StopIPMMode();
	// Quick Stop
	QuickStop();

	// Close Device
	if(m_KeyHandle)
	{
		if(VCS_CloseDevice(m_KeyHandle, &m_ErrorCode))
		{
			LOG_INFO("Device[%d] Closed!", m_NodeId);
			m_KeyHandle = NULL;
		}
		else
			ShowErrorInformation(m_ErrorCode);
	}
	return TRUE;
}

void EPOS2::ClearPVTQuque()
{
	IPMPVT * pvt = nullptr;
	m_PVTQueueLock.lock();
	while (!m_PVTQueue.empty())
	{
		pvt = m_PVTQueue.front();
		delete pvt;
		m_PVTQueue.pop();
	}
	m_PVTQueueLock.unlock();
}

int EPOS2::SetupNodeSafetyInfo(EPOS2SafetyParam & safety)
{
	m_SafeParam = safety;

	if (!m_KeyHandle) return FALSE;
	if (!VCS_SetMaxFollowingError(m_KeyHandle, m_NodeId, m_SafeParam.MaxFollowingError, &m_ErrorCode))
	{
		ShowErrorInformation(m_ErrorCode);
		return FALSE;
	}
	if (!VCS_SetMaxAcceleration(m_KeyHandle, m_NodeId, m_SafeParam.MaxAcceleration, &m_ErrorCode))
	{
		ShowErrorInformation(m_ErrorCode);
		return FALSE;
	}
	if (!VCS_SetMaxProfileVelocity(m_KeyHandle, m_NodeId, m_SafeParam.MaxProfileVelocity, &m_ErrorCode))
	{
		ShowErrorInformation(m_ErrorCode);
		return FALSE;
	}
	return TRUE;
}

int EPOS2::EnableNode()
{
	if(!m_KeyHandle) return FALSE;

	if(!ClearFault()) return FALSE;

	if(!VCS_SetEnableState(m_KeyHandle, m_NodeId, &m_ErrorCode))
	{
		ShowErrorInformation(m_ErrorCode);
		return FALSE;
	}
	return TRUE;
}

int EPOS2::DisableNode()
{
	if(!m_KeyHandle) return FALSE;

	if(!VCS_SetDisableState(m_KeyHandle, m_NodeId, &m_ErrorCode))
	{
		ShowErrorInformation(m_ErrorCode);
		return FALSE;
	}
	return TRUE;
}

int EPOS2::SetOperationMode( char OperationMode )
{
	if(!m_KeyHandle) return FALSE;

	if(!VCS_SetOperationMode(m_KeyHandle,m_NodeId,OperationMode,&m_ErrorCode))
	{
		ShowErrorInformation(m_ErrorCode);
		return FALSE;
	}
	return TRUE;
}

char EPOS2::GetOperationMode()
{
	if (!m_KeyHandle) return 0;

	if (!VCS_GetOperationMode(m_KeyHandle, m_NodeId, &m_Mode, &m_ErrorCode))
	{
		ShowErrorInformation(m_ErrorCode);
		return 0;
	}
	return m_Mode;
}

int EPOS2::ShowOperationMode()
{
	if(!m_KeyHandle) return FALSE;
	//Read Operation Mode
	if(VCS_GetOperationMode(m_KeyHandle, m_NodeId, &m_Mode, &m_ErrorCode))
	{
		printf("Current Operation Mode:\t");
		switch(m_Mode)
		{
		case OMD_PROFILE_POSITION_MODE:
			printf("Profile Position Mode\n\n");
			break;
		case OMD_PROFILE_VELOCITY_MODE:
			printf("Profile Velocity Mode\n\n");
			break;
		case OMD_HOMING_MODE:
			printf("Homing Mode\n\n");
			break;
		case OMD_INTERPOLATED_POSITION_MODE:
			printf("Interpolated Position Mode\n\n");
			break;
		case OMD_POSITION_MODE:
			printf("Position Mode\n\n");
			break;
		case OMD_VELOCITY_MODE:
			printf("Velocity Mode\n\n");
			break;
		case OMD_CURRENT_MODE:
			printf("Current Mode\n\n");
			break;
		case OMD_MASTER_ENCODER_MODE:
			printf("Master Encoder Mode\n\n");
			break;
		case OMD_STEP_DIRECTION_MODE:
			printf("Step Direction Mode\n\n");
			break;
		default:
			printf("Unknown Mode\n\n");
			break;
		}
		return TRUE;
	}
	else
	{
		ShowErrorInformation(m_ErrorCode);
		return FALSE;
	}
}

int EPOS2::ReadNodeVersion()
{
	int result = TRUE;
	if(!m_KeyHandle) return FALSE;

	//the firmware version
	if (!VCS_GetVersion(m_KeyHandle, m_NodeId, &m_hardwareVersion,
		&m_softwareVersion, &m_applicationNumber, &m_applicationVersion, &m_ErrorCode))
	{
		ShowErrorInformation(m_ErrorCode);
		result = FALSE;
	}
	return result;
}

int EPOS2::ShowErrorInformation( DWORD error_code )
{
	char* pStrErrorInfo;
	unsigned short strSize = 256;
	if((pStrErrorInfo = (char*)malloc(strSize)) == NULL)
	{
		LOG_ERROR("System Error: Not enough memory to allocate buffer for error information string.");
		return FALSE;
	}

	if(VCS_GetErrorInfo(error_code, pStrErrorInfo, strSize))
	{
		LOG_ERROR("[ERROR]: %s.",pStrErrorInfo);
		free(pStrErrorInfo);
		return TRUE;
	}
	else
	{
		free(pStrErrorInfo);
		LOG_ERROR("Error information can't be read!");
		return FALSE;
	}
}

void EPOS2::IPMThreadFunc(void * arg)
{
	EPOS2 *		epos2 = (EPOS2*)arg;
	IPMStatus	IPMStatus = { 0 };
	DWORD		ErrorCode = 0;
	DWORD		BufferSize = 0;
	IPMPVT *	PVT = nullptr;
	while (!epos2->m_IsIPMStop)
	{
		// read status
		if (!VCS_GetIpmStatus(epos2->m_KeyHandle, epos2->m_NodeId,
			&(IPMStatus.IsTrajectoryRunning),
			&(IPMStatus.IsUnderflowWarning), 
			&(IPMStatus.IsOverflowWarning), 
			&(IPMStatus.IsVelocityWarning),
			&(IPMStatus.IsAccelerationWarning), 
			&(IPMStatus.IsUnderflowError),
			&(IPMStatus.IsOverflowError),
			&(IPMStatus.IsVelocityError), 
			&(IPMStatus.IsAccelerationError),
			&ErrorCode))
		{
			ShowErrorInformation(ErrorCode);
			break;
		}
		if (IPMStatus.IsUnderflowError)
		{
			LOG_ERROR("Node: %d is UnderflowError!", epos2->m_NodeId);
			break;
		}
		if (IPMStatus.IsOverflowError)
		{
			LOG_ERROR("Node: %d is IsOverflowError!", epos2->m_NodeId);
			break;
		}
		if (IPMStatus.IsVelocityError)
		{
			LOG_ERROR("Node: %d is IsVelocityError!", epos2->m_NodeId);
			break;
		}
		if(IPMStatus.IsAccelerationError)
		{
			LOG_ERROR("Node: %d is IsAccelerationError!", epos2->m_NodeId);
			break;
		}
		if (/*!IPMStatus.IsTrajectoryRunning || IPMStatus.IsUnderflowWarning || */
			!IPMStatus.IsOverflowWarning)
		{
			if (!VCS_GetFreeIpmBufferSize(epos2->m_KeyHandle, epos2->m_NodeId, &BufferSize, &ErrorCode))
			{
				ShowErrorInformation(ErrorCode);
				break;
			}
			//LOG_ERROR("Free buffer size: %d\n", int(BufferSize));
			for (DWORD i = 0; i < BufferSize; i++)
			{
				// we should add pvt points now
				epos2->m_PVTQueueLock.lock();
				if (!epos2->m_PVTQueue.empty())
				{
					PVT = epos2->m_PVTQueue.front();
					epos2->m_PVTQueue.pop();
				}
				epos2->m_PVTQueueLock.unlock();
				if (PVT)
				{
					if (!VCS_AddPvtValueToIpmBuffer(epos2->m_KeyHandle, epos2->m_NodeId,
						PVT->Position, PVT->Velocity, PVT->Time, &ErrorCode))
					{
						ShowErrorInformation(ErrorCode);
						break;
					}
					LOG_ERROR("Adding new PVT(%d, %d, %d)", PVT->Position, PVT->Velocity, PVT->Time);

					delete PVT;
					PVT = nullptr;
				}
			}			
		}
		// stop trajectory if necessary
		if(IPMStatus.IsTrajectoryRunning && IPMStatus.IsUnderflowWarning)
		{
			epos2->m_PVTQueueLock.lock();
			if (epos2->m_PVTQueue.empty())
			{
				// No more pvt available, stop trajectory now
				epos2->StopTrajectory();
			}
			epos2->m_PVTQueueLock.unlock();
		}
		if(IPMStatus.IsOverflowWarning)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
		}
	}
	LOG_ERROR("IPM thread exited!");
}

int EPOS2::SetVelocityProfileParam(VelocityProfileParam &param)
{
	m_VelocityProfileParam = param;

	if (!m_KeyHandle) return FALSE;

	if (!VCS_SetVelocityProfile(m_KeyHandle, m_NodeId, m_VelocityProfileParam.ProfileAcceleration,
		m_VelocityProfileParam.ProfileDeceleration, &m_ErrorCode))
	{
		ShowErrorInformation(m_ErrorCode);
		return FALSE;
	}

	return TRUE;
}

int EPOS2::MoveV(long velocity)
{
	if(!m_KeyHandle) return FALSE;

	if(!SetOperationMode(OMD_PROFILE_VELOCITY_MODE)) return FALSE;
	
	if(!VCS_SetVelocityProfile(m_KeyHandle,m_NodeId, m_VelocityProfileParam.ProfileAcceleration, 
		m_VelocityProfileParam.ProfileDeceleration, &m_ErrorCode))
	{
		ShowErrorInformation(m_ErrorCode);
		return FALSE;
	}
	
	if(!VCS_ActivateProfileVelocityMode(m_KeyHandle,m_NodeId, &m_ErrorCode))
	{
		ShowErrorInformation(m_ErrorCode);
		return FALSE;
	}

	EnableNode();

	if(!VCS_MoveWithVelocity(m_KeyHandle,m_NodeId, velocity, &m_ErrorCode))
	{
		ShowErrorInformation(m_ErrorCode);
		return FALSE;
	}
	return TRUE;
}

int EPOS2::SetPositionProfileParam(PositionProfileParam &param)
{
	m_PositionProfileParam = param;

	if (!m_KeyHandle) return FALSE;

	if (!VCS_SetPositionProfile(m_KeyHandle, m_NodeId, m_PositionProfileParam.ProfileVelocity,
		m_PositionProfileParam.ProfileAcceleration, m_PositionProfileParam.ProfileDeceleration, &m_ErrorCode))
	{
		ShowErrorInformation(m_ErrorCode);
		return FALSE;
	}

	return TRUE;
}

int EPOS2::MoveP(long position, BOOL is_absolute)
{
	if(!m_KeyHandle) return FALSE;

	if (!SetOperationMode(OMD_PROFILE_POSITION_MODE)) return FALSE;

	if(!VCS_SetPositionProfile(m_KeyHandle,m_NodeId, m_PositionProfileParam.ProfileVelocity, 
		m_PositionProfileParam.ProfileAcceleration, m_PositionProfileParam.ProfileDeceleration, &m_ErrorCode))
	{
		ShowErrorInformation(m_ErrorCode);
		return FALSE;
	}

	if(!VCS_ActivateProfilePositionMode(m_KeyHandle,m_NodeId,&m_ErrorCode))
	{
		ShowErrorInformation(m_ErrorCode);
		return FALSE;
	}

	EnableNode();
	long StartPosition;

	if(VCS_GetPositionIs(m_KeyHandle, m_NodeId, &StartPosition, &m_ErrorCode))
	{
		if(!VCS_MoveToPosition(m_KeyHandle, m_NodeId, position, is_absolute, TRUE , &m_ErrorCode))
		{
			ShowErrorInformation(m_ErrorCode);
			return FALSE;
		}
	}
	return TRUE;
}

int EPOS2::LoadPVTData(const char * filename)
{
	std::ifstream infile(filename, std::ifstream::in);
	if (!infile.is_open())
	{
		LOG_ERROR("Load PVT data failed, file %s does not exists!\n", filename);
		return 0;
	}
	int count = 0;
	while (!infile.eof())
	{
		std::string s;
		std::getline(infile, s);
		if (s.empty())
		{
			continue;
		}
		IPMPVT * pvt = new IPMPVT();
		int t = 0;

		std::istringstream iss(s);
		iss >> pvt->Position >> pvt->Velocity;
		iss >> t;
		pvt->Time = t;

		m_PVTQueueLock.lock();
		m_PVTQueue.push(pvt);
		m_PVTQueueLock.unlock();
		count++;
	}
	infile.close();

	return count;
}

int EPOS2::SetIPMParam(IPMParam &param)
{
	m_IPMParam = param;

	if (!m_KeyHandle) return FALSE;

	if (!VCS_SetIpmBufferParameter(m_KeyHandle, m_NodeId, m_IPMParam.UnderFlowWarningLimit,
		m_IPMParam.OverFlowWarningLimit, &m_ErrorCode))
	{
		ShowErrorInformation(m_ErrorCode);
		return FALSE;
	}
	return TRUE;
}

int EPOS2::StartIPMMode()
{
	if (!m_KeyHandle) return FALSE;
	if (m_IPMThread) return FALSE;
	
	VCS_ClearFault(m_KeyHandle, m_NodeId, &m_ErrorCode);

	if (!SetOperationMode(OMD_INTERPOLATED_POSITION_MODE))
	{
		ShowErrorInformation(m_ErrorCode);
		return FALSE;
	}
	if (!VCS_SetIpmBufferParameter(m_KeyHandle, m_NodeId, m_IPMParam.UnderFlowWarningLimit,
		m_IPMParam.OverFlowWarningLimit, &m_ErrorCode))
	{
		ShowErrorInformation(m_ErrorCode);
		return FALSE;
	}
	if (!VCS_ClearIpmBuffer(m_KeyHandle, m_NodeId, &m_ErrorCode))
	{
		ShowErrorInformation(m_ErrorCode);
		return FALSE;
	}
	if (!VCS_ActivateInterpolatedPositionMode(m_KeyHandle, m_NodeId, &m_ErrorCode))
	{
		ShowErrorInformation(m_ErrorCode);
		return FALSE;
	}
	if (!VCS_SetEnableState(m_KeyHandle, m_NodeId, &m_ErrorCode))
	{
		ShowErrorInformation(m_ErrorCode);
		return FALSE;
	}
	m_IsIPMStop = FALSE;
	m_IPMThread = new std::thread(EPOS2::IPMThreadFunc, this);
	return TRUE;
}

int EPOS2::StartTrajectory()
{
	if (!m_KeyHandle) return FALSE;
	if (!VCS_StartIpmTrajectory(m_KeyHandle, m_NodeId, &m_ErrorCode))
	{
		ShowErrorInformation(m_ErrorCode);
		return FALSE;
	}
	return TRUE;
}

int EPOS2::StopTrajectory()
{
	if (!m_KeyHandle) return FALSE;
	if (!VCS_StopIpmTrajectory(m_KeyHandle, m_NodeId, &m_ErrorCode))
	{
		ShowErrorInformation(m_ErrorCode);
		return FALSE;
	}
	return TRUE;
}

int EPOS2::StopIPMMode()
{
	m_IsIPMStop = TRUE;
	if (m_IPMThread)
	{
		m_IPMThread->join();
		delete m_IPMThread;
		m_IPMThread = nullptr;
	}
	// clear PVT queue
	ClearPVTQuque();
	return TRUE;
}

int EPOS2::SetCurrentPositionAsHome()
{
	if(!m_KeyHandle) return FALSE;
	if(SetOperationMode(OMD_HOMING_MODE))
	{
		if (!VCS_SetHomingParameter(m_KeyHandle, m_NodeId, m_HomeParam.HomingAcceleration, m_HomeParam.SpeedSwitch, m_HomeParam.SpeedIndex,
			m_HomeParam.HomeOffset, m_HomeParam.CurrentTreshold, m_HomeParam.HomePosition, &m_ErrorCode))
		{
			ShowErrorInformation(m_ErrorCode);
			return FALSE;
		}
	}
	if(!VCS_ActivateHomingMode(m_KeyHandle,m_NodeId,&m_ErrorCode))
	{
		ShowErrorInformation(m_ErrorCode);
		return FALSE;
	}

	EnableNode();
	if (!VCS_FindHome(m_KeyHandle, m_NodeId, HM_ACTUAL_POSITION, &m_ErrorCode))
	{
		ShowErrorInformation(m_ErrorCode);
		return FALSE;
	}
	return TRUE;
}

int EPOS2::Halt()
{
	if(!m_KeyHandle) return FALSE;

	char mode = GetOperationMode();
	if (mode == OMD_PROFILE_VELOCITY_MODE)
	{
		if (!VCS_HaltVelocityMovement(m_KeyHandle, m_NodeId, &m_ErrorCode))
		{
			ShowErrorInformation(m_ErrorCode);
			return FALSE;
		}
	}
	else if (mode == OMD_PROFILE_POSITION_MODE)
	{
		if (!VCS_HaltPositionMovement(m_KeyHandle, m_NodeId, &m_ErrorCode))
		{
			ShowErrorInformation(m_ErrorCode);
			return FALSE;
		}
	}	
	return TRUE;
}

int EPOS2::WaitForMoveEnd(int timeout_ms)
{
	if (!m_KeyHandle) return FALSE;

	char mode = GetOperationMode();
	if (mode == OMD_PROFILE_VELOCITY_MODE || mode == OMD_PROFILE_POSITION_MODE)
	{
		if (!VCS_WaitForTargetReached(m_KeyHandle, m_NodeId, timeout_ms, &m_ErrorCode))
		{
			ShowErrorInformation(m_ErrorCode);
			return FALSE;
		}
	}
	return TRUE;
}

int EPOS2::QuickStop()
{
	if(!m_KeyHandle) return FALSE;
	if (!VCS_SetQuickStopState(m_KeyHandle, m_NodeId, &m_ErrorCode))
	{
		ShowErrorInformation(m_ErrorCode);
		return FALSE;
	}
	return TRUE;
}

int EPOS2::SetRecorderParam(RecorderParam &param)
{
	m_RecorderParam = param;
	return TRUE;
}

int EPOS2::StartRecording()
{
	if (!m_KeyHandle) return FALSE;

	if (!VCS_SetRecorderParameter(m_KeyHandle, m_NodeId, m_RecorderParam.SamplingPeriod, 
		m_RecorderParam.NbOfPrecedingSamples, &m_ErrorCode))
	{
		ShowErrorInformation(m_ErrorCode);
		return FALSE;
	}
	if (!VCS_EnableTrigger(m_KeyHandle, m_NodeId, m_RecorderParam.TriggerType, &m_ErrorCode))
	{
		ShowErrorInformation(m_ErrorCode);
		return FALSE;
	}
	for (BYTE i = 0; i < m_RecorderParam.ChannelSize; i++)
	{
		if (!VCS_ActivateChannel(m_KeyHandle, m_NodeId, m_RecorderParam.Channels[i].ChannelNumber, 
			m_RecorderParam.Channels[i].ObjectIndex, m_RecorderParam.Channels[i].ObjectSubIndex,
			m_RecorderParam.Channels[i].ObjectSize, &m_ErrorCode))
		{
			ShowErrorInformation(m_ErrorCode);
			return FALSE;
		}
	}
	if (!VCS_StartRecorder(m_KeyHandle, m_NodeId, &m_ErrorCode))
	{
		ShowErrorInformation(m_ErrorCode);
		return FALSE;
	}
	m_IsRecorderStop = FALSE;
	m_RecorderThread = std::thread(EPOS2::RecorderThreadFunc, this);
	return TRUE;
}

int EPOS2::StopRecording()
{
	if (!m_KeyHandle) return FALSE;
	if (m_RecorderThread.joinable())
	{
		m_IsRecorderStop = TRUE;
		m_RecorderThread.join();
	}
	if (!VCS_StopRecorder(m_KeyHandle, m_NodeId, &m_ErrorCode))
	{
		ShowErrorInformation(m_ErrorCode);
		return FALSE;
	}
	return TRUE;
}

void EPOS2::RecorderThreadFunc(void * arg)
{
	EPOS2 * epos2 = (EPOS2*)arg;
	BOOL IsRecorderRunning = FALSE;
	DWORD ErrorCode = 0;
	DWORD VectorSize[MAXON_EPOS2_DATA_RECORDER_MAX_CHANNEL] = { 0 };
	FILE * fp[MAXON_EPOS2_DATA_RECORDER_MAX_CHANNEL] = { 0 };

	char filename[256] = { 0 };
	for (BYTE i = 0; i < epos2->m_RecorderParam.ChannelSize; i++)
	{
		sprintf_s(filename, "Node_%d-Channel_%d.txt", int(epos2->m_NodeId), int(epos2->m_RecorderParam.Channels[i].ChannelNumber));
		fopen_s(&fp[i], filename, "w");
	}

	while (!epos2->m_IsRecorderStop)
	{
		if (!VCS_IsRecorderRunning(epos2->m_KeyHandle, epos2->m_NodeId, &IsRecorderRunning, &ErrorCode))
		{
			ShowErrorInformation(ErrorCode);
			break;
		}
		if (IsRecorderRunning)
		{
			if (!VCS_ReadChannelVectorSize(epos2->m_KeyHandle, epos2->m_NodeId, VectorSize, &ErrorCode))
			{
				ShowErrorInformation(ErrorCode);
				break;
			}
			for (BYTE i = 0; i < epos2->m_RecorderParam.ChannelSize; i++)
			{
				DWORD num = VectorSize[i];
				if (num > 0)
				{
					BYTE * data_points = (BYTE*)malloc(sizeof(BYTE) * num);
					if (VCS_ReadChannelDataVector(epos2->m_KeyHandle, epos2->m_NodeId, epos2->m_RecorderParam.Channels[i].ChannelNumber,
						data_points, num, &ErrorCode))
					{
						for (DWORD k = 0; k < num; k++)
						{
							fprintf(fp[i], "%d\n", int(data_points[k]));
						}
					}
					else
					{
						ShowErrorInformation(ErrorCode);
					}
					free(data_points);
				}
			}
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(5));
	}

	for (BYTE i = 0; i < epos2->m_RecorderParam.ChannelSize; i++)
	{
		fclose(fp[i]);
	}
	LOG_ERROR("Recorder thread exited!\n");
}

int EPOS2::FindHomeByPosLimit()
{
	if (!m_KeyHandle) return FALSE;

	if (!SetOperationMode(OMD_HOMING_MODE))
	{
		ShowErrorInformation(m_ErrorCode);
		return FALSE;
	}

	if (!VCS_SetHomingParameter(m_KeyHandle, m_NodeId, m_HomeParam.HomingAcceleration, m_HomeParam.SpeedSwitch, m_HomeParam.SpeedIndex,
		m_HomeParam.HomeOffset, m_HomeParam.CurrentTreshold, m_HomeParam.HomePosition, &m_ErrorCode))
	{
		ShowErrorInformation(m_ErrorCode);
		return FALSE;
	}

	if (!VCS_ActivateHomingMode(m_KeyHandle, m_NodeId, &m_ErrorCode))
	{
		ShowErrorInformation(m_ErrorCode);
		return FALSE;
	}

	EnableNode();

	if (!VCS_FindHome(m_KeyHandle, m_NodeId, HM_POSITIVE_LIMIT_SWITCH, &m_ErrorCode))
	{
		ShowErrorInformation(m_ErrorCode);
		return FALSE;
	}
	return TRUE;
}

int EPOS2::FindHomeByNegLimit()
{
	if (!m_KeyHandle) return FALSE;

	if (!SetOperationMode(OMD_HOMING_MODE)) return FALSE;

	if (!VCS_SetHomingParameter(m_KeyHandle, m_NodeId, m_HomeParam.HomingAcceleration, m_HomeParam.SpeedSwitch, m_HomeParam.SpeedIndex,
		m_HomeParam.HomeOffset, m_HomeParam.CurrentTreshold, m_HomeParam.HomePosition, &m_ErrorCode))
	{
		ShowErrorInformation(m_ErrorCode);
		return FALSE;
	}

	if (!VCS_ActivateHomingMode(m_KeyHandle, m_NodeId, &m_ErrorCode))
	{
		ShowErrorInformation(m_ErrorCode);
		return FALSE;
	}

	EnableNode();

	if (!VCS_FindHome(m_KeyHandle, m_NodeId, HM_NEGATIVE_LIMIT_SWITCH, &m_ErrorCode))
	{
		ShowErrorInformation(m_ErrorCode);
		return FALSE;
	}
	return TRUE;
}

int EPOS2::WaitForHomeEnd()
{
	if (!m_KeyHandle) return FALSE;

	char mode = GetOperationMode();
	if (mode == OMD_HOMING_MODE)
	{
		if (!VCS_WaitForHomingAttained(m_KeyHandle, m_NodeId, 0xffffff, &m_ErrorCode))
		{
			ShowErrorInformation(m_ErrorCode);
			return FALSE;
		}
		if (!VCS_SetQuickStopState(m_KeyHandle, m_NodeId, &m_ErrorCode))
		{
			ShowErrorInformation(m_ErrorCode);
			return FALSE;
		}
	}
	return TRUE;
}

int EPOS2::StopHoming()
{
	if (!m_KeyHandle) return FALSE;

	char mode = GetOperationMode();
	if (mode == OMD_HOMING_MODE)
	{
		if (!VCS_StopHoming(m_KeyHandle, m_NodeId, &m_ErrorCode))
		{
			ShowErrorInformation(m_ErrorCode);
			return FALSE;
		}
	}
	return TRUE;
}

int EPOS2::WriteNodeParams()
{
	if (!SetupNodeSafetyInfo(m_SafeParam))
	{
		return FALSE;
	}
	if (!SetVelocityProfileParam(m_VelocityProfileParam))
	{
		return FALSE;
	}
	if (!SetPositionProfileParam(m_PositionProfileParam))
	{
		return FALSE;
	}
	if (!SetHomingParam(m_HomeParam))
	{
		return FALSE;
	}
	if (!SetIPMParam(m_IPMParam))
	{
		return FALSE;
	}
	return TRUE;
}

int EPOS2::GetState()
{
	if (!m_KeyHandle) return -1;
	WORD state = 0; // ST_DISABLED, ST_ENABLED, ST_QUICKSTOP, ST_FAULT
	if (!VCS_GetState(m_KeyHandle, m_NodeId, &state, &m_ErrorCode))
	{
		ShowErrorInformation(m_ErrorCode);
		return -1;
	}
	return state;
}

int EPOS2::GetPosition()
{
	if (!m_KeyHandle) return 0;
	long pos = 0;
	if (!VCS_GetPositionIs(m_KeyHandle, m_NodeId, &pos, &m_ErrorCode))
	{
		ShowErrorInformation(m_ErrorCode);
		return 0;
	}
	return pos;
}

int EPOS2::GetVelocity()
{
	if (!m_KeyHandle) return 0;
	long v = 0;
	if (!VCS_GetVelocityIs(m_KeyHandle, m_NodeId, &v, &m_ErrorCode))
	{
		ShowErrorInformation(m_ErrorCode);
		return 0;
	}
	return v;
}

int EPOS2::GetMoveState()
{
	if (!m_KeyHandle) return 1; // 默认返回1
	BOOL state = 0;
	if (!VCS_GetMovementState(m_KeyHandle, m_NodeId, &state, &m_ErrorCode))
	{
		ShowErrorInformation(m_ErrorCode);
		return 0;
	}
	return state;
}

int EPOS2::GetTargetPosition()
{
	if (!m_KeyHandle) return 0;
	long v = 0;
	if (!VCS_GetTargetPosition(m_KeyHandle, m_NodeId, &v, &m_ErrorCode))
	{
		ShowErrorInformation(m_ErrorCode);
		return 0;
	}
	return v;
}

int EPOS2::GetTargetVelocity()
{
	if (!m_KeyHandle) return 0;
	long v = 0;
	if (!VCS_GetTargetVelocity(m_KeyHandle, m_NodeId, &v, &m_ErrorCode))
	{
		ShowErrorInformation(m_ErrorCode);
		return 0;
	}
	return v;
}

int EPOS2::GetDigitalInputs(DigitalInputs& di)
{
	if (!m_KeyHandle) return FALSE;
	if (!VCS_GetAllDigitalInputs(m_KeyHandle, m_NodeId, (WORD*)&di, &m_ErrorCode))
	{
		ShowErrorInformation(m_ErrorCode);
		return 0;
	}
	return TRUE;
}

int EPOS2::SetHomingParam(HomingParam& param)
{
	m_HomeParam = param;

	if (!m_KeyHandle) return FALSE;

	if (!VCS_SetHomingParameter(m_KeyHandle, m_NodeId, m_HomeParam.HomingAcceleration, m_HomeParam.SpeedSwitch, m_HomeParam.SpeedIndex,
		m_HomeParam.HomeOffset, m_HomeParam.CurrentTreshold, m_HomeParam.HomePosition, &m_ErrorCode))
	{
		ShowErrorInformation(m_ErrorCode);
		return FALSE;
	}
	return TRUE;
}

int EPOS2::EnablePosLimitSwitch(bool enable)
{
	if (!m_KeyHandle) return FALSE;
	// 是否启用该IO
	BOOL Mask = enable ? 1 : 0;
	// 设置limit触发模式：高电平触发或低电平触发
	BOOL Polarity = 0; // 1: Low active 0: High active
	if (!VCS_DigitalInputConfiguration(m_KeyHandle, m_NodeId, 5, DIC_POSITIVE_LIMIT_SWITCH, Mask, Polarity, 0, &m_ErrorCode))
	{
		ShowErrorInformation(m_ErrorCode);
		return FALSE;
	}
	return TRUE;
}

int EPOS2::EnableNegLimitSwitch(bool enable)
{
	if (!m_KeyHandle) return FALSE;
	// 是否启用该IO
	BOOL Mask = enable ? 1 : 0;
	// 设置limit触发模式：高电平触发或低电平触发
	BOOL Polarity = 0; // 1: Low active 0: High active
	if (!VCS_DigitalInputConfiguration(m_KeyHandle, m_NodeId, 6, DIC_NEGATIVE_LIMIT_SWITCH, Mask, Polarity, 0, &m_ErrorCode))
	{
		ShowErrorInformation(m_ErrorCode);
		return FALSE;
	}
	return TRUE;
}

