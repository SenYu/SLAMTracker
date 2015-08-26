#ifndef CANVASDRAWER_H_INCLUDED
#define CANVASDRAWER_H_INCLUDED

#include "stdafx.h"
#include "PoseState.h"
#include "PoseHelper.h"
/**
* \class CanvasDrawer
* \brief ���ڻ��ƹ켣����
*/
class CanvasDrawer
{
private:
	bool inited;
public:
	/** \fn ��ʼ������,��Ҫָ��idxImg,Ĭ��inited(false) */
	CanvasDrawer(int _ImgIdx);
	~CanvasDrawer();

public:
	cv::Mat matScaleCanvas;	/** \var ��ǰscale���� */
	cv::Mat matPathCanvas;	/** \var ��ǰpath���� */
	PoseState	gPose;	/** \var ��һ����Ҫ���Ƶĵ� */

	/** \var ��������ڻ���(x,y)��ԭ��ƫ��
	 *	����Point��������Ҫ�������ƫ�Ʋ��ܻ���canvas��
	 */
	cv::Point2f gPointBase; 

	std::string recordFilePath; /** \var ·����¼�ļ�·�� */

	/** \var ·����¼�ļ�������
	 *	����poseҪ���12�����ݽ��м�¼
	 */
	std::fstream fileTraceRecord; 
	PoseHelper* ptrPoseHelper; /** \var GroundTruth���ݸ�������ָ��,Ĭ��NULL */
	int idxImgBegin; /** \var ������ʼidxImg */
	std::deque<double> qDist;
public:
	/**	\fn ����·����¼�ļ���ַ 
	 *	\param recordFilePath �ļ�·��
	 *	\return �����ļ����Ƿ�ɹ�
	 */
	bool setLogPath(const std::string& recordFilePath = "");
	/**
	 *	\fn ���������ʼ��
	 *	\param initPose,���Ƶĵ�һ����̬,��֮���������̬��������
	 */
	void initAnimate(PoseState& initPose);
	/**
	 *	\fn ���ƻ���
	 *	\param curPose,����ǰ��̬��gPose���߻���,�����Ʒ���
	 *	\param _isTruth, �Ƿ����GroundTruth·��
	 */
	void drawCanvas(PoseState& curPose, bool _isTruth = true);
private:
	/**
	*	\fn �Ե�ǰ�˶���̬���м�¼,���һ���scale
	*	\param curPose,��ǰ��̬
	*	\param prePose,��һ����̬
	*/
	void logPose(PoseState& curPose, PoseState& prePose);
};

#endif