#ifndef POSEHELPER_H_INCLUDED
#define POSEHELPER_H_INCLUDED

#include "stdafx.h"

/**
 * \class PoseHelper
 * \brief ���ڶ�ȡGroundTruth��Poseλ��,���洢�Թ�����
 */
class PoseHelper
{
public:
	PoseHelper();
	~PoseHelper();
private:
	std::fstream fileGroundTruth;
	std::vector<cv::Point3d> vecPosesTruth;
	bool readPose(cv::Point3d& newPoint);
public:
	/** \fn ����fileGroundTruth��ȡ·�� */
	bool setGroundTruth(const std::string groundTruthPath = "");
	/** \fn ����idxImg,�����������֮ǰĳһ���λ�Ʋ�, <=0Ϊ���������0�� */
	cv::Point3d getPosition(int idxImg, int relativeIdx = 0);
	/** \fn ����pose�ļ��Ƿ�ɹ��� */
	bool inited();
};

#endif