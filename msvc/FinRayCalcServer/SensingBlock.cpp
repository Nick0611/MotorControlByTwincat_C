#include "ppcm.h"
sensingresult SensingBlock(statusrecord* st, double Vol[4])
{
	// contact circular object
	int startime = clock();
	int endtime1, endtime2;

	// contact calculation
	VectorXd q, Ft(6), fr(5), dX;
	double lc, fc;
	Vector4d sensor;
	int N=1;
	Grad_Obj_return GOR2;
	sensingresult result;
	VectorXd motor(6);
	Matrix4d conctact_frame;
	Vector3d contact_position;
	Vector3d contact_direction;

	motor.setZero();
	result.error_code = 1;

	double num = st->para.Nm;
	q = st->q;
	Ft = st->Ft;
	fr = st->fr;
	lc = st->lc;
	fc = st->fc;

	sensor << Vol[0], Vol[1], Vol[2], Vol[3];	

	endtime1 = clock();
	printf("\n----------------- ��֪����������� -----------------\n");
	while (1)
	{
		GOR2 = finger_sensing(q,Ft,fr,fc,lc, st->para, st->para.Seq, sensor);
		double norm = GOR2.c.norm();
		printf("��֪�������������%d\t\t��ǰģ:%f\n", N++, norm);
		if (N > 50)
		{
			result.error_code = 1;
			printf("\n�����������\n");
			break;
		}
		if (norm < 1e-6)
		{
			result.error_code = 0;
			break;			
		}			
		dX = pinv(GOR2.gradient) * GOR2.c;
		q -= dX.topRows(num);
		Ft -= dX.middleRows(num, 6);
		fr -= dX.middleRows(num + 6, 5);
		lc -= dX[ num + 11 ];
		fc -= dX[num + 12];
	}
	if (result.error_code == 0)
	{
		printf("------------------- ������ɣ�----------------------\n\n");
		// ���������ǰ״̬
		st->q = q;
		st->Ft = Ft;
		st->fr = fr;
		st->lc = lc;
		st->fc = fc;
	}
	else
	{
		printf("------------------- ����ʧ�ܣ�----------------------\n\n");
	}
	
	endtime2 = clock();
	cout << "           ��֪����ʱ�䣺" << (double(endtime2) - double(endtime1)) / CLOCKS_PER_SEC << 's' << endl << endl;
	conctact_frame = GOR2.gsc;
	contact_position= conctact_frame.topRightCorner(3, 1);
	contact_direction = conctact_frame.topLeftCorner(3, 1);
	
	result.Jac = GOR2.J;
	result.q = q;
	result.motor = motor;
	result.rib = GOR2.rib;
	result.rxi = GOR2.rxi;
	result.fc = fc;
	result.rC = contact_position;
	result.xC = contact_direction;

	return result;
}