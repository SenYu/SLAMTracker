#include "TrackRunner.h"
#include "CanvasDrawer.h"
#include "FrameParser.h"
#include "ScaleEstimator.h"

TrackRunner::TrackRunner(int _ImgBeginIdx, int _ImgEndIdx):
cDrawer(),
idxImgBegin(_ImgBeginIdx), idxImgEnd(_ImgEndIdx),
vecPoses(_ImgEndIdx+1), vecMotionLinks(_ImgEndIdx+1)
{
}


TrackRunner::~TrackRunner()
{
	//TODO: ����ɾ�� ����FeatureState* ��¼
}


void TrackRunner::initFirstFrame() {

	// �趨�������
	cDrawer.setLogPath(cv::format("./Output/coordOutput_%s.txt", Utils::getTimeNow().c_str()));

	// ���ڲ���ʼ�� PoseHelper
	cDrawer.useGroundTruth(CFG_sPathPoseGroutTruth);

	// ��ʼ����������
	cDrawer.initAnimate();

	//����һ��
	FeatureState* ptrCurFeature = new FeatureState(idxImgBegin);
	ptrCurFeature->loadImage(idxImgBegin);

	FrameParser::detectExtractFeatures(CFG_iMaxFeatures, *ptrCurFeature);
	deqFrameFeatures.push_back(ptrCurFeature);

	idxImgCur = idxImgBegin + 1;
}

int TrackRunner::runKeyStep() {
	printf("///////////////////////////////////////////\n");
	printf("// idxImgCur: %d:\n",idxImgCur);
	printf("///////////////////////////////////////////\n");
	TIME_BEGIN("MainLoop");

		TIME_BEGIN("FeatureDetect");
		// ��ȡ��ǰ֡����
		FeatureState* ptrCurFeature = new FeatureState(idxImgCur);
		FrameParser::detectExtractFeatures(CFG_iMaxFeatures, *ptrCurFeature);
		TIME_END("FeatureDetect");
	// [��] �ض�֡���ݼ�¼ int<-> ��ǰλ�ã�����, ȫ��¼
	// [��] �ض�֡���ݼ�¼ int<-> ������ vecKP, vecP, matDescriptor->�洢�������� 2000K һ��,���ּ�¼
	// [��] ֡�����ݼ�¼ �� vector< int> --- ��map<int,��>> ��matR, matT, Scale ȫ��¼

	// ��չ: [�ؼ�֡]�� �� [�����������֡]��, ÿ����һ֡�������[���]�ӣ�[���]�Ǹ���������, ������ˣ����Ҿ��빻�� �ӽ� [�ؼ�֡]����ȥ���������ٵ�

	//��ǰӦ���� nextStep��һ֡

	// �ȴ���ʷ������ѡ�� K ������ο�֡ + ALL �ؼ�֡���� match
	// ѡȡһ��֡ptr ����
	std::vector<FeatureState*> vecKeyList;
	vecKeyList = selectKeySequence(idxImgCur);

	// ���� FrameParser �ֱ��� �ο�֡<->��ǰ֡ �� matR��MatT
	std::vector<MotionState> vecCurMotions( vecKeyList.size() );
	ScaleEstimator sEstimator;

	TIME_BEGIN(cv::format("Match&Motion[%d]", vecKeyList.size()));
	for (int idx = 0; idx < vecKeyList.size();idx++) {
		FeatureState* ptrFeature = vecKeyList[idx];
		FrameParser fparser(ptrFeature, ptrCurFeature);
		fparser.matchFeaturePoints();
		fparser.validPointsByOpticalFlow(1.0f);

		//����Լ�����
		vecCurMotions[idx].idxImg[0] = ptrFeature->idxImg;
		vecCurMotions[idx].idxImg[1] = ptrCurFeature->idxImg;
		bool motionStatus = fparser.computeMotion(vecCurMotions[idx]);
		//����߶�
		if (motionStatus == false) continue;

		sEstimator.updateMotion(&vecCurMotions[idx]);
		double scale = sEstimator.computeScaleTransform();
		vecCurMotions[idx].scale = scale;
		//����vecCurMotions
	}
	TIME_END(cv::format("Match&Motion[%d]", vecKeyList.size()));


	//����һ��������� vecCurMotions,���ǹ��˵��ģ���Ҫdelete Motion
	int cntValid = filterMotions(vecCurMotions);

	// ���ݵ�������� delete CurFeature
	if (cntValid < 1) {
		delete ptrCurFeature;
		//TODO: ��֡
	}
	else {
		TIME_BEGIN("PoseEstimate");

		
		std::vector<PoseState> vecEstiPoses(vecCurMotions.size());
		for (int idx = 0; idx < vecCurMotions.size(); idx++) {
			MotionState& curMotion = vecCurMotions[idx];
			if (curMotion.inited == true) {
				//�� vecCurMotions[idx].idxImg[0]  ->  [1]
				//ִ��һ�����㣬�õ�һ�� PoseState, 
				vecEstiPoses[idx] = vecPoses[curMotion.idxImg[0]].Move(curMotion);
			}
		}
		// ����µ� PoseState;ƽ������ʲô��С����
		// vecEstiPoses �� true�� ���ƽ��ֵ?

		PoseState curPoseState(idxImgCur);

		//���¶��к͹ؼ�֡
		updateKeyList(ptrCurFeature);
		vecPoses[idxImgCur] = curPoseState;

		TIME_END("PoseEstimate");

		cDrawer.drawCanvas(curPoseState);
	}
	// ����� ��ǰ֡�йص�K�� ����, ȡƽ����������ô�� �����ǰ֡�� ��������

	// ���� CanvasDraw ������������groundtruth
	TIME_END("MainLoop");

	return ++idxImgCur;
}

void TrackRunner::showFrameMotion() {


}

void TrackRunner::showTrack() {


}

std::vector<FeatureState*> TrackRunner::selectKeySequence(int idxImg) {
	std::vector<FeatureState*> retVec;
	//��ʱ���� ǰһ֡
	retVec.push_back( deqFrameFeatures.back() );

	return retVec;
}

int TrackRunner::filterMotions(std::vector<MotionState>& vecMotion) {


	return vecMotion.size();
}

void TrackRunner::updateKeyList(FeatureState* ptrFeature) {

	while(deqFrameFeatures.size() >= 1) {

		FeatureState* ptrPop = deqFrameFeatures.front();
		deqFrameFeatures.pop_front();
		delete ptrPop;
	}

	deqFrameFeatures.push_back(ptrFeature);
	
}