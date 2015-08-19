#ifndef TRACKRUNNER_H_INCLUDED
#define TRACKRUNNER_H_INCLUDED

#include "stdafx.h"
#include "CanvasDrawer.h"
#include "PoseState.h"
#include "FeatureState.h"
#include "MotionState.h"

/**
 * \class TrackRunner
 * \brief ��·����������ִ������
 */
class TrackRunner
{
public:
	TrackRunner(int _ImgBeginIdx, int _ImgEndIdx);
	~TrackRunner();
protected:
	/** \var ���ص�ͼ����ŵ���ʼ{Begin}�ͽ���{End} */
	int idxImgBegin, idxImgEnd;
	/** \var ��ǰ�����е�ͼ����� */
	int idxImgCur;
	int cntRunOk;
protected:
	/** \var �������� */
	CanvasDrawer cDrawer;
	/** \var GroundTruth·�������� */
	PoseHelper pHelper;
	PoseHelper pVisio;
	bool bIsInRotate;

	// ������ʷ��¼
	std::vector<PoseState> vecPoses; /** \var ��¼�ӿ�ʼ�����ڵ�����������̬(λ��,����) */
	std::vector<FeatureState*> vecKeyFrameFeatures; /** \var �ؼ�֡������¼(KeyP,P,Descrip) */
	std::deque<FeatureState*> deqFrameFeatures; /** \var ���֡���� */
	std::vector< std::map<int, MotionState>> vecMotionLinks; /** \var ĳ֡��֮ǰĳ֡�� �˶�״̬ */
	std::vector<int> vecEnableIdxs; /** ���ñ�� */
protected:
	/** 
	 *	\fn �����֡���к͹ؼ�֡������ ѡ��һ������֡�����Թ�ƥ��  
	 *	��������С�� \var idxImg; �������麯�����䲻ͬ��ѡ�񷽷�
	 */
	virtual std::vector<FeatureState*> selectKeySequence(int idxImg = -1);
	virtual int filterMotions(std::vector<MotionState>& vecMotion, double oldDegreeT);
	virtual void updateKeyList(FeatureState* ptrFeature);
	virtual bool limitRotationDiff(MotionState& curMotion, double limit); /** �ж�λ��ƫ�� */
	virtual bool limitScaleDiff(MotionState& curMotion,double& curScale, double limit);
public:

	/** \fn �Ե�һ֡��Ҫ�����ݽ��г�ʼ�� */
	void initFirstFrame();

	/** \fn ����ÿһ֡�����ص�ǰ֡�� */
	int runKeyStep(); 

	void showFrameMotion();

	void showTrack();

	void lm();


};

#endif