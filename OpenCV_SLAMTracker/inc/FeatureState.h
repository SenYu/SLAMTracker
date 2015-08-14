#ifndef FEATURESTATE_H_INCLUDED
#define FEATURESTATE_H_INCLUDED

#include "stdafx.h"

class FeatureState
{
public:
	FeatureState();
	~FeatureState();
public:
	//��ǰ�������ݵ�ͼ����
	int idxImg;
	std::vector<cv::KeyPoint> vecKeyPoints;
	std::vector<cv::Point2f> vecFeaturePoints;
	//����д��ֵ��ֱ�ӿ�������
	cv::Mat matDescriptor;

	void SetState(std::vector<cv::KeyPoint>& _vecKeyPoints, std::vector<cv::Point2f>& _vecFeaturePoints, cv::Mat& _matDescriptor);
	void GetState(std::vector<cv::KeyPoint>& _vecKeyPoints, std::vector<cv::Point2f>& _vecFeaturePoints, cv::Mat& _matDescriptor) const;
};

#endif
