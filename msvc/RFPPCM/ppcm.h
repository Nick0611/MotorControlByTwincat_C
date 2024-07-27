#pragma once
//#define EIGEN_USE_MKL_ALL
//#define EIGEN_VECTORIZE_SSE4_2
#include <Eigen\Dense>
#include <Eigen/SVD>
#include<math.h>
#include<stdio.h>
#include<omp.h>
#include<iostream>
using namespace Eigen;
using namespace std;
# define pi 3.1415926535
// �����������ṹ�嶨��
//struct Grad_inv_return
//{
//	Eigen::MatrixXd gradient;
//	Eigen::VectorXd c;
//};
struct Grad_fdetect_return
{
	Eigen::MatrixXd gradient;
	Eigen::VectorXd c;
};
struct Grad_forward_return
{
	Eigen::MatrixXd gradient;
	Eigen::VectorXd c;
	Eigen::Matrix4d g_st;
};

struct Force_perception_return
{
	bool success;
	Eigen::VectorXd theta1;
	Eigen::VectorXd theta2;
	Eigen::Vector2d Fe;
};

struct Forward_return
{
	Eigen::VectorXd theta1;
	Eigen::VectorXd theta2;
	Eigen::Matrix4d g_st;
};

// ������ǰ״̬��¼
struct statusrecord
{
	VectorXd theta1;
	VectorXd theta2;
	VectorXd F1;
	VectorXd F2;
	double M;
};

struct RFPCM_para
{
	double length_limb[2];
	int num_seg;
	double offset[2];
	double thickness;
	double width;
	double distance;
	double Ki;
};
struct Diamond_para
{
	double L1;
	double L2;
	double a;
	double b;
};

// ����ָ������������
MatrixXd wed(VectorXd x);//Wedge����
MatrixXd expm(MatrixXd k, double theta);//����ָ����������
Matrix4d logm(MatrixXd x);//���������������
MatrixXd pinv(MatrixXd A);//��������溯������
MatrixXd Ad(MatrixXd mat);//��Ⱥ����任����
MatrixXd ad(VectorXd ksi);//���������任����
VectorXd Vee(MatrixXd mat);//Vees����
Matrix3d lambda(double theta);
MatrixXd expn(MatrixXd mat, int n);//�����ݺ���
//�Ƴ����к���
void RemoveRow(Eigen::MatrixXd& matrix, unsigned int rowToRemove);
void RemoveColumn(Eigen::MatrixXd& matrix, unsigned int colToRemove);
void RemoveEle(Eigen::VectorXd& vector, unsigned int eleToRemove);
//����״̬��ʼ��
void inv_statusinit(statusrecord* st);
//���⹦�ܺ���
//Grad_inv_return Grad_inv(VectorXd theta1, VectorXd theta2, VectorXd F1, VectorXd F2, double M, Matrix4d g_t);
Grad_fdetect_return Grad_force_detect(RFPCM_para para, Vector2d P, Vector2d R, VectorXd theta1, VectorXd theta2, VectorXd F1, VectorXd F2, VectorXd Fe, Matrix4d g_st);
Grad_forward_return Grad_forward(RFPCM_para para, Vector2d P, Vector2d R, VectorXd theta1, VectorXd theta2, VectorXd F1, VectorXd F2, double M, Matrix4d g_st);
Forward_return Forward_kinematics(RFPCM_para para, Vector2d P, Vector2d R);
Force_perception_return Force_perception(RFPCM_para para, double x, double y, Vector2d P, Vector2d R);
void plot_result(RFPCM_para para, VectorXd theta1, VectorXd theta2, Vector2d P, Vector2d R, MatrixXd& L1, MatrixXd& L2);
Vector2d Diamond_forward(Diamond_para para, double inputangle[2]);
