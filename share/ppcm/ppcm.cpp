#include "ppcm.h"

//函数Ad()的定义
MatrixXd Ad(MatrixXd mat)
{
	MatrixXd H(6, 6);
	Matrix3d p_hat, R;
	Vector3d p;
	R = mat.block(0, 0, 3, 3);
	H.setZero(6, 6);
	H.block(0, 0, 3, 3) = R;
	H.block(3, 3, 3, 3) = R;
	p << mat.block(0, 3, 3, 1);
	p_hat << 0, -p(2), p(1),
		p(2), 0, -p(0),
		-p(1), p(0), 0;
	H.block(3, 0, 3, 3) = p_hat * R;
	return H;
}
//函数ad()的定义
MatrixXd ad(VectorXd ksi)
{
	MatrixXd H(6, 6);
	Matrix3d w_hat, v_hat;
	w_hat << 0.0, -ksi(2, 0), ksi(1, 0),
		ksi(2, 0), 0.0, -ksi(0, 0),
		-ksi(1, 0), ksi(0, 0), 0.0;
	v_hat << 0.0, -ksi(5, 0), ksi(4, 0),
		ksi(5, 0), 0.0, -ksi(3, 0),
		-ksi(4, 0), ksi(3, 0), 0.0;
	H << w_hat, MatrixXd::Zero(3, 3),
		v_hat, w_hat;
	return H;
}

//矩阵指数函数定义expm()
Matrix4d expm(MatrixXd k, double theta)
{
	Matrix4d Expo_Mat;
	Expo_Mat = MatrixXd::Identity(4, 4) + k * theta + (1.0 - cos(theta)) * (k * k) + (theta - sin(theta)) * (k * k * k);
	return Expo_Mat;
}

//矩阵的幂函数定义expn()
MatrixXd expn(MatrixXd mat, int n)
{
	MatrixXd m;
	m.setIdentity(mat.rows(), mat.cols());
	for (int i = 1; i <= n; ++i)
	{
		m *= mat;
	}
	return m;
}

//矩阵对数函数定义logm()
Matrix4d logm(MatrixXd x)
{
	double theta;
	double nm;
	//double tr;
	Matrix4d Log_Mat;
	//theta = acos(x.trace() *0.5 - 1.0);
	nm = x.trace() * 0.5 - 1.0;
	if ((nm - 1) > 0 && (nm - 1) < 1e-7)
		nm = 1;
	theta = acos(nm);
	//tr = x.trace();
	//std::cout << tr << "   theta:" << theta << "  cos(theta)=" << nm<<std::endl;
	if (fabs(theta) < 1e-7)
	{
		Log_Mat = 0.125 * (2 * x * x * x - 9 * x * x + 18 * x - 11 * MatrixXd::Identity(4, 4)) / 0.75;
	}
	else
	{
		Log_Mat = (0.125 / (pow(sin(theta * 0.5), 3) * cos(theta * 0.5))) * ((theta * cos(2 * theta) - sin(theta)) * (MatrixXd::Identity(4, 4)) -
			(theta * cos(theta) + 2 * theta * cos(2 * theta) - sin(theta) - sin(2 * theta)) * x +
			(2 * theta * cos(theta) + theta * cos(2 * theta) - sin(theta) - sin(2 * theta)) * x * x -
			(theta * cos(theta) - sin(theta)) * x * x * x);
	}
	return Log_Mat;
}

//lambda函数定义
Matrix3d lambda(double theta)
{
	double Cos_t, Sin_t, a_1i[5];
	Vector3d e0(0.0, 0.0, 0.0), e1(1.0, 0.0, 0.0), e2(0.0, 1.0, 0.0);
	Matrix3d lam, ad;
	lam.setZero(3, 3);
	ad << e2, -e1, e0;
	Cos_t = cos(theta);
	Sin_t = sin(theta);
	a_1i[0] = theta;
	a_1i[1] = 0.5 * (4 - theta * Sin_t - 4 * Cos_t);
	a_1i[2] = 0.5 * (4 * theta - 5 * Sin_t + theta * Cos_t);
	a_1i[3] = 0.5 * (2 - theta * Sin_t - 2 * Cos_t);
	a_1i[4] = 0.5 * (2 * theta - 3 * Sin_t + theta * Cos_t);
	for (int z = 1; z <= 5; ++z)
	{
		lam += a_1i[z - 1] * expn(ad, z - 1);
	}
	return lam;
}

//矩阵广义逆函数定义pinv()
MatrixXd pinv(MatrixXd A)
{
	//进行SVD分解
	JacobiSVD<MatrixXd> svd_A(A, ComputeThinU | ComputeThinV);
	//构建SVD分解结果
	double tolerance = 1.e-8;
	MatrixXd U = svd_A.matrixU();
	MatrixXd V = svd_A.matrixV();
	MatrixXd D = svd_A.singularValues();
	//构建D矩阵广义逆
	MatrixXd S(V.cols(), U.cols());
	S.setZero();
//#pragma omp parallel for default(shared)
	for (int i = 0; i < D.size(); ++i)
	{
		if (D(i, 0) > tolerance)
			S(i, i) = 1.0 / D(i, 0);
		else
			S(i, i) = 0;
	}
	return V * S * U.transpose();
}

//Vee算子函数定义
VectorXd Vee(MatrixXd mat)
{
	VectorXd K(6);
	K << -mat(1, 2), mat(0, 2), -mat(0, 1), mat.block(0, 3, 3, 1);
	return K;
}

//Wedge算子函数定义
MatrixXd wed(VectorXd x)
{
	MatrixXd hat;
	if (x.rows() == 3)
	{
		Matrix3d h;
		h << 0, -x(2), x(1),
			x(2), 0, -x(0),
			-x(1), x(0), 0;
		hat = h;
	}
	else
	{
		Matrix4d h;
		h << 0, -x(2), x(1), x(3),
			x(2), 0, -x(0), x(4),
			-x(1), x(0), 0, x(5),
			0, 0, 0, 0;
		hat = h;
	}
	return hat;
}

//void sns_statusinit(statusrecord* st)
//{
//	double radius = 15e-3;
//	Vector4d obj;
//	Vector3d center;
//	center << -25e-3, 50e-3, 0;
//	obj << center, radius; // 圆形物体中心位置和半径大小
//
//	Matrix3d I3, Z3;
//	Matrix4d I4;
//	Vector3d e1(1, 0, 0), e2(0, 1, 0), e3(0, 0, 1), r0(0, 0, 0);
//	double ds1 = 17e-3, ds2 = 22e-3, ds3 = 22e-3, ds4 = 22e-3, ds5 = 22e-3, ds6 = 35e-3;
//	double lS1, lS2, lS3, lS4, lS5, lS6, ls0;
//	double lB1, lB2, lB3, lB4, lB5;
//	double db1 = 27.16e-3, db2 = 24.20e-3, db3 = 23.28e-3, db4 = 22.39e-3, db5 = 21.54e-3;
//	Vector3d  rS00, rB00;
//	double db0 = 56.56e-3, lb0;
//	Vector3d ub0, vb0;
//	Vector3d rS01, rS02, rS03, rS04, rS05, rS06;
//	Vector3d rB01, rB02, rB03, rB04, rB05;
//	double dr1, dr2, dr3, dr4, dr5;
//	VectorXd dr(5);
//	Matrix3d Rb0;
//	Matrix4d gt0;
//	Matrix4d gs10, gs20, gs30, gs40, gs50;
//	Matrix4d gb10, gb20, gb30, gb40, gb50;
//	MatrixXd gs0(4, 5 * 4), gb0(4, 5 * 4);
//	double dis1, dis2, ki0 = 2e-3, ki1, ki2;
//	int num1 = 30, num2 = 30, num;
//	I3.setIdentity();
//	I4.setIdentity();
//	Z3.setZero();
//
//	// geometric parameters of the contact side: without deformation
//	rS00 << r0 + 0 * e1; // position of the base
//	rS01 = rS00 + ds1 * e2;
//	rS02 = rS01 + ds2 * e2;
//	rS03 = rS02 + ds3 * e2;
//	rS04 = rS03 + ds4 * e2;
//	rS05 = rS04 + ds5 * e2;
//	rS06 = rS05 + ds6 * e2;
//
//	lS1 = ds1;
//	lS2 = lS1 + ds2;
//	lS3 = lS2 + ds3;
//	lS4 = lS3 + ds4;
//	lS5 = lS4 + ds5;
//	lS6 = lS5 + ds6;
//	// geometric parameters of the other side
//	rB00 = rS00 + db0 * e1;
//	lb0 = (rS06 - rB00).norm();
//	ub0 = (rS06 - rB00) / lb0;
//	vb0 = wed(e3) * ub0;
//
//	rB01 = rB00 + db1 * ub0;
//	rB02 = rB01 + db2 * ub0;
//	rB03 = rB02 + db3 * ub0;
//	rB04 = rB03 + db4 * ub0;
//	rB05 = rB04 + db5 * ub0;
//
//	lB1 = lb0 - db1;
//	lB2 = lB1 - db2;
//	lB3 = lB2 - db3;
//	lB4 = lB3 - db4;
//	lB5 = lB4 - db5;
//	//length of the ribs
//	dr1 = (rS01 - rB01).norm();
//	dr2 = (rS02 - rB02).norm();
//	dr3 = (rS03 - rB03).norm();
//	dr4 = (rS04 - rB04).norm();
//	dr5 = (rS05 - rB05).norm();
//	dr << dr1 + 0.5e-3, dr2 + 0.5e-3, dr3 + 0.5e-3, dr4 + 0.5e-3, dr5 + 0.5e-3;
//
//	// construction of the local frame
//	gs10 << I3, rS01,
//		0, 0, 0, 1;
//	gs20 << I3, rS02,
//		0, 0, 0, 1;
//	gs30 << I3, rS03,
//		0, 0, 0, 1;
//	gs40 << I3, rS04,
//		0, 0, 0, 1;
//	gs50 << I3, rS05,
//		0, 0, 0, 1;
//	gs0 << gs10, gs20, gs30, gs40, gs50;
//
//	Rb0 << -vb0, ub0, e3;
//
//	gb10 << Rb0, rB01,
//		0, 0, 0, 1;
//	gb20 << Rb0, rB02,
//		0, 0, 0, 1;
//	gb30 << Rb0, rB03,
//		0, 0, 0, 1;
//	gb40 << Rb0, rB04,
//		0, 0, 0, 1;
//	gb50 << Rb0, rB05,
//		0, 0, 0, 1;
//	gb0 << gb10, gb20, gb30, gb40, gb50;
//	gt0 << I3, rB00,
//		0, 0, 0, 1;
//	// 
//	ls0 = lS6-12e-3;
//	dis1 = ls0 / num1;
//	ki1 = dis1 * ki0;
//	dis2 = (lb0-12.5e-3) / num2;
//	ki2 = dis2 * ki0;
//	num = num1 + num2;
//	// discretization of the flexible link
//	MatrixXd Tw(6, num), Kq(num, num);
//	Vector3d ri;
//	VectorXd xi(6);
//	Tw.setZero();
//	Kq.setZero();
//	Kq.topLeftCorner(num1, num1) = ki1 * MatrixXd::Identity(num1, num1);
//	Kq.bottomRightCorner(num2, num2) = ki2 * MatrixXd::Identity(num2, num2);
//	for (int i = 0; i < num; ++i)
//	{
//		if (i < num1)
//			ri = rS00 + (double(i) + 0.5) * dis1 * e2;
//		else
//			ri = rS06 - (double(i - num1) + 0.5) * dis2 * ub0 - ub0 * 12.5e-3;
//		xi << e3, wed(ri)* e3;
//		Tw.col(i) = xi;
//	}
//	double dsl = 20e-3;
//	//double ls11 = 33.28e-3, ls21 = 55.28e-3, ls31 = 78.28e-3, ls41 = 101.28e-3;
//	double ls11 = 24.5e-3, ls21 =47e-3, ls31 = 70e-3, ls41 = 93e-3;
//	int Ns11, Ns12, Ns21, Ns22, Ns31, Ns32, Ns41, Ns42;
//	int NoS1, NoS2, NoS3, NoS4, NoS5;
//	int NoB1, NoB2, NoB3, NoB4, NoB5;
//	VectorXi NoS(5), NoB(5);
//	MatrixXd Seq(4, num);
//	Seq.setZero();
//	Ns11 = num1 + floor(ls11 / dis2);
//	Ns12 = Ns11 + floor(dsl / dis2);
//	Ns21 = num1 + floor(ls21 / dis2);
//	Ns22 = Ns21 + floor(dsl / dis2);
//	Ns31 = num1 + floor(ls31 / dis2);
//	Ns32 = Ns31 + floor(dsl / dis2);
//	Ns41 = num1 + floor(ls41 / dis2);
//	Ns42 = Ns41 + floor(dsl / dis2);
//
//	Seq.block(0, Ns11 - 1, 1, Ns12 - Ns11 + 1) << RowVectorXd::Ones(Ns12 - Ns11 + 1);
//	Seq.block(1, Ns21 - 1, 1, Ns22 - Ns21 + 1) << RowVectorXd::Ones(Ns22 - Ns21 + 1);
//	Seq.block(2, Ns31 - 1, 1, Ns32 - Ns31 + 1) << RowVectorXd::Ones(Ns32 - Ns31 + 1);
//	Seq.block(3, Ns41 - 1, 1, Ns42 - Ns41 + 1) << RowVectorXd::Ones(Ns42 - Ns41 + 1);
//
//	NoS1 = floor(lS1 / dis1);
//	NoS2 = floor(lS2 / dis1);
//	NoS3 = floor(lS3 / dis1);
//	NoS4 = floor(lS4 / dis1);
//	NoS5 = floor(lS5 / dis1);
//
//	NoB1 = num1 + floor(lB1 / dis2);
//	NoB2 = num1 + floor(lB2 / dis2);
//	NoB3 = num1 + floor(lB3 / dis2);
//	NoB4 = num1 + floor(lB4 / dis2);
//	NoB5 = num1 + floor(lB5 / dis2);
//
//	NoS << NoS1, NoS2, NoS3, NoS4, NoS5;
//	NoB << NoB1, NoB2, NoB3, NoB4, NoB5;
//
//	st->para.dis1 = dis1;
//	st->para.dr = dr;
//	st->para.gb0 = gb0;
//	st->para.gs0 = gs0;
//	st->para.gt0 = gt0;
//	st->para.Kq = Kq;
//	st->para.Nm = num;
//	st->para.NoB = NoB;
//	st->para.NoS = NoS;
//	st->para.rS00 = rS00;
//	st->para.Tw = Tw;
//	st->para.Seq = Seq;
//
//	VectorXd q(num), Ft(6), fr(5), dX;
//	double fc = 0, lc = 70e-3;
//	q.setZero();
//	Ft.setZero();
//	fr.setZero();
//
//	st->fc = fc;
//	st->fr = fr;
//	st->Ft = Ft;
//	st->q = q;
//	st->lc = lc;
//	cout << "初始化成功!" << endl;
//}
void sns_statusinit(statusrecord* st)
{
	double radius = 15e-3;
	Vector4d obj;
	Vector3d center;
	center << -25e-3, 50e-3, 0;
	obj << center, radius; // 圆形物体中心位置和半径大小

	Matrix3d I3, Z3;
	Matrix4d I4;
	Vector3d e1(1, 0, 0), e2(0, 1, 0), e3(0, 0, 1), r0(0, 0, 0);
	double ds1 = 16.9e-3, ds2 = 22.15e-3, ds3 = 22.15e-3, ds4 = 21.9e-3, ds5 = 22.55e-3, ds6 = 35.2e-3;
	double lS1, lS2, lS3, lS4, lS5, lS6, ls0;
	double lB1, lB2, lB3, lB4, lB5;
	double db1 = 27.7e-3, db2 = 24.2e-3, db3 = 24e-3, db4 = 22.24e-3, db5 = 21.80e-3;
	Vector3d  rS00, rB00;
	double db0 = 56.56e-3, lb0;
	Vector3d ub0, vb0;
	Vector3d rS01, rS02, rS03, rS04, rS05, rS06;
	Vector3d rB01, rB02, rB03, rB04, rB05;
	double dr1, dr2, dr3, dr4, dr5;
	VectorXd dr(5);
	Matrix3d Rb0;
	Matrix4d gt0;
	Matrix4d gs10, gs20, gs30, gs40, gs50;
	Matrix4d gb10, gb20, gb30, gb40, gb50;
	MatrixXd gs0(4, 5 * 4), gb0(4, 5 * 4);
	double dis1, dis2, ki0 = 2e-8, ki1, ki2;
	int num1 = 30*2, num2 = 30*2, num;
	I3.setIdentity();
	I4.setIdentity();
	Z3.setZero();

	// geometric parameters of the contact side: without deformation
	rS00 << r0 + 0 * e1; // position of the base
	rS01 = rS00 + ds1 * e2;
	rS02 = rS01 + ds2 * e2;
	rS03 = rS02 + ds3 * e2;
	rS04 = rS03 + ds4 * e2;
	rS05 = rS04 + ds5 * e2;
	rS06 = rS05 + ds6 * e2;

	lS1 = ds1;
	lS2 = lS1 + ds2;
	lS3 = lS2 + ds3;
	lS4 = lS3 + ds4;
	lS5 = lS4 + ds5;
	lS6 = lS5 + ds6;
	// geometric parameters of the other side
	rB00 = rS00 + db0 * e1;
	lb0 = (rS06 - rB00).norm();
	ub0 = (rS06 - rB00) / lb0;
	vb0 = wed(e3) * ub0;

	rB01 = rB00 + db1 * ub0;
	rB02 = rB01 + db2 * ub0;
	rB03 = rB02 + db3 * ub0;
	rB04 = rB03 + db4 * ub0;
	rB05 = rB04 + db5 * ub0;

	lB1 = lb0 - db1 - 12.5e-3;
	lB2 = lB1 - db2;
	lB3 = lB2 - db3;
	lB4 = lB3 - db4;
	lB5 = lB4 - db5;
	//length of the ribs
	dr1 = (rS01 - rB01).norm();
	dr2 = (rS02 - rB02).norm();
	dr3 = (rS03 - rB03).norm();
	dr4 = (rS04 - rB04).norm();
	dr5 = (rS05 - rB05).norm();
	dr << dr1, dr2, dr3, dr4, dr5;

	// construction of the local frame
	gs10 << I3, rS01,
		0, 0, 0, 1;
	gs20 << I3, rS02,
		0, 0, 0, 1;
	gs30 << I3, rS03,
		0, 0, 0, 1;
	gs40 << I3, rS04,
		0, 0, 0, 1;
	gs50 << I3, rS05,
		0, 0, 0, 1;
	gs0 << gs10, gs20, gs30, gs40, gs50;

	Rb0 << -vb0, ub0, e3;

	gb10 << Rb0, rB01,
		0, 0, 0, 1;
	gb20 << Rb0, rB02,
		0, 0, 0, 1;
	gb30 << Rb0, rB03,
		0, 0, 0, 1;
	gb40 << Rb0, rB04,
		0, 0, 0, 1;
	gb50 << Rb0, rB05,
		0, 0, 0, 1;
	gb0 << gb10, gb20, gb30, gb40, gb50;
	gt0 << I3, rB00,
		0, 0, 0, 1;
	// 
	ls0 = lS6 - 12e-3;
	dis1 = ls0 / num1;
	dis2 = (lb0 - 12.5e-3) / num2;

	//double thi = 0.4e-3, wid = 20e-3;
	//double kI = 200 * 0.23e9; //Young's modulus
	//double Iu = wid * (thi * thi * thi) / 12, EI = 1 * kI * Iu;// bending stiffness : cross section
	//ki1 = EI / dis1; 
	//ki2 = EI / dis2;

	ki1 = ki0 / dis1;
	ki2 = ki0 / dis2;

	num = num1 + num2;
	// discretization of the flexible link
	MatrixXd Tw(6, num), Kq(num, num);
	Vector3d ri;
	VectorXd xi(6);
	Tw.setZero();
	Kq.setZero();
	Kq.topLeftCorner(num1, num1) = ki1 * MatrixXd::Identity(num1, num1);
	Kq.bottomRightCorner(num2, num2) = ki2 * MatrixXd::Identity(num2, num2);
	for (int i = 0; i < num; ++i)
	{
		if (i < num1)
			ri = rS00 + (double(i) + 0.5) * dis1 * e2;
		else
			ri = rS06 - (double(i - num1) + 0.5) * dis2 * ub0 - ub0 * 12.5e-3;
		xi << e3, wed(ri)* e3;
		Tw.col(i) = xi;
	}
	double dsl = 6e-3;
	//double ls11 = 33.28e-3, ls21 = 55.28e-3, ls31 = 78.28e-3, ls41 = 101.28e-3;
	double ls11 = 20.48e-3 + 7e-3;
	double ls21 = (20.48 + 22.54 + 7) * 1e-3;
	double ls31 = (20.48 + 22.54 + 23 + 7) * 1e-3;
	double ls41 = (20.48 + 22.54 + 23 + 23 + 7) * 1e-3;
	int Ns11, Ns12, Ns21, Ns22, Ns31, Ns32, Ns41, Ns42;
	int NoS1, NoS2, NoS3, NoS4, NoS5;
	int NoB1, NoB2, NoB3, NoB4, NoB5;
	VectorXi NoS(5), NoB(5);
	MatrixXd Seq(4, num);
	Seq.setZero();
	Ns11 = num1 + floor(ls11 / dis2);
	Ns12 = Ns11 + floor(dsl / dis2);
	Ns21 = num1 + floor(ls21 / dis2);
	Ns22 = Ns21 + floor(dsl / dis2);
	Ns31 = num1 + floor(ls31 / dis2);
	Ns32 = Ns31 + floor(dsl / dis2);
	Ns41 = num1 + floor(ls41 / dis2);
	Ns42 = Ns41 + floor(dsl / dis2);

	Seq.block(0, Ns11 - 1, 1, Ns12 - Ns11 + 1) << RowVectorXd::Ones(Ns12 - Ns11 + 1);
	Seq.block(1, Ns21 - 1, 1, Ns22 - Ns21 + 1) << RowVectorXd::Ones(Ns22 - Ns21 + 1);
	Seq.block(2, Ns31 - 1, 1, Ns32 - Ns31 + 1) << RowVectorXd::Ones(Ns32 - Ns31 + 1);
	Seq.block(3, Ns41 - 1, 1, Ns42 - Ns41 + 1) << RowVectorXd::Ones(Ns42 - Ns41 + 1);

	NoS1 = floor(lS1 / dis1);
	NoS2 = floor(lS2 / dis1);
	NoS3 = floor(lS3 / dis1);
	NoS4 = floor(lS4 / dis1);
	NoS5 = floor(lS5 / dis1);

	NoB1 = num1 + floor(lB1 / dis2);
	NoB2 = num1 + floor(lB2 / dis2);
	NoB3 = num1 + floor(lB3 / dis2);
	NoB4 = num1 + floor(lB4 / dis2);
	NoB5 = num1 + floor(lB5 / dis2);

	NoS << NoS1, NoS2, NoS3, NoS4, NoS5;
	NoB << NoB1, NoB2, NoB3, NoB4, NoB5;

	st->para.dis1 = dis1;
	st->para.dr = dr;
	st->para.gb0 = gb0;
	st->para.gs0 = gs0;
	st->para.gt0 = gt0;
	st->para.Kq = Kq;
	st->para.Nm = num;
	st->para.NoB = NoB;
	st->para.NoS = NoS;
	st->para.rS00 = rS00;
	st->para.Tw = Tw;
	st->para.Seq = Seq;

	VectorXd q(num), Ft(6), fr(5), dX;
	double fc = 0, lc = 70e-3;
	q.setZero();
	Ft.setZero();
	fr.setZero();

	st->fc = fc;
	st->fr = fr;
	st->Ft = Ft;
	st->q = q;
	st->lc = lc;
	cout << "初始化成功!" << endl;
}