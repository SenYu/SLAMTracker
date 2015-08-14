#ifndef TRACKRUNNER_H_INCLUDED
#define TRACKRUNNER_H_INCLUDED

#include "stdafx.h"
#include "CanvasDrawer.h"


/**
 * \class TrackRunner
 * \brief ��·����������ִ������
 */
class TrackRunner
{
public:
	TrackRunner();
	~TrackRunner();

private:
	// ��������
	CanvasDrawer cDrawer;

	// ������ʷ��¼

public:

	// �Ե�һ֡��Ҫ�����ݽ��г�ʼ��
	void initFirstFrame();

	// ����ÿһ֡�����ص�ǰ֡��
	int runKeyStep(int nextStep = -1); 

	void showFrameMotion();

	void showTrack();
};

#endif