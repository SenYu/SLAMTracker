#ifndef FRAMEPARSER_H_INCLUDED
#define FRAMEPARSER_H_INCLUDED

#include "stdafx.h"
#include "FeatureState.h"
#include "MotionState.h"

/**
*	\class FrameParser
*	\brief ֡�䴦����
*/
class FrameParser
{

public:
	cv::Mat matOrignImage[2]; /** \var ǰ����֡ͼ����� */
	cv::Mat matDescriptor[2]; /** \var ǰ����֡�����Ӿ��� */

	std::vector<cv::KeyPoint> vecKeyPoints[2]; /** \var ǰ����֡�ؼ��㼯�� */
	std::vector<cv::Point2f> vecFeaturePoints[2]; /** \var ǰ����֡�����㼯�� */
	std::vector<int> vecPairPointIdx[2]; /** \var ǰ����֡ƥ����������� */
	std::vector<cv::Point2f> vecPairPoint[2]; /** \var ǰ����֡ƥ��㼯�� */
	std::map<cv::Point2f, cv::Point2f, Utils::PointComp<float> > mapPairPoints; /** \var ��һ֡��ǰһ֡��mapӳ�� */

	std::vector<bool> vecFiltedMask; /** \var ���Ϲ����ɰ� */

	bool isTimeProfile;
	bool isShowImage;
	bool isLogData;
	int preImgIdx;
	int curImgIdx;
public:

	/** \fn ��ʼ������
	 *	\param prePtr ǰһ֡��������
	 *	\param curPtr ��һ֡��������
	 */
	FrameParser(FeatureState* prePtr, FeatureState* curPtr);
	~FrameParser();

	//��������
	/** \fn ��������ƥ��,���ҽ��й���ɸѡ */
	void match(double opThreshold = 2.0f);
	/** \fn ������֤ */
	void validPointsByOpticalFlow(double threshold = 1.0f);
	/** \fn �����˶����� */
	bool computeMotion(MotionState& motion, int minFundamentMatches = 25);


//	//��̬����
//	static int detectExtractFeatures(int nFeatures, FeatureState& fState);
//	static int detectExtractFeatures(int nFeatures, cv::Mat& matImage, std::vector<cv::KeyPoint>& vecKeyPoints, std::vector<cv::Point2f>& vecFeaturePoints, cv::Mat & matDescriptor, int idxImg);
//private:
//	static bool writeFeature(int idxImg, int nFeature, std::vector<cv::KeyPoint>& vecKeyPoints, std::vector<cv::Point2f>& vecFeaturePoints, cv::Mat & matDescriptor);
//	static bool loadFeature(int idxImg, int nFeature, std::vector<cv::KeyPoint>& vecKeyPoints, std::vector<cv::Point2f>& vecFeaturePoints, cv::Mat & matDescriptor);

};

#endif // FRAMEPARSER_H_INCLUDED