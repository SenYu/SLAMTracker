#ifndef MOTIONSTATE_H_INCLUDED
#define MOTIONSTATE_H_INCLUDED

#include "stdafx.h"

/**
 *	\class MotionState
 *	\brief �˶�״̬���洢��֡��任����, δ��ʼ����.inited����Ϊfalse
 */
class MotionState
{
public:
	MotionState();
	~MotionState();
public:
	/** \var ǰ��Ա�����ͼƬ�ı�� */
	int idxImg[2];
	bool inited;
	//��ת, ƽ�ƾ���
	cv::Mat matR, matT;
	//TODO: �ǵøĳ� get()��ʽ������set
	double degreeR, degreeT;
	double scale;
	//ǰ����֡�� ��Ӧ��ӳ��
	//ƽ��ÿ֡ 500����ԣ�ÿ����8Bytes,һ�� 8K, 4000֡ 32MB.���Խ���
	std::map<cv::Point2f, cv::Point2f, Utils::PointComp<float>> mapPairPoints;

public:
	/** ���degreeR��degreeT, δ����(-100)�����㣬����ֱ�ӷ��� */
	double getDegree(const std::string& str);
	void setState(cv::Mat& _matR, cv::Mat& _matT);
};

#endif