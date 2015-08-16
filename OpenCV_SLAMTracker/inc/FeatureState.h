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
	/** \fn ָ��_ImgIdx���Զ����ض�Ӧͼ��,���ز���throw���� */
	FeatureState(int _ImgIdx = -1);
	~FeatureState();
public:

	bool inited; /** \var �Ƿ��ʼ��(ͼ���Ƿ��ȡ�ɹ�) */
	int idxImg; /** \var ͼ���� */
	std::vector<cv::KeyPoint> vecKeyPoints; /** \var KeyPoint�ؼ��㼯�� */
	std::vector<cv::Point2f> vecFeaturePoints; /** \var KeyPoint����ȡ�����������㼯�� */
	cv::Mat matImage; /** \var ��ȡͼ��������� */
	cv::Mat matDescriptor; /** \var ͼ�������� */

	/** \fn ����ָ��idx��ͼ��ͼ��
	 *	\param _ImgIdx ָ��ͼ��idx, ʹ�������е�·�����ж�ȡ
	 *	\return bool �Ƿ���سɹ�
	 */
	bool loadImage(int _ImgIdx);

	/** \fn ����ָ�����ý����������
	 *	\param nFeatures SIFT������Ĳ���
	 *	\return int ��⵽������������
	 */
	int detect(int nFeatures = 0);

	/** \fn ostream�����Ԫ����
	 *	\brief �����ʽ
	 *		"FeatureState[%d]"
	 *		"Image:%d*%d"
	 *		"FeaturePoints:%d"
	 */
	friend std::ostream& operator<<(std::ostream& out, const FeatureState& fs);

private:

	/** \fn ��FeatureState�ڲ����ݵ�չ����ʽ */
	int detectExtractFeatures(int nFeatures, cv::Mat& matImage, std::vector<cv::KeyPoint>& vecKeyPoints, std::vector<cv::Point2f>& vecFeaturePoints, cv::Mat & matDescriptor, int idxImg);
private:

	/** \fn ������д����������
 	 *	\return bool �Ƿ�ɹ����ļ�
	 */
	static bool writeFeature(int idxImg, int nFeature, std::vector<cv::KeyPoint>& vecKeyPoints, std::vector<cv::Point2f>& vecFeaturePoints, cv::Mat & matDescriptor);
	/** \fn �ӻ��������������
	 *	\return bool �Ƿ�ɹ����ļ�
	 */
	static bool loadFeature(int idxImg, int nFeature, std::vector<cv::KeyPoint>& vecKeyPoints, std::vector<cv::Point2f>& vecFeaturePoints, cv::Mat & matDescriptor);

};

#endif
