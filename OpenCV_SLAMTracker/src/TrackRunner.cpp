#include "TrackRunner.h"
#include "CanvasDrawer.h"
#include "FrameParser.h"
#include "ScaleEstimator.h"

TrackRunner::TrackRunner(int _ImgBeginIdx, int _ImgEndIdx):
cDrawer(_ImgBeginIdx),
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

	//��ʼ���������̬
	//TODO: ������Ǵ�0��ʼ�Ļ�����Ҫ�ѳ�ʼ�Ƕ�����Ϊ GroundTruth�� ����, ����pos����
	vecPoses[idxImgBegin].dir = Const::pnt3d_001;
	vecPoses[idxImgBegin].pos = Const::pnt3d_000;
	vecPoses[idxImgBegin].inited = true;
	vecPoses[idxImgBegin].idxImg = idxImgBegin;

	// ��ʼ����������
	cDrawer.initAnimate(vecPoses[idxImgBegin]);

	//����idx = idxImgBegin ʱ��� FeatureState, 
	FeatureState* ptrCurFeature = new FeatureState(idxImgBegin);

	//TODO: ������������ܰ��뵽 FeatureState ��,������ FrameParser
	FrameParser::detectExtractFeatures(CFG_iMaxFeatures, *ptrCurFeature);

	//������ö�����ѹ��
	deqFrameFeatures.push_back(ptrCurFeature);

	//�´����д� idxImgBegin+1 ��ʼ
	idxImgCur = idxImgBegin + 1;
}

int TrackRunner::runKeyStep() {
	printf("///////////////////////////////////////////\n");
	printf("///////// idxImgCur: %06d /////////\n",idxImgCur);
	printf("///////////////////////////////////////////\n");
	TIME_BEGIN("MainLoop");

		TIME_BEGIN("FeatureDetect");
		// ��ȡ��ǰ֡����
		//���Կ��� ���̲߳��м���,Ȼ��������Ÿ� wait
		//�����Ǹ��߳����һ���Ÿ� signal
		FeatureState* ptrCurFeature = new FeatureState(idxImgCur);
		FrameParser::detectExtractFeatures(CFG_iMaxFeatures, *ptrCurFeature);

		std::cout << *ptrCurFeature << std::endl;
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
			if (fparser.vecPairPoint[0].size() > 0 && bIsInRotate == false)
				fparser.validPointsByOpticalFlow(1.0f);

			//����Լ�����
			vecCurMotions[idx].idxImg[0] = ptrFeature->idxImg;
			vecCurMotions[idx].idxImg[1] = ptrCurFeature->idxImg;
			bool motionStatus = fparser.computeMotion(vecCurMotions[idx]);
			//����߶�
			vecCurMotions[idx].mapPairPoints = fparser.mapPairPoints;

			//���ι���
			//MotionState tmpMotion;
			//tmpMotion.idxImg[0] = vecCurMotions[idx].idxImg[0];
			//tmpMotion.idxImg[1] = vecCurMotions[idx].idxImg[1];

			//fparser.validPointsByOpticalFlow(1.0f);


			if (motionStatus == false) {
				vecCurMotions[idx].inited = false;
				continue;
			}

			sEstimator.updateMotion(&vecCurMotions[idx]);
			double scale = sEstimator.computeScaleTransform();

			if ( scale<0 || scale < 0.4f || scale > 20.0f) { 
				motionStatus = false; 
				vecCurMotions[idx].inited = false;
				continue; 
			}

			vecCurMotions[idx].scale = scale;

			std::cout << vecCurMotions[idx] << std::endl;
			//����vecCurMotions
		}
		TIME_END(cv::format("Match&Motion[%d]", vecKeyList.size()));


		//����һ��������� vecCurMotions,���ǹ��˵��ģ���Ҫdelete Motion
		int cntValid = filterMotions(vecCurMotions);

		// ���ݵ�������� delete CurFeature
		if (cntValid < 1) {
			std::cout << "���ݵ����" << std::endl;
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
					std::cout << vecPoses[curMotion.idxImg[0]] << std::endl;
					std::cout << curMotion << std::endl;
					vecEstiPoses[idx] = vecPoses[curMotion.idxImg[0]].move(curMotion);
					std::cout << vecEstiPoses[idx] << std::endl;
				}
			}
			// ����µ� PoseState;ƽ������ʲô��С����
			// vecEstiPoses �� true�� ���ƽ��ֵ?
			
			if (std::abs(vecCurMotions[0].degreeT) > 7.0f) {
				printf("...\n");
				bIsInRotate = true;
			}
			else {
				bIsInRotate = false;
			}

			PoseState curPoseState = vecEstiPoses[0];
			if (curPoseState.inited == false) {
				throw std::exception("����Ŀ���", 1);
			}
			//���¶��к͹ؼ�֡
			updateKeyList(ptrCurFeature);
			vecPoses[idxImgCur] = curPoseState;


			TIME_END("PoseEstimate");

			std::cout << curPoseState << std::endl;

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
	auto iter = deqFrameFeatures.begin();
	for (; iter != deqFrameFeatures.end();iter++)
		retVec.push_back( *iter );

	//if (retVec.size() > 2)
	//	throw std::exception("����2", 2);

	return retVec;
}

int TrackRunner::filterMotions(std::vector<MotionState>& vecMotion) {

	auto iter = vecMotion.begin();
	for (; iter != vecMotion.end();) {
		if (iter->inited == false) {
			//eraseִ�к󷵻���һ��������, �����end()�Ļ���++�ͻ�ҵ�
			iter = vecMotion.erase(iter);
		}
		else {
			iter++;
		}
	}
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