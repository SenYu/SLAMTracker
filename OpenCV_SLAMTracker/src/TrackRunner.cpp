#include "TrackRunner.h"
#include "CanvasDrawer.h"
#include "FrameParser.h"
#include "ScaleEstimator.h"
#include "Skeleton.h"

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
	cDrawer.setLogPath(cv::format("./Output/coordOutput_%s_%s.txt", Utils::getTimeNow().c_str(), CFG_sDataName.c_str()));
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


	//printf("////////////////////////////////////////////\n");
	printf("//////////// idxImgCur: %06d ////////////\n",idxImgCur);
	//printf("////////////////////////////////////////////\n");

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
				printf("�˶���������ʧ��\n");
				continue;
			}

			// λ����ת��ֵ
			if (CFG_bIsLimitRotationDiff && limitRotationDiff(vecCurMotions[idx],CFG_dRotationDiffLimit) == false) {
				printf("��ת��\n");
				vecCurMotions[idx].inited = false;
				continue;
			}

			// �߶ȹ��㿪��
			ScaleEstimator sEstimator;
			sEstimator.updateMotion(&vecCurMotions[idx]);
			double curScale = sEstimator.computeScaleTransform();

			if (curScale <= 0) {
				printf("�߶�û�����\n");
				curScale = 200;
			}
			// �߶���������
			// �������֡�ˣ���֡�����һ������ֵ����������ֵ֮������������һ����
			// ���������֡����
			if (limitScaleDiff(vecCurMotions[idx], curScale, CFG_dScaleInvIncreaseDiffLimit) == false) {
				vecCurMotions[idx].inited = false;
				continue;
			}
			int idxDelta = vecCurMotions[idx].idxImg[1] - vecCurMotions[idx].idxImg[0];
			if (CFG_iPreAverageFilter > 0) {
				auto& qDist = cDrawer.qDist;
				auto iter =  qDist.rbegin();
				double aver = 0.0f;
				int cnt = 0;
				for (; cnt < CFG_iPreAverageFilter && iter != qDist.rend(); iter++, cnt++) {
					printf("%d %f\n", idxImgCur, *iter);
					aver += *iter;
				}
				aver += 1.65f / curScale / idxDelta;
				aver /= cnt + 1;
				//if (aver < 1.65f / curScale / idxDelta) {
					curScale = 1.65 / aver / idxDelta;
				//}
			}

			/** TODO: ������Ҫһ�����ʵĳ߶����� */
			
			if ((curScale*idxDelta < CFG_dScaleRatioLimitBottom /*/ idxDelta*/ || curScale*idxDelta > CFG_dScaleRatioLimitTop)) {
				
				printf("Scale LIMIT %d-%d:%f %f\n", vecCurMotions[idx].idxImg[0],vecCurMotions[idx].idxImg[1], curScale, 1.65f / curScale);
				if (curScale*idxDelta < CFG_dScaleRatioLimitBottom) {
					curScale = CFG_dScaleRatioLimitBottom / idxDelta;
				}
				else
					curScale = CFG_dScaleRatioLimitTop / idxDelta;

				//if (idxDelta > 0) {
				//	vecCurMotions[idx].inited = true;
				//	if (curScale < CFG_dScaleRatioLimitBottom) {
				//		curScale = CFG_dScaleRatioLimitBottom;
				//	}
				//	else
				//		curScale = CFG_dScaleRatioLimitTop;
				//}
				//else {
				//	motionStatus = false;
				//	vecCurMotions[idx].inited = false;
				//	continue;
				//}
				
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
			if (std::abs(vecCurMotions[0].getDegree("T")) > 7.0f) {
				if (CFG_bIsLogGlobal)
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

			//����Ļ� ��β�ֵ dir3 �Ǹ�����
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
			cv::Mat _rtDir(3, 1, CV_64FC1);
			_rtDir.at<double>(0, 0) = curPoseState.dir.x;
			_rtDir.at<double>(1, 0) = curPoseState.dir.y;
			_rtDir.at<double>(2, 0) = curPoseState.dir.z;

			Utils::getRodriguesRotation(_rtDir, curPoseState.dir3);
			// ���¶��к͹ؼ�֡
			updateKeyList(ptrCurFeature);
			cntRunOk++;

			vecPoses[idxImgCur] = curPoseState;

			TIME_END("PoseEstimate");
			//if (CFG_bIsLogGlobal)
			std::cout << vecPoses[idxImgCur] << std::endl;
			// ����������ǰλ��, �Լ�GroundTruthʵ��·��
			// �ڲ��޸� vecPoses �����ݣ������� curPoseState ��
			cDrawer.drawCanvas(vecPoses[idxImgCur]);

			if (false && idxImgCur == 50) {
				printf("��ʼ����\n");

				Skeleton skl;
				skl.initData(vecPoses, vecMotionLinks);
				double err = skl.calcError();
				double preErr = err;
				printf("i=%d err=%f\n", -1, err);
				if (CFG_bIsLogGlobal)
				std::cout << skl.vecX[0] << std::endl;
				if (CFG_bIsLogGlobal)
				std::cout << skl.vecX[1] << std::endl;
				for (int i = 0; i < 1000; i++) {
					skl.calcDiff();
					skl.merge(1e-3);
					skl.fixMatrix();
					err = skl.calcError();
					
					printf("i=%d err=%f\n", i, err);
					if (CFG_bIsLogGlobal)
					std::cout << skl.vecX[0] << std::endl;
					if (CFG_bIsLogGlobal)
					std::cout << skl.vecX[1] << std::endl;
					if (preErr< err) {
						break;
					}
					preErr = err;
				}
				
				for (int i = idxImgBegin + 1; i < idxImgCur; i++) {
					if ( skl.vecX[i].rows) {
						PoseState t(i);
						t.inited = true;
						t.pos.x = skl.vecX[i].at<double>(0, 0);
						t.pos.y = skl.vecX[i].at<double>(1, 0);
						t.pos.z = skl.vecX[i].at<double>(2, 0);
						if (CFG_bIsLogGlobal)
						std::cout << t.pos << std::endl;
					}

				}
				cv::waitKey();

				printf("��������\n");
			}


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
		if (CFG_bIsLogGlobal)
		std::cout << vecPosesBack[i] << std::endl;
	}

	for (int idxCur = 0; idxCur < vecMotionLinks.size(); idxCur++) {
		auto& mp = vecMotionLinks[idxCur];
		if (mp.size() == 0&&idxCur !=0) break;
		for (auto& pair : mp) {
			int idxPre = pair.first;
			MotionState& motion = pair.second;

			if (CFG_bIsLogGlobal)
			printf("Motion[%d-%d]\n", idxPre, idxCur);
			if (CFG_bIsLogGlobal)
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


	double preDelta = (preMotion1.getDegree("T") - preMotion2.getDegree("T")) / (idxPre1 - idxPre2),
		curDelta = (curMotion.getDegree("T") - preMotion1.getDegree("T")) / (idxCur - idxPre1);

	if (  idxCur - idxPre1  > vecEnableIdxs[idxCur] - vecEnableIdxs[idxPre1] ) {
		printf("֮ǰ��֡ ������ cur(%d:%f %d:%f): %f  pre(%d:%f %d:%f): %f\n", 
			idxCur, curMotion.getDegree("T"), idxPre1, preMotion1.getDegree("T"), curDelta,
			idxPre1, preMotion1.getDegree("T"), idxPre2, preMotion2.getDegree("T"), preDelta);
		return true;
	}


	if (std::abs(curDelta - preDelta) > limit) {
		printf("Rotation Error %d:%f %d:%f \n", idxPre1, preDelta, idxCur, curDelta);
		return false;
	}

	return true;
}

bool TrackRunner::limitScaleDiff(MotionState& curMotion, double& curScale, double limit) {
	int idxCur = curMotion.idxImg[1],
		idxPre1 = curMotion.idxImg[0],
		idxPre2;
	int isSkipBetweenCurAndPre1;
	int isSkipBetweenPre1AndPre2;

	bool isLimitTop = true,
		isLimitBottom = true;

	if (idxPre1 == 0) return true;
	MotionState &preMotion1 = vecMotionLinks[idxPre1].rbegin()->second;

	idxPre2 = preMotion1.idxImg[0];
	
	isSkipBetweenCurAndPre1 = (idxCur - idxPre1) - (vecEnableIdxs[idxCur] - vecEnableIdxs[idxPre1]) ;
	isSkipBetweenPre1AndPre2 = (idxPre1 - idxPre2) - (vecEnableIdxs[idxPre1] - vecEnableIdxs[idxPre2]) ;
	
	printf("Scale Skip[%d-%d-%d] %d,%d\n", idxCur, idxPre1, idxPre2, isSkipBetweenCurAndPre1, isSkipBetweenPre1AndPre2);

	//��ǰ��1����ǰһ����0��
	if (isSkipBetweenCurAndPre1 == 1 && isSkipBetweenPre1AndPre2 == 0) {
		//����ǰ������
		//isLimitTop = true;
		//isLimitBottom = false;
	}
	//��ǰ����2���ˣ�����
	else if (isSkipBetweenCurAndPre1 > 1 && isSkipBetweenPre1AndPre2 == 0) {
		return true;
	}
	//��ǰû����֮ǰ����>0��
	else if ( isSkipBetweenCurAndPre1 == 0 && isSkipBetweenPre1AndPre2 > 0) {
		return true;
	}
	//��ǰ���˼�����֮ǰ���˼���
	else if (isSkipBetweenCurAndPre1 > 0 && isSkipBetweenPre1AndPre2 > 0) {
		return true;
	}

	//double curDelta = (1.65f / curScale / (idxCur - idxPre1)) / (1.65f / preMotion1.scale / (idxPre1 - idxPre2));
	double curDelta = (1.65f / curScale / (idxCur - idxPre1)) - (1.65f / preMotion1.scale / (idxPre1 - idxPre2));

	isLimitTop = (1.65f / curScale / (idxCur - idxPre1)) > 1.0f;
	isLimitBottom = (1.65f / preMotion1.scale / (idxPre1 - idxPre2)) < 0.5f;

	//if (std::abs(curDelta - 1.0f) > limit) {
	if (std::abs(curDelta ) > 1.2f* limit) {
		if (std::abs(curDelta) > limit)
		printf("Scale Error %d-%d:%f %f    %d-%d:%f %f\n", 
			idxPre2, idxPre1, preMotion1.scale*(idxPre1 - idxPre2), 1.65f / preMotion1.scale / (idxPre1 - idxPre2), 
			idxPre1, idxCur, curScale*(idxCur - idxPre1), 1.65f / curScale / (idxCur - idxPre1));
		////double sign = curDelta > 1.0f ? +1.0f : -1.0f;
		double sign = curDelta > 0 ? +1.0f : -1.0f;
		if (sign > 0 && isLimitTop == false) return true;
		if (sign < 0 && isLimitBottom == false) return true;

		////curScale = 1.65f / (idxCur - idxPre1) / (1.65f / preMotion1.scale / (idxPre1 - idxPre2) * sign *(limit+1.0f));
		
		curScale = 1.65f / (idxCur - idxPre1) / (1.65f / preMotion1.scale / (idxPre1 - idxPre2) +sign *(limit));
		return true;
	}


	return true;
}