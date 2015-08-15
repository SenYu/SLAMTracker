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
	/** ���ص�ͼ����ŵ���ʼ{Begin}�ͽ���{End} */
	int idxImgBegin, idxImgEnd;
	/** ��ǰ�����е�ͼ����� */
	int idxImgCur;
protected:
	// ��������
	CanvasDrawer cDrawer;
	bool bIsInRotate;
	// ������ʷ��¼
	std::vector<PoseState> vecPoses; /** \var ��¼�ӿ�ʼ�����ڵ�����������̬(λ��,����) */
	std::vector<FeatureState*> vecKeyFrameFeatures; /** \var �ؼ�֡������¼(KeyP,P,Descrip) */
	std::deque<FeatureState*> deqFrameFeatures; /** \var ���֡���� */
	std::vector< std::map<int, MotionState>> vecMotionLinks; /** \var ĳ֡��֮ǰĳ֡�� �˶�״̬ */

protected:
	/** 
	 *	\fn �����֡���к͹ؼ�֡������ ѡ��һ������֡�����Թ�ƥ��  
	 *	��������С�� \var idxImg; �������麯�����䲻ͬ��ѡ�񷽷�
	 */
	virtual std::vector<FeatureState*> selectKeySequence(int idxImg = -1);
	virtual int filterMotions(std::vector<MotionState>& vecMotion);
	virtual void updateKeyList(FeatureState* ptrFeature);
public:

	// �Ե�һ֡��Ҫ�����ݽ��г�ʼ��
	void initFirstFrame();

	// ����ÿһ֡�����ص�ǰ֡��
	int runKeyStep(); 

	void showFrameMotion();

	void showTrack();
};

#endif