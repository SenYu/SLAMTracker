#include "TrackRunner.h"
#include "CanvasDrawer.h"

TrackRunner::TrackRunner()
{
}


TrackRunner::~TrackRunner()
{
}


void TrackRunner::initFirstFrame() {

	// �趨�������
	cDrawer.setLogPath(cv::format("./Output/coordOutput_%s.txt", Utils::getTimeNow().c_str()));

	// ���ڲ���ʼ�� PoseHelper
	cDrawer.useGroundTruth(CFG_sPathPoseGroutTruth);

	// ��ʼ����������
	cDrawer.initAnimate();

}

int TrackRunner::runKeyStep(int nextStep) {

	return nextStep+1;
}

void TrackRunner::showFrameMotion() {


}

void TrackRunner::showTrack() {


}