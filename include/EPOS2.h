#ifndef EPOS_H__
#define EPOS_H__

#define _USE_MATH_DEFINES
#include <iostream>
#include "Definitions.h"
#include <thread>
#include <queue>
#include <mutex>
#include <math.h>
#include <string>

#ifdef _WIN32
#pragma comment(lib,"EposCmd.lib")
#endif

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif

#ifndef _WIN32
#define DWORD		unsigned int
#endif

#if 1 // EPOS2 24-2 小号电机参数-maxon DCX 16S电机参数
#define MAXON_EPOS2_GERA_REDUCTION		62		// 减速箱减速比
#define MAXON_EPOS2_QC_PER_CIRCLE		1024*4*MAXON_EPOS2_GERA_REDUCTION	// maxon末端每圈qc数目
#define MAXON_EPOS2_MAX_VELOCITY		9430	// maxon最大转速，单位rpm
#else // EPOS2 24-5 大号电机参数-4_RE40 24V+GP42C 26-1+HEDL500线编码器(1)
#define MAXON_EPOS2_GERA_REDUCTION		26		// 减速箱减速比
#define MAXON_EPOS2_QC_PER_CIRCLE		500*4*MAXON_EPOS2_GERA_REDUCTION	// maxon末端每圈qc数目
#define MAXON_EPOS2_MAX_VELOCITY		6940	// maxon最大转速，单位rpm
#endif

#define MAXON_EPOS2_IPM_MAX_BUFFER		64		// IPM buffer最大值
#define MAXON_EPOS2_IPM_MIN_TIME_DELTA	1		// IPM 时间点最小值，单位ms
#define MAXON_EPOS2_IPM_MAX_TIME_DELTA	255		// IPM 时间点最大值，单位ms

#define MAXON_EPOS2_DATA_RECORDER_MAX_CHANNEL 4

namespace Device {

	inline double QC2Rad(int qc)
	{
		return qc * 2 * M_PI / MAXON_EPOS2_QC_PER_CIRCLE;
	}

	inline double QC2Deg(int qc)
	{
		return qc * 360.0 / MAXON_EPOS2_QC_PER_CIRCLE;
	}

	inline int Deg2QC(double deg)
	{
		return int(deg / 360.0 * MAXON_EPOS2_QC_PER_CIRCLE);
	}

	struct EPOS2SafetyParam 
	{
		DWORD MaxFollowingError;
		DWORD MaxAcceleration;
		DWORD MaxProfileVelocity;
	};

	struct HomingParam
	{
		DWORD HomingAcceleration;
		DWORD SpeedSwitch;
		DWORD SpeedIndex;
		long HomeOffset;
		WORD CurrentTreshold;
		long HomePosition;
	};

	struct PositionProfileParam
	{
		DWORD ProfileAcceleration;
		DWORD ProfileDeceleration;
		DWORD ProfileVelocity;
	};

	struct VelocityProfileParam
	{
		DWORD ProfileAcceleration;
		DWORD ProfileDeceleration;
	};

	struct IPMParam
	{
		DWORD MaxBufferSize;
		WORD UnderFlowWarningLimit;
		WORD OverFlowWarningLimit;
	};

	struct IPMStatus
	{
		BOOL IsTrajectoryRunning;
		BOOL IsUnderflowWarning;
		BOOL IsOverflowWarning;
		BOOL IsVelocityWarning;
		BOOL IsAccelerationWarning;
		BOOL IsUnderflowError;
		BOOL IsOverflowError;
		BOOL IsVelocityError;
		BOOL IsAccelerationError;
	};

	struct IPMPVT
	{
		long Position;		// unit: qc
		long Velocity;		// unit: rpm
		BYTE Time;			// unit: ms, range: (0, 255)
	};

	struct RecorderChannelInfo
	{
		BYTE ChannelNumber;
		WORD ObjectIndex;
		BYTE ObjectSubIndex;
		BYTE ObjectSize;
	};

	struct RecorderParam
	{
		WORD SamplingPeriod;
		WORD NbOfPrecedingSamples;
		BYTE TriggerType;
		BYTE ChannelSize;
		RecorderChannelInfo Channels[MAXON_EPOS2_DATA_RECORDER_MAX_CHANNEL];
	};

	struct DigitalInputs
	{
		WORD DI_NEGATIVE_LIMIT_SWITCH : 1;		//Bit0
		WORD DI_POSITIVE_LIMIT_SWITCH : 1;		//Bit1
		WORD DI_HOME_SWITCH	: 1;				//Bit2
		WORD DI_POSITION_MARKER	: 1;			//Bit3
		WORD DI_DRIVE_ENABLE : 1;				//Bit4
		WORD DI_QUICK_STOP : 1;					//Bit5
		WORD DI_TOUCH_PROBE1 : 1;				//Bit6
		WORD DI_NOT_USED : 1;					//Bit7
		WORD DI_GENERAL_PURPOSE_H : 1;			//Bit8
		WORD DI_GENERAL_PURPOSE_G : 1;			//Bit9
		WORD DI_GENERAL_PURPOSE_F : 1;			//Bit10
		WORD DI_GENERAL_PURPOSE_E : 1;			//Bit11
		WORD DI_GENERAL_PURPOSE_D : 1;			//Bit12
		WORD DI_GENERAL_PURPOSE_C : 1;			//Bit13
		WORD DI_GENERAL_PURPOSE_B : 1;			//Bit14
		WORD DI_GENERAL_PURPOSE_A : 1;			//Bit15
	};

	class EPOS2
	{
	public:
		EPOS2(unsigned short NodeId);
		~EPOS2();
		
		int OpenDevice();
		int CloseDevice();
		unsigned short GetNodeID() { return m_NodeId; }

		int ReadNodeVersion();
		int ReadNodeParams();
		int WriteNodeParams();

		bool IsOpen() { return m_KeyHandle != NULL; }

		int SetupNodeSafetyInfo(EPOS2SafetyParam & safety);
		EPOS2SafetyParam * GetSafetyInfo() { return &m_SafeParam; }

		int EnableNode();
		int DisableNode();
		int SetOperationMode(char OperationMode);
		char GetOperationMode();

		int ClearFault();
		//////////////////////////////////////////////////////////////////////////
		// State
		int GetState();
		int GetPosition();
		int GetVelocity();
		int GetMoveState();
		int GetTargetPosition();
		int GetTargetVelocity();
		int GetDigitalInputs(DigitalInputs & di);
		BOOL IsTargetReached() { return GetMoveState(); }

		//////////////////////////////////////////////////////
		// Infomation
		int ShowOperationMode();

		int SetVelocityProfileParam(VelocityProfileParam &param);
		VelocityProfileParam* GetVelocityProfileParam() { return &m_VelocityProfileParam; }
		int SetPositionProfileParam(PositionProfileParam &param);
		PositionProfileParam* GetPositionProfileParam() { return &m_PositionProfileParam; }

		int MoveV(long velocity);
		int MoveP(long position, BOOL is_absolute = TRUE);
		int Halt();
		int WaitForMoveEnd(int timeout_ms);
		
		//////////////////////////////////////////////////////
		// PVT Mode
		int SetIPMParam(IPMParam &param);
		IPMParam* GetIPMParam() { return &m_IPMParam; }
		int LoadPVTData(const char * filename);
		int StartIPMMode();
		int StartTrajectory();
		int StopTrajectory();
		int StopIPMMode();
		void ClearPVTQuque();

		// Homing
		int SetHomingParam(HomingParam& param);
		HomingParam* GetHomingParam() { return &m_HomeParam; }
		int SetCurrentPositionAsHome();
		int FindHomeByPosLimit();
		int FindHomeByNegLimit();
		int WaitForHomeEnd();
		int StopHoming();

		// Data record
		int SetRecorderParam(RecorderParam &param);
		int StartRecording();
		int StopRecording();

		// limit
		int EnablePosLimitSwitch(bool enable);
		int EnableNegLimitSwitch(bool enable);
		
		int QuickStop();
	protected:
		static int ShowErrorInformation(DWORD error_code);
		static void IPMThreadFunc(void * arg);
		static void RecorderThreadFunc(void * arg);
	public: // version info
		char m_libname[64] = { 0 };
		char m_libversion[64] = { 0 };

		unsigned short m_hardwareVersion = 0;
		unsigned short m_softwareVersion = 0;
		unsigned short m_applicationNumber = 0;
		unsigned short m_applicationVersion = 0;
	protected:
		char					m_Mode;
		DWORD					m_ErrorCode;
		void *					m_KeyHandle;
		unsigned short			m_NodeId;
		// CANopen parameters
		DWORD					m_BaudRate;
		DWORD					m_Timeout_ms;

		// Safety parameters
		EPOS2SafetyParam		m_SafeParam;
		// Profile parameters
		PositionProfileParam	m_PositionProfileParam;
		VelocityProfileParam	m_VelocityProfileParam;
		// Homing Mode parameters
		HomingParam				m_HomeParam;
		// IPM paramaters
		IPMParam				m_IPMParam;
		
		std::queue<IPMPVT *>	m_PVTQueue;
		std::mutex				m_PVTQueueLock;
		std::thread *			m_IPMThread;
		BOOL					m_IsIPMStop;

		// Recorder
		RecorderParam			m_RecorderParam;
		std::thread				m_RecorderThread;
		BOOL					m_IsRecorderStop;
	};
} //end of namespace
#endif
