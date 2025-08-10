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
// �����������ṹ�嶨��
struct Grad_Obj_return
{
	Eigen::MatrixXd gradient; //�ݶȾ���
	Eigen::VectorXd c;		  //Ŀ�꺯��ֵ
	MatrixXd J;				  //�ſɱȾ���
	MatrixXd rib;             //�߰�λ��
	MatrixXd rxi;			  //���԰��Ч�ؽ�λ��
	Matrix4d gsc;             //�Ӵ�����ϵ
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
// ������ǰ״̬��¼
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
	VectorXd q;     //�ؽڽǶ�
	MatrixXd Jac;   //�ſɱȾ���
	VectorXd motor; //���������
	MatrixXd rib;   //�߰�λ��
	MatrixXd rxi;   //���԰��Ч�ؽ�λ��
	//���������������ڼ��㵯�԰���״
	//��ʱ��Ҫ�õ��Ľ��Ϊ�Ӵ�λ�á���С�ͽӴ�����С
	Vector3d rC;    //�Ӵ�λ��
	Vector3d xC;    //�Ӵ�����
	double fc;      //�Ӵ�����С
	int error_code; //0 ��������  1 �����д���
}sensingresult;

// ����ָ������������
MatrixXd wed(VectorXd x);//Wedge����
Matrix4d expm(MatrixXd k, double theta);//����ָ����������
Matrix4d logm(MatrixXd x);//���������������
MatrixXd pinv(MatrixXd A);//��������溯������
MatrixXd Ad(MatrixXd mat);//��Ⱥ����任����
MatrixXd ad(VectorXd ksi);//���������任����
VectorXd Vee(MatrixXd mat);//Vees����
Matrix3d lambda(double theta);
MatrixXd expn(MatrixXd mat, int n);//�����ݺ���

//finray�Ӵ����֪����
void sns_statusinit(statusrecord* st); // ��֪����״̬��ʼ��
Grad_Obj_return finger_contact(VectorXd q, VectorXd Ft, VectorXd fr, double fc, double lc, parameters par, Vector4d obj);
Grad_Obj_return finger_sensing(VectorXd q, VectorXd Ft, VectorXd fr, double fc, double lc, parameters par, MatrixXd Seq,Vector4d ds0);
sensingresult SensingBlock(statusrecord* st, double Vol[4]);