#pragma once
#include <memory>
#include <vector>
#include "EPOS2.h"

using namespace Device;

// 丝杠螺距
#define RFPPCM_SCREW_PITCH_MM		2.0

// 四个驱动副约束范围
#define RFPPCM_JOINT_R_LOW_DEG		-0.25*360.0
#define RFPPCM_JOINT_R_HIGH_DEG		0.25*360.0

#define RFPPCM_JOINT_P_LOW_MM		-50.0*RFPPCM_SCREW_PITCH_MM
#define RFPPCM_JOINT_P_HIGH_MM		25.0*RFPPCM_SCREW_PITCH_MM

inline double MM2DEG(double len)
{
	return len * 360.0 / RFPPCM_SCREW_PITCH_MM;
}

inline double DEG2MM(double deg)
{
	return deg / 360.0 * RFPPCM_SCREW_PITCH_MM;
}

class RFPPCMController
{
public:
	RFPPCMController();

	bool ConnectEPOS2();
	void DisconnectEPOS2();
	bool IsConnected() { return m_connected; }
	void ClearFault();

	void StartHoming();
	void StopHoimg();
	
	void Enable();
	void Disable();

	void QuickStop();

	void MoveTo(double r1_deg, double r2_deg, double p3_mm, double p4_mm);
	void StopMove();
	bool IsMoveEnd();

	size_t GetEPOS2Num() { return m_links.size(); }
	std::shared_ptr<EPOS2> GetEPOS2(size_t index);

	int GetMotorQC(size_t index);
protected:
	std::vector<std::shared_ptr<EPOS2>> m_links;
	bool m_connected = false;
};

