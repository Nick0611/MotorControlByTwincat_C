#include "RFPPCMController.h"

RFPPCMController::RFPPCMController()
{
	m_links.push_back(std::make_shared<EPOS2>(1));
	m_links.push_back(std::make_shared<EPOS2>(2));
	m_links.push_back(std::make_shared<EPOS2>(3));
	m_links.push_back(std::make_shared<EPOS2>(4));
}

bool RFPPCMController::ConnectEPOS2()
{
	for (std::shared_ptr<EPOS2>& epos2: m_links)
	{
		if (!epos2->OpenDevice())
		{
			return false;
		}
	}
	m_connected = true;
	return true;
}

void RFPPCMController::DisconnectEPOS2()
{
	for (std::shared_ptr<EPOS2>& epos2 : m_links)
	{
		if (!epos2->CloseDevice())
		{
		}
	}
	m_connected = false;
}

void RFPPCMController::ClearFault()
{
	for (std::shared_ptr<EPOS2>& epos2 : m_links)
	{
		if (!epos2->ClearFault())
		{
		}
	}
}

void RFPPCMController::StartHoming()
{
	m_links[0]->FindHomeByNegLimit();
	m_links[1]->FindHomeByNegLimit();
	m_links[2]->FindHomeByPosLimit();
	m_links[3]->FindHomeByPosLimit();
	//for (std::shared_ptr<EPOS2>& epos2 : m_links)
	//{
		//if (!epos2->FindHomeByNegLimit())
		//{
			
		//}
	//}
}

void RFPPCMController::StopHoimg()
{
	for (std::shared_ptr<EPOS2>& epos2 : m_links)
	{
		if (!epos2->StopHoming())
		{
		}
	}
}

void RFPPCMController::Enable()
{
	for (std::shared_ptr<EPOS2>& epos2 : m_links)
	{
		epos2->EnableNode();
	}
}

void RFPPCMController::Disable()
{
	for (std::shared_ptr<EPOS2>& epos2 : m_links)
	{
		epos2->DisableNode();
	}
}

void RFPPCMController::QuickStop()
{
	for (std::shared_ptr<EPOS2>& epos2 : m_links)
	{
		epos2->QuickStop();
	}
}

void RFPPCMController::MoveTo(double r1_deg, double r2_deg, double p3_mm, double p4_mm)
{
	int q1 = -Device::Deg2QC(r1_deg);
	int q2 = -Device::Deg2QC(r2_deg);
	int q3 = -Device::Deg2QC(MM2DEG(p3_mm));
	int q4 = -Device::Deg2QC(MM2DEG(p4_mm));

	size_t n = m_links.size();
	if (n > 0)
	{
		m_links[0]->MoveP(q1);
	}
	if (n > 1)
	{
		m_links[1]->MoveP(q2);
	}
	if (n > 2)
	{
		m_links[2]->MoveP(q3);
	}
	if (n > 3)
	{
		m_links[3]->MoveP(q4);
	}
}

void RFPPCMController::StopMove()
{
	for (auto& epos2 : m_links)
	{
		epos2->Halt();
	}
}

bool RFPPCMController::IsMoveEnd()
{
	BOOL ret = TRUE;
	for (auto & epos2: m_links)
	{
		ret &= epos2->IsTargetReached();
	}
	return ret;
}

std::shared_ptr<Device::EPOS2> RFPPCMController::GetEPOS2(size_t index)
{
	if (index >= 0 && index < m_links.size())
	{
		return m_links[index];
	}
	return nullptr;
}

int RFPPCMController::GetMotorQC(size_t index)
{
	if (index >= 0 && index < m_links.size())
	{
		std::shared_ptr<EPOS2> epos2 = m_links[index];
		if (epos2->IsOpen())
		{
			return epos2->GetPosition();
		}
	}
	return 0;
}
