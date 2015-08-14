#ifndef MOTIONSTATE_H_INCLUDED
#define MOTIONSTATE_H_INCLUDED

#include "stdafx.h"

//////////////////////////////////////////////////////////////////////////
/*
std::vector<cv::Mat> vecMatR, vecMatT, vecMatPos, vecMatDir;
vecMatR.reserve(_ImageLoadEnd + 1);
vecMatT.reserve(_ImageLoadEnd + 1);
vecMatPos.reserve(_ImageLoadEnd + 1);
vecMatDir.reserve(_ImageLoadEnd + 1);
 */

class MotionState
{
public:
	MotionState();
	~MotionState();
public:
	/** \var ǰ��Ա�����ͼƬ�ı�� */
	int idxImg[2];
	bool avaliable;
	//��ת, ƽ�ƾ���
	cv::Mat matR, matT;
	double degreeR, degreeT;
	//����,��������
	cv::Mat matPos, matDir;
	//ǰ����֡�� ��Ӧ��ӳ��
	//ƽ��ÿ֡ 500����ԣ�ÿ����8Bytes,һ�� 8K, 4000֡ 32MB.���Խ���
	std::map<cv::Point2f, cv::Point2f, Utils::PointComp<float>> mapPairPoints;

public:
	void SetState(cv::Mat& _matR, cv::Mat& _matT, cv::Point3d _matPos, cv::Point3d _matDir);
};

#endif