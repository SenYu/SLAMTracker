#include "TrackRunner.h"
#include "CanvasDrawer.h"
#include "FrameParser.h"
#include "ScaleEstimator.h"

TrackRunner::TrackRunner(int _ImgBeginIdx, int _ImgEndIdx):
cDrawer(_ImgBeginIdx),
idxImgBegin(_ImgBeginIdx), idxImgEnd(_ImgEndIdx),
vecPoses(_ImgEndIdx+1), vecMotionLinks(_ImgEndIdx+1),
vecEnableIdxs(_ImgEndIdx+1),
cntRunOk(0)
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
	pVisio.setGroundTruth("./Poses/00_.txt");
	// ��Drawer����groundTruth
	cDrawer.ptrPoseHelper = &pHelper;

	//��ʼ���������̬
	//TODO: ������Ǵ�0��ʼ�Ļ�����Ҫ�ѳ�ʼ�Ƕ�����Ϊ GroundTruth�� ����, ����pos����
	vecPoses[idxImgBegin].dir = Const::pnt3d_001;
	vecPoses[idxImgBegin].pos = Const::pnt3d_000;
	vecPoses[idxImgBegin].dir3 = Const::mat33_111.clone();
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
		

		// ���� FrameParser �ֱ��� �ο�֡<->��ǰ֡ �� matR��MatT
		TIME_BEGIN(cv::format("Match&Motion[%d]", vecKeyList.size()));
#pragma omp parallel for
		for (int idx = 0; idx < vecKeyList.size();idx++) {
			FeatureState* ptrFeature = vecKeyList[idx];

			/** TODO: ���ǶԲ�ͬ������ò�ͬ���˹��� */
			// ��ʼ�� ֡�䴦����
			FrameParser fparser(ptrFeature, ptrCurFeature);
			fparser.match(CFG_dOpticalFlowThreshold);
		/*	if (fparser.vecPairPoint[0].size() > 0 && bIsInRotate == false)
				fparser.validPointsByOpticalFlow(1.0f);*/


			// �Լ����� �����˶�matR��matT
			vecCurMotions[idx].idxImg[0] = ptrFeature->idxImg;
			vecCurMotions[idx].idxImg[1] = ptrCurFeature->idxImg;
			vecCurMotions[idx].mapPairPoints = fparser.mapPairPoints;
			bool motionStatus = fparser.computeMotion(vecCurMotions[idx]);
			vecEnableIdxs[idxImgCur] = cntRunOk;
			// �˶���������ʧ��
			if (motionStatus == false) {
				continue;
			}

			// λ����ת��ֵ
			if (CFG_bIsLimitRotationDiff && limitRotationDiff(vecCurMotions[idx],CFG_dRotationDiffLimit) == false) {
				vecCurMotions[idx].inited = false;
				continue;
			}

			// �߶ȹ��㿪��
			ScaleEstimator sEstimator;
			sEstimator.updateMotion(&vecCurMotions[idx]);
			double curScale = sEstimator.computeScaleTransform();

			// �߶���������
			if (limitScaleDiff(vecCurMotions[idx], curScale, CFG_dScaleInvIncreaseDiffLimit) == false) {
				vecCurMotions[idx].inited = false;
				continue;
			}

			/** TODO: ������Ҫһ�����ʵĳ߶����� */
			int idxDelta = vecCurMotions[idx].idxImg[1] - vecCurMotions[idx].idxImg[0];
			if (curScale<0 || ( (curScale < CFG_dScaleRatioLimitBottom /*/ idxDelta*/ || curScale > CFG_dScaleRatioLimitTop) )  ) {
				if (idxDelta > 5) {
					vecCurMotions[idx].inited = true;
					if (curScale < CFG_dScaleRatioLimitBottom) {
						curScale = CFG_dScaleRatioLimitBottom;
					}
					else
						curScale = CFG_dScaleRatioLimitTop;
				}
				else {
					motionStatus = false;
					vecCurMotions[idx].inited = false;
					continue;
				}
				
			}


			// Ŀǰʹ�� GroundTruth�� �˶��߶ȣ����Ŀǰ���ֻ����ת�Ƕ���
			if (CFG_bIsUseGroundTruthDistance) {
				vecCurMotions[idx].scale = 1.65f / cv::norm(pHelper.getPosition(vecCurMotions[idx].idxImg[1], vecCurMotions[idx].idxImg[0]));
				//vecCurMotions[idx].scale = 1.65f / cv::norm(pVisio.getPosition(vecCurMotions[idx].idxImg[1], vecCurMotions[idx].idxImg[0]));
			}
			else {
				vecCurMotions[idx].scale = curScale;
			}
			vecCurMotions[idx]._matR_ = vecCurMotions[idx].matR * vecPoses[vecCurMotions[idx].idxImg[0]].dir3;

		}
		TIME_END(cv::format("Match&Motion[%d]", vecKeyList.size()));

		//����һ��������� vecCurMotions,Ŀǰ�ȹ��˵� initedΪfalse��
		int cntValid = filterMotions(vecCurMotions,0);

		// ���������û����, ����֡
		if (cntValid < 1) {
			delete ptrCurFeature;
			vecEnableIdxs[idxImgCur] = 0;
		}
		else {
			/** TODO: �����������ô����Ч��, 1��Ҫ�ж�closure; 2��Ҫ�ۺ�(ȡƽ��������)�˶�״̬ */
			TIME_BEGIN("PoseEstimate");

			// ���ڼ��������ÿ��Motion, Ӧ�ø��˶�
			std::vector<PoseState> vecEstiPoses;
			for (int idx = 0; idx < vecCurMotions.size(); idx++) {
				MotionState& curMotion = vecCurMotions[idx];
				if (curMotion.inited == true) {
					// ��vecPose�Ķ�Ӧλ��,Ӧ��Motion�õ� vecEstiPoses��λ��
					vecEstiPoses.push_back(vecPoses[curMotion.idxImg[0]].move(curMotion));

					//����Ǽ�
					vecMotionLinks[curMotion.idxImg[1]].insert(std::make_pair(curMotion.idxImg[0], curMotion));
				}
			}
			
			/** TODO: ��Ҫ�и������ж���ǰ�Ƿ���ת��. */
			if (std::abs(vecCurMotions[0].degreeT) > 7.0f) {
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
				double pow[] = { 1.0f / 2.0f, 1.0f / 4.0f, 1.0f / 8.0f, 1.0f / 16.0f,1.0f/32,1.0f/64,1.0/128 };
				
				for (int idx = 0; idx < vecEstLen; idx++) {
					curPoseState.pos += vecEstiPoses[idx].pos;// *pow[idx];
					curPoseState.dir += vecEstiPoses[idx].dir;// *pow[idx];
				}
				//curPoseState.pos += vecEstiPoses[vecEstLen - 1].pos * pow[vecEstLen - 1];
				//curPoseState.dir += vecEstiPoses[vecEstLen - 1].dir * pow[vecEstLen - 1];

				curPoseState.pos = curPoseState.pos * (1.0f / vecEstLen);
				curPoseState.dir = curPoseState.dir * (1.0f / vecEstLen);
				curPoseState.dir = curPoseState.dir * (1.0f / cv::norm(curPoseState.dir));
			}

			// ���¶��к͹ؼ�֡
			updateKeyList(ptrCurFeature);
			cntRunOk++;

			vecPoses[idxImgCur] = curPoseState;

			TIME_END("PoseEstimate");

			std::cout << vecPoses[idxImgCur] << std::endl;
			// ����������ǰλ��, �Լ�GroundTruthʵ��·��
			// �ڲ��޸� vecPoses �����ݣ������� curPoseState ��
			cDrawer.drawCanvas(vecPoses[idxImgCur]);
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

int TrackRunner::filterMotions(std::vector<MotionState>& vecMotion, double oldDegreeT) {

	auto iter = vecMotion.begin();
	int len = vecMotion.size();
	int reduce = 0;
	for (; iter != vecMotion.end();) {
		bool isDelete = false;
		if (iter->inited == false) {
			
			isDelete = true;
			reduce++;
		}
		//else {
		//	 if (std::abs(oldDegreeT - iter->degreeT) > 10){
		//		isDelete = true;
		//	}
		//}

		if (isDelete) {
			//eraseִ�к󷵻���һ��������, �����end()�Ļ���++�ͻ�ҵ�
			iter = vecMotion.erase(iter);
		}
		else {
			iter++;
		}
	}
	return len - reduce;
}

void TrackRunner::updateKeyList(FeatureState* ptrFeature) {

	while (deqFrameFeatures.size() >= CFG_iDequeFrameNumber) {

		FeatureState* ptrPop = deqFrameFeatures.front();
		deqFrameFeatures.pop_front();
		delete ptrPop;
	}

	deqFrameFeatures.push_back(ptrFeature);
	
}

void TrackRunner::lm() {
	printf("==== Least Square ====\n");
	auto vecPosesBack = vecPoses;
	std::vector<PoseState> vecDiff(vecMotionLinks.size());

	//std::function<void(MotionState&)> func1 = [&](MotionState& m) {
	//	int j = m.idxImg[0],
	//		i = m.idxImg[1];
	//	PoseState moved = vecPosesBack[j].move(m);
	//	vecDiff[i].pos += 2.0f*(vecPosesBack[i].pos - moved.pos);
	//	vecDiff[i].dir += 2.0f*(vecPosesBack[i].dir - moved.dir);
	//};

	//std::function<void(MotionState&)> func2 = [&](MotionState& m) {

	//};

	for (int i = 0; i < vecMotionLinks.size(); i++) {
		if (vecPosesBack[i].inited == false) break;
		std::cout << vecPosesBack[i] << std::endl;
	}

	for (int idxCur = 0; idxCur < vecMotionLinks.size(); idxCur++) {
		auto& mp = vecMotionLinks[idxCur];
		if (mp.size() == 0&&idxCur !=0) break;
		for (auto& pair : mp) {
			int idxPre = pair.first;
			MotionState& motion = pair.second;

			printf("Motion[%d-%d]\n", idxPre, idxCur);
			std::cout << motion << std::endl;

		}
	}
}

//��Ҫ�ж� �� ��.......���� ���� ����.....�� ���� ��.....��.....��
bool TrackRunner::limitRotationDiff(MotionState& curMotion, double limit) {

	//���ҵ� ǰ�漸�� Pose��ֱ��motion
	int idxCur = curMotion.idxImg[1],
		idxPre1 = curMotion.idxImg[0],
		idxPre2, idxPre3;
	bool isSkipFrames;
	bool isSkipBetweenCurAndPre1;
	bool isSkipBetweenPre1AndPre2;
	bool isSkipBetweenPre2AndPre3;

	if (idxPre1 == 0) return true;
	MotionState &preMotion1 = vecMotionLinks[idxPre1].rbegin()->second;
	idxPre2 = preMotion1.idxImg[0];

	if (idxPre2 == 0) return true;
	MotionState &preMotion2 = vecMotionLinks[idxPre2].rbegin()->second;
	idxPre3 = preMotion2.idxImg[0];


	isSkipBetweenCurAndPre1 = (idxCur - idxPre1) - (vecEnableIdxs[idxCur] - vecEnableIdxs[idxPre1]) > 0;
	isSkipBetweenPre1AndPre2 = (idxPre1 - idxPre2) - (vecEnableIdxs[idxPre1] - vecEnableIdxs[idxPre2]) > 0;
	isSkipBetweenPre2AndPre3 = (idxPre2 - idxPre3) - (vecEnableIdxs[idxPre2] - vecEnableIdxs[idxPre3]) > 0;
	isSkipFrames = isSkipBetweenCurAndPre1 || isSkipBetweenPre1AndPre2 || isSkipBetweenPre2AndPre3;

	//���������֡�����ж�

	printf("���ñ��%d-%d: %d-%d\n", idxCur, idxPre1, vecEnableIdxs[idxCur], vecEnableIdxs[idxPre1]);
	if (vecEnableIdxs[idxCur] - vecEnableIdxs[idxPre1] <= CFG_iDequeFrameNumber && idxCur - idxPre1 > CFG_iDequeFrameNumber) {
		printf("\n");
		return true;
	}


	double preDelta = (preMotion1.degreeT - preMotion2.degreeT) / (idxPre1 - idxPre2),
		curDelta = (curMotion.degreeT - preMotion1.degreeT) / (idxCur - idxPre1);

	if (std::abs(curDelta - preDelta) > limit) {
		printf("[%d-%d] Rotation Error\n",idxPre1, idxCur);
		return false;
	}

	return true;
}

bool TrackRunner::limitScaleDiff(MotionState& curMotion, double& curScale, double limit) {
	int idxCur = curMotion.idxImg[1],
		idxPre1 = curMotion.idxImg[0],
		idxPre2;

	if (idxPre1 == 0) return true;
	if (vecEnableIdxs[idxCur] - vecEnableIdxs[idxPre1] <= CFG_iDequeFrameNumber && idxCur - idxPre1 > CFG_iDequeFrameNumber) return true;
	MotionState &preMotion1 = vecMotionLinks[idxPre1].rbegin()->second;

	idxPre2 = preMotion1.idxImg[0];
	if (idxCur - idxPre1 > 1) return true;
	double curDelta = 1.65f / curScale / (idxCur - idxPre1) - 1.65f / preMotion1.scale / ( idxPre1 -idxPre2);

	if (std::abs(curDelta ) > limit) {
		printf("[%d-%d] Scale Error \n", idxPre1, idxCur);
		double sign = curDelta > 0 ? +1.0f : -1.0f;
		curScale = 1.65f / (idxCur - idxPre1) / (1.65f / preMotion1.scale / (idxPre1 - idxPre2) + sign * limit);
		return true;
	}


	return true;
}