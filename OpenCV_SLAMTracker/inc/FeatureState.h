#ifndef FEATURESTATE_H_INCLUDED
#define FEATURESTATE_H_INCLUDED

#include "stdafx.h"

/**
 *	\class FeatureState
 *	\brief ÿһ֡��������, ����new��delete���洢
 */
class FeatureState
{
public:
	/** ָ��_ImgIdx���Զ����ض�Ӧͼ��,���ز���throw���� */
	FeatureState(int _ImgIdx = -1);
	~FeatureState();
public:
	//��ǰ�������ݵ�ͼ����
	bool inited;
	int idxImg;
	std::vector<cv::KeyPoint> vecKeyPoints;
	std::vector<cv::Point2f> vecFeaturePoints;
	cv::Mat matImage;
	cv::Mat matDescriptor;

	bool loadImage(int _ImgIdx);
	friend std::ostream& operator<<(std::ostream& out, const FeatureState& fs);
	//void SetState(std::vector<cv::KeyPoint>& _vecKeyPoints, std::vector<cv::Point2f>& _vecFeaturePoints, cv::Mat& _matDescriptor);
	//void GetState(std::vector<cv::KeyPoint>& _vecKeyPoints, std::vector<cv::Point2f>& _vecFeaturePoints, cv::Mat& _matDescriptor) const;
};

#endif
