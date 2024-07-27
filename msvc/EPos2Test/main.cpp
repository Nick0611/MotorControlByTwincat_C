#define _USE_MATH_DEFINES 
#include <iostream>
#include <stdio.h>
#include <math.h>
//#include "tools.h"
#include "EPOS2.h"

using namespace Device;

double Pos2Rad(int pos)
{
	return pos * 2 * M_PI / MAXON_EPOS2_QC_PER_CIRCLE;
}

int main()
{
	RecorderParam param = { 0 };
	param.ChannelSize = 1;
	param.Channels[0].ChannelNumber = 0;
	param.Channels[0].ObjectIndex = 0x2;
	param.Channels[0].ObjectSubIndex = 0x2;
	param.Channels[0].ObjectSize = 4;

	int node_id = 2;
	
	Device::EPOS2 epos2(node_id);

	epos2.SetRecorderParam(param);

	int bRet = epos2.OpenDevice();
	if (!bRet)
	{
		return EXIT_FAILURE;
	}
	EPOS2SafetyParam safety;
	safety.MaxAcceleration = 1000;
	safety.MaxFollowingError = 100000;
	safety.MaxProfileVelocity = MAXON_EPOS2_MAX_VELOCITY;
	epos2.SetupNodeSafetyInfo(safety);

	// move to home
	PositionProfileParam pos_param;
	pos_param.ProfileVelocity = 1000;
	pos_param.ProfileAcceleration = 1000;
	pos_param.ProfileDeceleration = 1000;
	epos2.SetPositionProfileParam(pos_param);
	
	epos2.MoveP(0);

	VelocityProfileParam velocity_param;
	velocity_param.ProfileAcceleration = 1000;
	velocity_param.ProfileDeceleration = 1000;

	IPMParam ipm_param;
	ipm_param.UnderFlowWarningLimit = 4;
	ipm_param.OverFlowWarningLimit = 60;
	epos2.SetIPMParam(ipm_param);

	bool stop = false;
	while(!stop)
	{
		std::cout << "Please enter cmd:";
		char cmd = 0;
		std::cin >> cmd;
		switch (cmd)
		{
		case 's':
		{
			int count = epos2.LoadPVTData("motor_3.txt");
			epos2.StartIPMMode();
			break;
		}
		case 't':
		{
			epos2.StopIPMMode();
			break;
		}
		case '1':
		{
			epos2.StartTrajectory();
			break;
		}
		case '2':
		{
			epos2.StopTrajectory();
			break;
		}
		case 'm':
		{
			epos2.SetPositionProfileParam(pos_param);
			epos2.MoveP(0);
			break;
		}
		case 'v':
		{
			epos2.SetVelocityProfileParam(velocity_param);
			epos2.MoveV(100);
			break;
		}
		case 'h':
		{
			epos2.Halt();
			break;
		}
		case 'q':
		{
			stop = true;
			epos2.QuickStop();
			epos2.DisableNode();
			break;
		}
		default:
			printf("invalid option %c\n", cmd);
			break;
		}
	}
	printf("Press enter to exit: ");
	getchar();
	return 0;
}
