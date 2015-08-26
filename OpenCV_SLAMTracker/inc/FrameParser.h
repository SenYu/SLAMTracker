#ifndef FRAMEPARSER_H_INCLUDED
#define FRAMEPARSER_H_INCLUDED

#include "stdafx.h"
#include "FeatureState.h"
#include "MotionState.h"

/**
*	\class	FrameParser
*	\brief	֡�䴦����
*			������֡������FeatureState����, ��ȡ�����ݽ��ж�Ӧ��ƥ����˶�����
*/
class FrameParser
{

protected:
	cv::Mat matOrignImage[2]; /** \var ǰ����֡ͼ����� */
	cv::Mat matDescriptor[2]; /** \var ǰ����֡�����Ӿ��� */

	std::vector<cv::KeyPoint> vecKeyPoints[2]; /** \var ǰ����֡�ؼ��㼯�� */
	std::vector<cv::Point2f> vecFeaturePoints[2]; /** \var ǰ����֡�����㼯�� */
	std::vector<int> vecPairPointIdx[2]; /** \var ǰ����֡ƥ����������� */
	std::vector<cv::Point2f> vecPairPoint[2]; /** \var ǰ����֡ƥ��㼯�� */
	std::map<cv::Point2f, cv::Point2f, Utils::PointComp<float> > mapPairPoints; /** \var ��һ֡��ǰһ֡��mapӳ�� */
	std::vector<bool> vecFiltedMask; /** \var ���Ϲ����ɰ� */

	int preImgIdx;	/** \var ֡����� ��֡�е�ǰһ֡ */
	int curImgIdx; /** \var ֡����� ��֡�еĺ�һ֡ */
public:
	/** Getter method */
	std::map<cv::Point2f, cv::Point2f, Utils::PointComp<float> > getMapPairPointsConst() const { return mapPairPoints; }
	std::map<cv::Point2f, cv::Point2f, Utils::PointComp<float> >& getMapPairPointsRef() { return mapPairPoints; }

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

};

#endif // FRAMEPARSER_H_INCLUDED