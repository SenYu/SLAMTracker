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
	cv::Mat matScale;
	cv::Mat matCanvas;	/** \var ��ǰ���� */
	PoseState	gPose;	/** \var ��һ����Ҫ���Ƶĵ� */
	cv::Point2f gPointBase; /** \var ��������ڻ���(x,y)��ԭ��ƫ�� */

	std::string recordFilePath;
	std::fstream fileTraceRecord; /** \var ·����¼�ļ� */
	PoseHelper* ptrPoseHelper; /** \var GroundTruth���ݸ�������ָ��,Ĭ��NULL */
	int idxImgBegin; /** \var ������ʼidxImg */

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
	void logPose(PoseState& curPose, PoseState& prePose);
};

#endif