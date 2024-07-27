#include"ppcm.h"

Grad_Obj_return finger_contact(VectorXd q, VectorXd Ft, VectorXd fr, double fc, double lc, parameters par, Vector4d obj)
{
	Matrix3d I3, Z3;
	Matrix4d I4;
	Vector3d e1(1, 0, 0), e2(0, 1, 0), e3(0, 0, 1), e0(0, 0, 0);
	Vector3d rO0=obj.topRows(3);
	double rad = obj(3);
	I3.setIdentity();
	I4.setIdentity();
	Z3.setZero();

	int num = par.Nm;
	MatrixXd x = par.Tw;
	MatrixXd Kq = par.Kq;
	VectorXi NoB = par.NoB;
	VectorXi NoS = par.NoS;
	MatrixXd gb0 = par.gb0;
	MatrixXd gs0 = par.gs0;
	VectorXd lr0 = par.dr;
	double dis1 = par.dis1;

	int Nos1 = NoS(0), Nob1 = NoB(0);
	int Nos2 = NoS(1), Nob2 = NoB(1);
	int Nos3 = NoS(2), Nob3 = NoB(2);
	int Nos4 = NoS(3), Nob4 = NoB(3);
	int Nos5 = NoS(4), Nob5 = NoB(4);

	int NoC = floor(lc / dis1);
	if (NoC > num / 2)
		NoC = num / 3;

	Matrix4d gst = I4, gi, gsc;
	MatrixXd Jt(6, num), At(6, 6), Ai(6, 6), Jc(6, num);
	MatrixXd Js1(6, num), Js2(6, num), Js3(6, num), Js4(6, num), Js5(6, num);
	MatrixXd Jb1(6, num), Jb2(6, num), Jb3(6, num), Jb4(6, num), Jb5(6, num);
	MatrixXd gb(4, gb0.cols()), gs(4, gs0.cols());
	MatrixXd Jb(6, num * NoB.rows()), Js(6, num * NoS.rows());
	Jt.setZero();
	At.setIdentity();
	Jb.setZero();
	Js.setZero();
	double qi;	
	for (int ii = 0; ii < num; ++ii)
	{
		qi = q(ii);
		VectorXd xi(6);
		xi = x.col(ii);
		Jt.col(ii) = At * xi;
		gi = expm(wed(xi), qi);
		Ai = Ad(gi);
		gst = gst * gi;
		At = At * Ai;
		//gb
		if (ii == Nob1-1)
		{
			gb.leftCols(4) = gst * gb0.leftCols(4);
			Jb1 = Jt;
		}			
		if (ii == Nob2-1)
		{
			gb.middleCols(4, 4) = gst * gb0.middleCols(4, 4);
			Jb2 = Jt;
		}	
		if (ii == Nob3-1)
		{
			gb.middleCols(8, 4) = gst * gb0.middleCols(8, 4);
			Jb3 = Jt;
		}			
		if (ii == Nob4-1)
		{
			gb.middleCols(12, 4) = gst * gb0.middleCols(12, 4);
			Jb4 = Jt;
		}			
		if (ii == Nob5-1)
		{
			gb.middleCols(16, 4) = gst * gb0.middleCols(16, 4);
			Jb5 = Jt;
		}
		//gs
		if (ii == Nos1-1)
		{
			gs.leftCols(4) = gst * gs0.leftCols(4);
			Js1 = Jt;
		}
		if (ii == Nos2-1)
		{
			gs.middleCols(4, 4) = gst * gs0.middleCols(4, 4);
			Js2 = Jt;
		}
		if (ii == Nos3-1)
		{
			gs.middleCols(8, 4) = gst * gs0.middleCols(8, 4);
			Js3 = Jt;
		}
		if (ii == Nos4-1)
		{
			gs.middleCols(12, 4) = gst * gs0.middleCols(12, 4);
			Js4 = Jt;
		}
		if (ii == Nos5-1)
		{
			gs.middleCols(16, 4) = gst * gs0.middleCols(16, 4);
			Js5 = Jt;
		}
		//gsc
		if (ii == NoC-1)
		{
			gsc = gst;
			Jc = Jt;
		}
	}
	Jb << Jb1, Jb2, Jb3, Jb4, Jb5;
	Js << Js1, Js2, Js3, Js4, Js5;
	//contact frame
	Matrix4d gc0;
	Vector3d rC, xC, yC, rO, dc0;
	VectorXd Wc(6), Fc(6),dFc_l(60),dFc_f(60);
	MatrixXd drC(3, num), dxC(3, num), drO(3, num), dWc(6, num);
	MatrixXd KFc(num, num);
	gc0 << I3, lc* e2,
		0, 0, 0, 1;
	gsc = gsc * gc0;
	rC << gsc.topRightCorner(3, 1);
	xC << gsc.topLeftCorner(3, 1);
	yC << gsc.block(0, 1, 3, 1);
	rO = rC - rad * xC;
	Wc << rC.cross(xC), xC;
	Fc << fc * Wc;
	drC << (MatrixXd(3, 6) << -wed(rC), I3).finished() * Jc;
	dxC << (MatrixXd(3, 6) << -wed(xC), Z3).finished() * Jc;
	dWc << wed(rC)*dxC - wed(xC)*drC, dxC;
	drO = drC - rad * dxC;
	dc0 = yC;

	dFc_l << fc * Jc.transpose() * (VectorXd(6)<<-xC.cross(yC),e0).finished();
	dFc_f << Jc.transpose() * Wc;
	KFc << fc * Jc.transpose() * dWc;
	//
	int MM = NoS.rows();
	MatrixXd dl_q(MM, num), Kr(num, num);
	VectorXd lr(MM);
	MatrixXd dt_df(num, 5);
	dl_q.setZero();
	lr.setZero();
	dt_df.setZero();	
	Kr.setZero();
	MatrixXd Jsk(6, num), Jbk(6, num);
	Vector3d rSk,rBk,urk;
	VectorXd Wrk(6);
	MatrixXd dWrk(6, num), drSk(3, num), drBk(3, num), durk(3, num), dFrk(6, num);
	double frk, lrk;
	VectorXd dlrk(60),Fr(60);
	Fr.setZero();
	for (int kk = 0; kk < MM; ++kk)
	{
		rSk = gs.block(0, 4 * kk + 3, 3, 1);
		Jsk = Js.middleCols(kk * num, num);
		rBk = gb.block(0, 4 * kk + 3, 3, 1);
		Jbk = Jb.middleCols(kk * num, num);
		frk = fr(kk);
		lrk = (rSk - rBk).norm();
		urk = (rSk - rBk) / lrk;
		Wrk << rSk.cross(urk), urk;
		drSk = (MatrixXd(3, 6) << -wed(rSk), I3).finished() * Jsk;
		drBk = (MatrixXd(3, 6) << -wed(rBk), I3).finished() * Jbk;
		dlrk = urk.transpose() * (drSk - drBk);
		durk = (I3 - urk * (urk.transpose())) * (drSk - drBk) / lrk;
		dWrk << wed(rSk) * durk - wed(urk) * drSk,
			    durk;
		dFrk = frk * (Jsk - Jbk).transpose() * dWrk;
		Fr = Fr + frk * (Jsk - Jbk).transpose() * Wrk;
		MatrixXd KJsk(num, num), KJbk(num, num);
		KJsk.setZero();
		KJbk.setZero();
		//#pragma omp parallel
		{
		//#pragma omp for schedule(static,int(num/8))
				for (int i = 0; i < num - 1; ++i)
				{
					for (int k = i + 1; k < num; ++k)
					{
						//KJsk(k, i) = frk * Wrk.transpose() * ad(Jsk.col(i)) * Jsk.col(k);
						KJsk(k, i) = frk * (Wrk(3) * (Jsk(2, k) * Jsk(4, i) - Jsk(4, k) * Jsk(2, i)) + Wrk(4) * (-Jsk(2, k) * Jsk(3, i) + Jsk(3, k) * Jsk(2, i)));
						//KJbk(k, i) = -frk * Wrk.transpose() * ad(Jbk.col(i)) * Jbk.col(k);
						KJbk(k, i) = -frk * (Wrk(3) * (Jbk(2, k) * Jbk(4, i) - Jbk(4, k) * Jbk(2, i)) + Wrk(4) * (-Jbk(2, k) * Jbk(3, i) + Jbk(3, k) * Jbk(2, i)));
					}
				}
		}

		lr(kk) = lrk;
		dl_q.row(kk) = dlrk;
		dt_df.col(kk) = (Jsk - Jbk).transpose() * Wrk;
		Kr = Kr + dFrk + KJsk + KJbk;
	}
	MatrixXd KJt(num, num), KJc(num, num);
	MatrixXd Kc(num, num), Kt(num, num);
	KJt.setZero();
	KJc.setZero();
//#pragma omp parallel
	{
//#pragma omp for schedule(static,int(num/8))
		for (int i = 0; i < num - 1; ++i)
		{
			for (int k = i + 1; k < num; ++k)
			{
				//KJt(k, i) = Ft.transpose() * ad(Jt.col(i)) * Jt.col(k);
				KJt(k, i) = Ft(3) * (Jt(2, k) * Jt(4, i) - Jt(4, k) * Jt(2, i)) + Ft(4) * (-Jt(2, k) * Jt(3, i) + Jt(3, k) * Jt(2, i));
				//KJc(k, i) = Fc.transpose() * ad(Jc.col(i)) * Jc.col(k);
				KJc(k, i) = Fc(3) * (Jc(2, k) * Jc(4, i) - Jc(4, k) * Jc(2, i)) + Fc(4) * (-Jc(2, k) * Jc(3, i) + Jc(3, k) * Jc(2, i));
			}
		}
	}
	Kc = KJc + KFc;
	Kt = KJt;
	//
	VectorXd dg(6),dl(5),dr(3),dt(num),dY(num+14);
	MatrixXd dt_dq(num, num),Jac(num+14,num+13);
	dg = Vee(logm(gst));
	dl = lr - lr0;
	dr = rO - rO0;
	dt = Kq * q - Fr - Jt.transpose()*Ft - Jc.transpose() * Fc;
	dY << dg, dl, dr, dt;

	dt_dq = Kq - Kt - Kc - Kr;
	Jac.setZero();

	Jac.block(0, 0, 6, num) = Jt;
	Jac.block(6, 0, 5, num) = dl_q;
	Jac.block(11, 0, 3, num) = drO;
	Jac.block(11, num + 11, 3, 1) = dc0;
	Jac.block(14, 0, num, num) = dt_dq;
	Jac.block(14, num, num, 6) = -Jt.transpose();
	Jac.block(14, num+6, num, 5) = -dt_df;
	Jac.block(14, num + 11, num, 1) = -dFc_l;
	Jac.block(14, num + 12, num, 1) = -dFc_f;	
	Grad_Obj_return gor;
	gor.c = dY;
	gor.gradient = Jac;
	return gor;
}
