#ifndef FEATURESTATE_H_INCLUDED
#define FEATURESTATE_H_INCLUDED

#include "stdafx.h"

/**
 *	\class FeatureState
 *	\brief	ÿһ֡��������, ����new��delete���洢
 *			���ڸ���һ��ͼ���ţ���ȡ��Ӧͼ�񣬽����������⣬������������
 */
class FeatureState
{
public:
	/** \fn ָ��_ImgIdx���Զ����ض�Ӧͼ��,���ز���throw���� */
	FeatureState(int _ImgIdx = -1);
	~FeatureState();

protected:
	bool inited; /** \var �Ƿ��ʼ��(ͼ���Ƿ��ȡ�ɹ�) */
	int idxImg; /** \var ���������Ӧ��ͼ���� */

	std::vector<cv::KeyPoint> vecKeyPoints; /** \var KeyPoint�ؼ��㼯�� */
	/** \var KeyPoint����ȡ�����������㼯��
	 *	KeyPoint��Point2f��ת�� 
	 */
	std::vector<cv::Point2f> vecFeaturePoints; 
	cv::Mat matImage; /** \var ��ȡͼ��������� */
	cv::Mat matDescriptor; /** \var ͼ�������� */
	
public:

	/** getter method */
	bool getInited() const { return inited; }
	int getIdxImg() const { return idxImg; }

	/** const���ͷ�����, ��const������ */
	std::vector<cv::KeyPoint> getVecKeyPointsConst() const { return vecKeyPoints; }
	std::vector<cv::KeyPoint>& getVecKeyPointsRef() { return vecKeyPoints; }

	std::vector<cv::Point2f> getVecFeaturePointsConst() const { return vecFeaturePoints; }
	std::vector<cv::Point2f>& getVecFeaturePointsRef() { return vecFeaturePoints; }

	cv::Mat getMatImageConst() const { return matImage.clone(); }
	cv::Mat& getMatImageRef() { return matImage; }

	cv::Mat getMatDescriptorConst() const { return matDescriptor.clone(); }
	cv::Mat& getMatDescriptorRef() { return matDescriptor; }

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

protected:
	/** \brief virtual����������д */

	/** \fn ��FeatureState�ڲ����ݵ�չ����ʽ */
	virtual int detectExtractFeatures(int nFeatures, cv::Mat& matImage, std::vector<cv::KeyPoint>& vecKeyPoints, std::vector<cv::Point2f>& vecFeaturePoints, cv::Mat & matDescriptor, int idxImg);

	/** \fn ������д����������
 	 *	\return bool �Ƿ�ɹ����ļ�
	 */
	virtual bool writeFeature(int idxImg, int nFeature, std::vector<cv::KeyPoint>& vecKeyPoints, std::vector<cv::Point2f>& vecFeaturePoints, cv::Mat & matDescriptor);
	
	/** \fn �ӻ��������������
	 *	\return bool �Ƿ�ɹ����ļ�
	 */
	virtual bool loadFeature(int idxImg, int nFeature, std::vector<cv::KeyPoint>& vecKeyPoints, std::vector<cv::Point2f>& vecFeaturePoints, cv::Mat & matDescriptor);

};

#endif
