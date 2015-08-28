#include "FrameParser.h"
#include "ScaleEstimator.h"
FrameParser::~FrameParser() {
	
}


void FrameParser::match(double opThreshold) {
	bool _isTimeProfile = true;
	bool _isShowImage = true;
	bool _isLogData = true;

	cv::BruteForceMatcher<cv::L2<float>> BFMatcher;
	std::vector<cv::DMatch> vecBFMatches;
	cv::Mat matBFMathes;

	//����ƥ��
	if (_isTimeProfile) TIME_BEGIN("BF-Matcher");
	BFMatcher.match(matDescriptor[0], matDescriptor[1], vecBFMatches);
	if (_isTimeProfile) TIME_END("BF-Matcher");

	//ƥ�����֮��ŵ� vecPairPoint[2] ��
	for (auto& match : vecBFMatches) {
		vecPairPointIdx[0].push_back(match.queryIdx);
		vecPairPointIdx[1].push_back(match.trainIdx);

		vecPairPoint[0].push_back(vecFeaturePoints[0][match.queryIdx]);
		vecPairPoint[1].push_back(vecFeaturePoints[1][match.trainIdx]);
	}

	if (CFG_bIsLogGlobal)
	if (true || _isLogData)
		printf("BFMatch.size=%d\n", vecBFMatches.size());

	// �������� ƥ�����飬�ŵ�vecFiltedMask��
	validPointsByOpticalFlow(opThreshold);

	//��һ�����ص�� ��ֵ
	if (CFG_bIsLogGlobal)
	printf("poin-pair distance\n");
	std::vector<int> vecTmpIdx[2];
	
	vecFiltedMask.reserve(vecPairPoint[0].size());
	vecFiltedMask.clear();

	//���˵�̫�̵��ƶ�
	for (int i = 0; i < vecPairPoint[0].size(); i++) {
		double dist = cv::norm(vecPairPoint[0][i] - vecPairPoint[1][i]);
		if (dist <7.0f){
			vecFiltedMask[i] = false;
			continue;
		}
		vecFiltedMask[i] = true;
	}

	// ʹ�� vecFiltedMask ���� vecPairPoint
	for (int i = 0; i < vecPairPoint[0].size(); i++) {
		if (true == vecFiltedMask[i]) {
			vecTmpIdx[0].push_back(vecPairPointIdx[0][i]);
			vecTmpIdx[1].push_back(vecPairPointIdx[1][i]);
		}
	}

	vecPairPointIdx[0] = vecTmpIdx[0];
	vecPairPointIdx[1] = vecTmpIdx[1];

	vecPairPoint[0].clear();
	vecPairPoint[1].clear();
	for (int i = 0; i < vecPairPointIdx[0].size(); i++) {
		vecPairPoint[0].push_back(vecFeaturePoints[0][vecPairPointIdx[0][i]]);
		vecPairPoint[1].push_back(vecFeaturePoints[1][vecPairPointIdx[1][i]]);
	}

	//mapPairPoints����map
	for (int i = 0; i < vecPairPoint[0].size(); i++) {
		mapPairPoints.insert(std::make_pair(vecPairPoint[1][i], vecPairPoint[0][i]));
	}
}

void FrameParser::validPointsByOpticalFlow(double threshold) {
	bool _isTimeProfile = true;
	bool _isShowImage = true;
	bool _isLogData = true;

	// ��������ƥ��
	std::vector<cv::Point2f> vecOpticalFound;
	std::vector<uchar>  vecOpticalStatus;
	std::vector<float> vecOpticalErr; 

	if (_isTimeProfile) TIME_BEGIN("optical-flow");
	if (CFG_bIsLogGlobal)
	printf("%d %d %d %d\n", matOrignImage[0].rows,
		matOrignImage[1].rows,
		vecPairPoint[0].size(), vecPairPoint[1].size());

	cv::calcOpticalFlowPyrLK(
		matOrignImage[0], matOrignImage[1], 
		vecPairPoint[0], vecOpticalFound, 
		vecOpticalStatus, vecOpticalErr);
	if (_isTimeProfile) TIME_END("optical-flow");

	// ����opticalStatusɸһ��vecPairPoint, ɸ��֮���ӽ�set< pair<int,int> ,comp>
	// ɸѡ�ظ���
	std::set< std::pair<int, int> > setPair;

	std::vector<double> vecDist;
	for (int idxStatus = 0; idxStatus < vecOpticalStatus.size(); idxStatus++) {
		if (true == vecOpticalStatus[idxStatus]) {
			cv::Point2f
				&p_Pre = vecPairPoint[0][idxStatus],
				&p_BF = vecPairPoint[1][idxStatus],
				&p_OF = vecOpticalFound[idxStatus];
			double dx = abs(p_BF.x - p_OF.x),
				dy = abs(p_BF.y - p_OF.y);
			double dist = dx*dx + dy*dy;
			
			vecDist.push_back(dist);
		}
	}
	//  ���Ƶ�800����ʱ����ֵ
	/*std::sort(vecDist.begin(), vecDist.end());
	if (vecDist.size() > 800) {
		threshold = std::min(threshold, vecDist[300 - 1]);
	}*/

	//������ֵɸ��
	for (int idxStatus = 0; idxStatus < vecOpticalStatus.size(); idxStatus++) {
		if (true == vecOpticalStatus[idxStatus]) {
			cv::Point2f
				&p_Pre = vecPairPoint[0][idxStatus],
				&p_BF = vecPairPoint[1][idxStatus],
				&p_OF = vecOpticalFound[idxStatus];
			double dx = abs(p_BF.x - p_OF.x),
				dy = abs(p_BF.y - p_OF.y);
			if (dx*dx + dy*dy > threshold) {
				vecOpticalStatus[idxStatus] = false;
			}
			else {
				setPair.insert(std::pair<int, int>(
					vecPairPointIdx[0][idxStatus],
					vecPairPointIdx[1][idxStatus]
					));
			}
		}
	}

	for (int idx = 0; idx < 2; idx++) {
		vecPairPoint[idx].clear();
		vecPairPointIdx[idx].clear();
	}

	for (auto& pr : setPair) {
		vecPairPointIdx[0].push_back(pr.first);
		vecPairPointIdx[1].push_back(pr.second);

		vecPairPoint[0].push_back(vecFeaturePoints[0][pr.first]);
		vecPairPoint[1].push_back(vecFeaturePoints[1][pr.second]);
	}

	
}


bool FrameParser::computeMotion(MotionState& motion, int minFundamentMatches) {
	bool _isTimeProfile = true;
	bool _isShowImage = true;
	bool _isLogData = true;
	//�Ƿ��ܹ����������(����ƥ���������minFundamentMatches)
	bool _isUseFundamentalMatrix = vecPairPoint[0].size() >= minFundamentMatches;

	motion.setIdxImg(0, preImgIdx);
	motion.setIdxImg(1, curImgIdx);
	motion.setMapPairPoints(mapPairPoints);

	//Ĭ�Ϸ���ֵ
	bool retStatus = true;

	cv::Mat matFundamental, matFundStatus;

	if (CFG_bIsLogGlobal)
	if (_isLogData)
		printf("Optical Pair[0].size=%d Pair[1].size=%d\n", vecPairPoint[0].size(), vecPairPoint[1].size());

	//������Խ�������� 
	if (_isUseFundamentalMatrix) {
		if (_isTimeProfile) TIME_BEGIN("fundamental matrix");
		//TODO: ���ϣ����ƥ�䲻����Ļ�����Ĭ�� �����򣬲���ת .
		//TODO�����ϣ����ƥ���С�� 15�����ش���������֡ʶ��ֱ����ǰһ֡�ͺ�һ֡���м�⡣
		matFundamental = cv::findFundamentalMat(vecPairPoint[0], vecPairPoint[1], matFundStatus, CV_FM_LMEDS);
		if (_isTimeProfile) TIME_END("fundamental matrix");

		//���ʾ������
		cv::Mat matE(3, 3, CV_64FC1);
		matE = CFG_mCameraParameter.t() * matFundamental* CFG_mCameraParameter;


		if (CFG_bIsLogGlobal)
		if (_isLogData) {
			printf("FundamentalMatrix=");
			std::cout << matFundamental << std::endl;
			printf("EssentialMatrix=");
			std::cout << matE << std::endl;
		}

		//SVD�ֽ�
		cv::SVD svdComputer;
		cv::Mat matU(3, 3, CV_64FC1), matS(3, 3, CV_64FC1), matVT(3, 3, CV_64FC1);

		if (_isTimeProfile) TIME_BEGIN("SVD Decomp");
		svdComputer.compute(matE, matS, matU, matVT, cv::SVD::FULL_UV);
		if (_isTimeProfile) TIME_END("SVD Decomp");

		//��������������ˣ�����Ĭ��ֱ��
		if (cv::sum(matS)[0] < 1.0f) {
			motion.setMatR(Const::mat33_111.clone());
			motion.setMatT(Const::mat31_001.clone());
			_isUseFundamentalMatrix = false;
			retStatus = false;
			motion.setErrType(Const::CErrType::LimitPOINT);
		}
		else {
			//��� matU �� matVT ������ʽС��0����һ��
			if (cv::determinant(matU) < 0)
				matU = -matU;
			if (cv::determinant(matVT) < 0)
				matVT = -matVT;
			if (CFG_bIsLogGlobal)
			if (_isLogData) {
				printf("SVD\n");
				std::cout << matU << std::endl << matS << std::endl << matVT << std::endl;
			}

			cv::Mat matR[4],
				matT[4];
			cv::Mat matW(3, 3, CV_64FC1);
			matW = 0.0f;
			matW.at<double>(0, 1) = -1.0f;
			matW.at<double>(1, 0) = 1.0f;
			matW.at<double>(2, 2) = 1.0f;

			//matR �� matT �����ֿ��ܽ�
			for (int i = 0; i < 4; i++) {
				matR[i] = matU * (i % 2 ? matW.t() : matW) * matVT;
				matT[i] = (i / 2 ? 1.0f : -1.0f) * matU.col(2);

				matT[i] = -matR[i].inv() * matT[i];
				matR[i] = matR[i].inv();

				//Trick: ǿ��Ĭ����ǰ������, ����������ݼ������ ��˵.
				if (matT[i].at<double>(2, 0) < 0.0f)  matT[i] = -matT[i];

				if (CFG_bIsLogGlobal)
				if (false&& _isLogData) {
					printf("i=%d\n R=\n", i);
					std::cout << matR[i] << std::endl;
					printf("T=\n");
					std::cout << matT[i] << std::endl;
				}
			}
			//test all
			cv::Mat Q(4, 4, CV_64FC1);
			cv::Mat u1(4, 1, CV_64FC1), u2(4, 1, CV_64FC1);
			cv::Mat res(1, 1, CV_64FC1);
			double compSEL[4];
		
			for (int i = 0; i < 4; i++) {
				//Trick: ѡ����ת����ת����С���Ǹ�
				cv::Mat tmp = (matR[i] * Const::mat31_100);
				compSEL[i] = tmp.at<double>(0, 0);
				if (CFG_bIsLogGlobal)
				if ( _isLogData )
					printf("valid[%d] = %f\n", i, compSEL[i]);
			}
			double maxValidation = -100.0f; int selectDirectionIdx = 0;
			for (int i = 0; i < 2; i++) {
				if (compSEL[i] > maxValidation) {
					maxValidation = compSEL[i];
					selectDirectionIdx = i;
				}
			}

			//ԭ������˵Ӧ��ʹ�����ǲ�����ѡ��,���ƺ�Ч��������
			//if (compSEL[0] > 0 && compSEL[1] > 0){
			//	// ���ǲ���
			//	ScaleEstimator sEstimator;
			//	motion.matT = matT[0].clone();
			//	int num_inlier = 0;
			//	
			//	for (int i = 0; i < 2; i++) {
			//		motion.matR =  matR[i].clone();
			//		sEstimator.updateMotion(&motion);
			//		int num = sEstimator.triangulate();
			//		if (num > num_inlier) {
			//			selectDirectionIdx = i;
			//			num_inlier = num;
			//		}
			//		if (CFG_bIsLogGlobal)
			//		printf("[%d]=%d\n", i, num);
			//	}		
			//}
			if (CFG_bIsLogGlobal)
			printf("selectDirectionIdx=%d\n", selectDirectionIdx);
			motion.setMatR(matR[selectDirectionIdx].clone());
			motion.setMatT(matT[0].clone());
			
		}
		
	}
	else {
		motion.setMatR(Const::mat33_111.clone());
		motion.setMatT(Const::mat31_001.clone());
		retStatus = false;
		motion.setErrType(Const::CErrType::LimitPOINT);
		//TODO�� ����˷���ֱ�ߣ������������� ��֡��
	}

	///���ͼ��� �Ƕ�
	/*if (_isShowImage) {
		DrawFeaturesFlow(matRotation, matTransform);
		}*/
	motion.setInited( retStatus );
	return retStatus;
}
//
//bool FrameParser::DrawFeaturesFlow(cv::Mat& matRotation, cv::Mat& matTransform) {
//	matOutputImage[0] = matOrignImage[0].clone();
//
//	for (int idx = 0; idx < vecPairPoint[0].size(); idx++) {
//		cv::Point2f
//			&pre = vecPairPoint[0][idx],
//			&next = vecPairPoint[1][idx];
//		cv::circle(matOutputImage[0], pre, 2, cv::Scalar(-1));
//		//cv::putText(matOutputImage[0], cv::format("%d", idx), pre, CV_FONT_NORMAL, 0.4, cv::Scalar(0, 0, 255));
//		cv::line(matOutputImage[0], pre, next, cv::Scalar(255, 0, 0));
//	}
//
//	//������
//	cv::Mat orignDirection(3, 1, CV_64FC1), camera2(3, 1, CV_64FC1);
//	cv::Point2f baseP(1000.0f, 320.0f);
//
//	orignDirection = 0.0f;
//	orignDirection.at<double>(2, 0) = 1.0f;
//
//
//	cv::circle(matOutputImage[0], baseP, 2, cv::Scalar(255, 0, 255), 2);
//
//	for (int i = 0; i < 1; i++) {
//		cv::Point2f _baseP = baseP;
//		_baseP = baseP + 30.0f* cv::Point2f(matTransform.at<double>(0, 0), -matTransform.at<double>(2, 0));
//		cv::line(matOutputImage[0], baseP, _baseP, cv::Scalar(255, 0, 255));
//		cv::circle(matOutputImage[0], _baseP, 2, cv::Scalar(255, 255, 0), 2);
//
//		camera2 = matRotation * orignDirection;
//		cv::line(matOutputImage[0], _baseP, _baseP + 60.0f* cv::Point2f(camera2.at<double>(0, 0), -camera2.at<double>(2, 0)), cv::Scalar((1 - i) * 255, i * 255, (1 - i) * 255));
//	}
//
//	//����ǰ֡��
//	cv::putText(matOutputImage[0], cv::format("[%d - %d]", preImgIdx, curImgIdx), baseP + cv::Point2f(80, 30), CV_FONT_NORMAL, 0.5f, cv::Scalar(255, 0, 255));
//
//	cv::imshow("pair", matOutputImage[0]);
//	cv::waitKey(1);
//	return true;
//
//}
//

FrameParser::FrameParser(FeatureState* prePtr, FeatureState* curPtr) {

	preImgIdx = prePtr->getIdxImg();
	curImgIdx = curPtr->getIdxImg();
	
	std::vector<FeatureState*> vecPtr(2);
	vecPtr[0] = prePtr;
	vecPtr[1] = curPtr;

	for (int idx = 0; idx < 2; idx++) {
		vecKeyPoints[idx] = vecPtr[idx]->getVecKeyPointsConst();
		vecFeaturePoints[idx] = vecPtr[idx]->getVecFeaturePointsConst();
		matDescriptor[idx] = vecPtr[idx]->getMatDescriptorRef();
		matOrignImage[idx] = vecPtr[idx]->getMatImageRef();
	}

	if (CFG_bIsLogGlobal) {
		printf("======== FrameParser ========\n");
		printf("Pre:%d Cur:%d\n", preImgIdx, curImgIdx);
		printf("matImg[0]:%d; matImg[1]:%d\n", matOrignImage[0].rows, matOrignImage[1].rows);
	}

}