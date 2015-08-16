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
	//����ɾ��new����������FeatureState* ��¼
	for (FeatureState* ptr : vecKeyFrameFeatures)
		delete ptr;
	for (FeatureState* ptr : deqFrameFeatures)
		delete ptr;
}


void TrackRunner::initFirstFrame() {

	// �趨�������
	cDrawer.setLogPath(cv::format("./Output/coordOutput_%s.txt", Utils::getTimeNow().c_str()));
	// ��ʼ�� PoseHelper
	pHelper.setGroundTruth(CFG_sPathPoseGroutTruth);
	// ��Drawer����groundTruth
	cDrawer.ptrPoseHelper = &pHelper;

	//��ʼ���������̬
	//TODO: ������Ǵ�0��ʼ�Ļ�����Ҫ�ѳ�ʼ�Ƕ�����Ϊ GroundTruth�� ����, ����pos����
	vecPoses[idxImgBegin].dir = Const::pnt3d_001;
	vecPoses[idxImgBegin].pos = Const::pnt3d_000;
	vecPoses[idxImgBegin].inited = true;
	vecPoses[idxImgBegin].idxImg = idxImgBegin;

	// �������������ʼ��
	cDrawer.initAnimate(vecPoses[idxImgBegin]);

	// ��ʼ��idx = idxImgBegin ������������������
	FeatureState* ptrCurFeature = new FeatureState(idxImgBegin);
	ptrCurFeature->detect(CFG_iMaxFeatures);

	// ��������ö�����ѹ��
	deqFrameFeatures.push_back(ptrCurFeature);

	//�´����д� idxImgBegin+1 ��ʼ
	idxImgCur = idxImgBegin + 1;
}

// [��] �ض�֡���ݼ�¼ int<-> ��ǰλ�ã�����, ȫ��¼
// [��] �ض�֡���ݼ�¼ int<-> ������ vecKP, vecP, matDescriptor->�洢�������� 2000K һ��,���ּ�¼
// [��] ֡�����ݼ�¼ �� vector< int> --- ��map<int,��>> ��matR, matT, Scale ȫ��¼

// ��չ: [�ؼ�֡]�� �� [�����������֡]��, ÿ����һ֡�������[���]�ӣ�[���]�Ǹ���������, ������ˣ����Ҿ��빻�� �ӽ� [�ؼ�֡]����ȥ���������ٵ�

//��ǰӦ���� nextStep��һ֡

// �ȴ���ʷ������ѡ�� K ������ο�֡ + ALL �ؼ�֡���� match
// ѡȡһ��֡ptr ����
int TrackRunner::runKeyStep() {

	printf("////////////////////////////////////////////\n");
	printf("//////////// idxImgCur: %06d ////////////\n",idxImgCur);
	printf("////////////////////////////////////////////\n");

	TIME_BEGIN("MainLoop");

		TIME_BEGIN("FeatureDetect");
		/** TODO: ���Կ��� ���̲߳��м���,Ȼ��������Ÿ� wait
		 *		  �����Ǹ��߳����һ���Ÿ� signal
		 */
		// ��ȡ��ǰ֡����
		FeatureState* ptrCurFeature = new FeatureState(idxImgCur);
		ptrCurFeature->detect(CFG_iMaxFeatures);
		TIME_END("FeatureDetect");

		// ȷ����ǰ֡��Ҫ�Ƚϵ� ��ʷ֡����
		std::vector<FeatureState*> vecKeyList;
		/** TODO: ������Ҫ�� �Ӿ��Ŵ�ģ��Ԥ��closure */
		vecKeyList = selectKeySequence(idxImgCur);

		// ��ÿһ����ʷ-��ǰ֡ �����˶�����
		std::vector<MotionState> vecCurMotions( vecKeyList.size() );
		ScaleEstimator sEstimator;

		// ���� FrameParser �ֱ��� �ο�֡<->��ǰ֡ �� matR��MatT
		TIME_BEGIN(cv::format("Match&Motion[%d]", vecKeyList.size()));
		for (int idx = 0; idx < vecKeyList.size();idx++) {
			FeatureState* ptrFeature = vecKeyList[idx];

			/** TODO: ���ǶԲ�ͬ������ò�ͬ���˹��� */
			// ��ʼ�� ֡�䴦����
			FrameParser fparser(ptrFeature, ptrCurFeature);
			fparser.match(2.0f);

			// �Լ����� �����˶�matR��matT
			vecCurMotions[idx].idxImg[0] = ptrFeature->idxImg;
			vecCurMotions[idx].idxImg[1] = ptrCurFeature->idxImg;
			vecCurMotions[idx].mapPairPoints = fparser.mapPairPoints;
			bool motionStatus = fparser.computeMotion(vecCurMotions[idx]);
			
			// �˶���������ʧ��
			if (motionStatus == false) {
				continue;
			}

			// �߶ȹ��㿪��
			sEstimator.updateMotion(&vecCurMotions[idx]);
			double scale = sEstimator.computeScaleTransform();

			/** TODO: ������Ҫһ�����ʵĳ߶����� */
			if ( scale<0 || scale < 0.4f || scale > 20.0f) { 
				motionStatus = false; 
				vecCurMotions[idx].inited = false;
				continue; 
			}

			// Ŀǰʹ�� GroundTruth�� �˶��߶ȣ����Ŀǰ���ֻ����ת�Ƕ���
			//vecCurMotions[idx].scale = scale;
			vecCurMotions[idx].scale = 1.65f/cv::norm(pHelper.getPosition(vecCurMotions[idx].idxImg[1], vecCurMotions[idx].idxImg[0]));

		}
		TIME_END(cv::format("Match&Motion[%d]", vecKeyList.size()));

		//����һ��������� vecCurMotions,Ŀǰ�ȹ��˵� initedΪfalse��
		int cntValid = filterMotions(vecCurMotions);

		// ���������û����, ����֡
		if (cntValid < 1) {
			delete ptrCurFeature;
		}
		else {
			/** TODO: �����������ô����Ч��, 1��Ҫ�ж�closure; 2��Ҫ�ۺ�(ȡƽ��������)�˶�״̬ */
			TIME_BEGIN("PoseEstimate");

			// ���ڼ��������ÿ��Motion, Ӧ�ø��˶�
			std::vector<PoseState> vecEstiPoses(vecCurMotions.size());
			for (int idx = 0; idx < vecCurMotions.size(); idx++) {
				MotionState& curMotion = vecCurMotions[idx];
				if (curMotion.inited == true) {
					// ��vecPose�Ķ�Ӧλ��,Ӧ��Motion�õ� vecEstiPoses��λ��
					vecEstiPoses[idx] = vecPoses[curMotion.idxImg[0]].move(curMotion);
				}
			}
			
			/** TODO: ��Ҫ�и������ж���ǰ�Ƿ���ת��. */
			if (std::abs(vecCurMotions[0].degreeT) > 5.0f) {
				printf("...\n");
				bIsInRotate = true;
			}
			else {
				bIsInRotate = false;
			}

			/** TODO: �µĶ�λ��ѡ�񷽷� */
			// �Զ������õ�����λ��,���յȱ����� 1/2; 1/4; 1/8���� ��Ȩƽ��....��low
			PoseState curPoseState = vecEstiPoses[0];
			int vecEstLen = vecEstiPoses.size();
			if (vecEstLen >= 2) {
				curPoseState.pos = Const::pnt3d_000;
				curPoseState.dir = Const::pnt3d_000;
				double pow[] = { 1.0f / 2.0f, 1.0f / 4.0f, 1.0f / 8.0f, 1.0f / 16.0f };
				
				for (int idx = 0; idx < vecEstLen; idx++) {
					curPoseState.pos += vecEstiPoses[idx].pos * pow[idx];
					curPoseState.dir += vecEstiPoses[idx].dir * pow[idx];
				}
				curPoseState.pos += vecEstiPoses[vecEstLen - 1].pos * pow[vecEstLen - 1];
				curPoseState.dir += vecEstiPoses[vecEstLen - 1].dir * pow[vecEstLen - 1];

				curPoseState.dir = curPoseState.dir * (1.0f/cv::norm(curPoseState.dir));
			}

			// ���¶��к͹ؼ�֡
			updateKeyList(ptrCurFeature);
			vecPoses[idxImgCur] = curPoseState;

			TIME_END("PoseEstimate");

			std::cout << curPoseState << std::endl;
			// ����������ǰλ��, �Լ�GroundTruthʵ��·��
			cDrawer.drawCanvas(curPoseState);
		}

	TIME_END("MainLoop");

	return ++idxImgCur;
}

void TrackRunner::showFrameMotion() {


}

void TrackRunner::showTrack() {


}

std::vector<FeatureState*> TrackRunner::selectKeySequence(int idxImg) {
	std::vector<FeatureState*> retVec;
	
	// ���������������������
	auto iter = deqFrameFeatures.begin();
	for (; iter != deqFrameFeatures.end();iter++)
		retVec.push_back( *iter );

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