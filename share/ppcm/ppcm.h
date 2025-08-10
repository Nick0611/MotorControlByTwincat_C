#pragma once
#define EIGEN_USE_MKL_ALL
#define EIGEN_VECTORIZE_SSE4_2
#include <Eigen\Dense>
#include <Eigen/SVD>
#include<math.h>
#include<stdio.h>
#include<omp.h>
#include<iostream>
#include "CPSAPI/CPSAPI.h"
#include "CPSAPI/CPSDef.h"
using namespace Eigen;
using namespace std;
# define pi 3.1415926535
// 计算过程所需结构体定义
struct Grad_Obj_return
{
	Eigen::MatrixXd gradient; //梯度矩阵
	Eigen::VectorXd c;		  //目标函数值
	MatrixXd J;				  //雅可比矩阵
	MatrixXd rib;             //肋板位置
	MatrixXd rxi;			  //弹性板等效关节位置
	Matrix4d gsc;             //接触坐标系
};
struct parameters
{
	int Nm;
	MatrixXd Tw;
	MatrixXd Kq;
	VectorXi NoB;
	VectorXi NoS;
	MatrixXd gb0;
	MatrixXd gs0;
	VectorXd dr;
	Matrix4d gt0;
	double dis1;
	Vector3d rS00;
	MatrixXd Seq;
};
// 机构当前状态记录
struct statusrecord
{
	VectorXd q;
	VectorXd Ft;
	VectorXd fr;
	double   fc;
	double   lc;
	parameters para;
};

typedef struct 
{
	VectorXd q;     //关节角度
	MatrixXd Jac;   //雅可比矩阵
	VectorXd motor; //电机驱动量
	MatrixXd rib;   //肋板位置
	MatrixXd rxi;   //弹性板等效关节位置
	//上述变量可以用于计算弹性板形状
	//暂时需要用到的结果为接触位置、大小和接触力大小
	Vector3d rC;    //接触位置
	Vector3d xC;    //接触方向
	double fc;      //接触力大小
	int error_code; //0 代表正常  1 代表有错误
}sensingresult;

// 矩阵指数函数的声明
MatrixXd wed(VectorXd x);//Wedge算子
Matrix4d expm(MatrixXd k, double theta);//矩阵指数函数声明
Matrix4d logm(MatrixXd x);//矩阵对数函数声明
MatrixXd pinv(MatrixXd A);//矩阵广义逆函数声明
MatrixXd Ad(MatrixXd mat);//李群伴随变换矩阵
MatrixXd ad(VectorXd ksi);//李代数伴随变换矩阵
VectorXd Vee(MatrixXd mat);//Vees算子
Matrix3d lambda(double theta);
MatrixXd expn(MatrixXd mat, int n);//矩阵幂函数

//finray接触与感知计算
void sns_statusinit(statusrecord* st); // 感知计算状态初始化
Grad_Obj_return finger_contact(VectorXd q, VectorXd Ft, VectorXd fr, double fc, double lc, parameters par, Vector4d obj);
Grad_Obj_return finger_sensing(VectorXd q, VectorXd Ft, VectorXd fr, double fc, double lc, parameters par, MatrixXd Seq,Vector4d ds0);
sensingresult SensingBlock(statusrecord* st, double Vol[4]);