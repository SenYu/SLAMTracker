#include "FrameParser.h"

void FrameParser::InitData(int idx0, int idx1) {
	isInitedByCloneImage = false;
	isTimeProfile = true;
	isShowImage = true;
	isLogData = true;

	preImgIdx = idx0;
	curImgIdx = idx1;

	/*
	7.188560000000e+02 0.000000000000e+00 6.071928000000e+02
	0.000000000000e+00 7.188560000000e+02 1.852157000000e+02
	0.000000000000e+00 0.000000000000e+00 1.000000000000e+00
	*/
	//double arrCameraParam[] = { 718.856f, 0, 607.1928f, 0, 718.856f, 185.2157f, 0, 0, 1 };
	//CFG_mCameraParameter = cv::Mat(3, 3, CV_64FC1,arrCameraParam).clone();


}



FrameParser::~FrameParser() {
	
}

int FrameParser::detectExtractFeatures(int nFeatures, FeatureState& fState) {
	return FrameParser::detectExtractFeatures(nFeatures, fState.matImage, fState.vecKeyPoints, fState.vecFeaturePoints, fState.matDescriptor, fState.idxImg);
}

int FrameParser::detectExtractFeatures(
	int nFeatures, cv::Mat& matImage,
	std::vector<cv::KeyPoint>& vecKeyPoints,
	std::vector<cv::Point2f>& vecFeaturePoints,
	cv::Mat & matDescriptor,
	int idxImg) {

	bool _isTimeProfile = true;
	bool _isShowImage = false;
	bool _isLogData = true;
	bool _isUseLocalFeature = CFG_bIsUseCacheFeature;
	bool _isCacheCurrentFeature = CFG_bIsCacheCurrentFeature;

	if (_isUseLocalFeature) {
		TIME_BEGIN("Feature Read");
		bool ret = loadFeature(idxImg,nFeatures,vecKeyPoints,vecFeaturePoints,matDescriptor);
		TIME_END("Feature Read");
		//��ȡ���ɹ��Ļ����Զ�����
		if ( ret == true)
			return vecFeaturePoints.size();
	}

	//////////////////////////////////////////////////////////////////////////
	//�ؼ�������
	cv::SiftFeatureDetector siftDetector(nFeatures);
	if (_isTimeProfile) TIME_BEGIN(cv::format("image-detect-%d", idxImg));
	siftDetector.detect(matImage, vecKeyPoints);
	if (_isTimeProfile) TIME_END(cv::format("image-detect-%d", idxImg));

	vecFeaturePoints.clear();

	//////////////////////////////////////////////////////////////////////////
	// KeyPoint ת���� Point2
	for (auto& kpt : vecKeyPoints)
		vecFeaturePoints.push_back(kpt.pt);

	if (_isLogData)
		printf("detect keypoint.size=%d\n", vecFeaturePoints.size());

	//////////////////////////////////////////////////////////////////////////
	// 128ά����������ȡ
	cv::SiftDescriptorExtractor siftExtractor(nFeatures);
	if (_isTimeProfile) TIME_BEGIN(cv::format("desc-compute-%d", idxImg));
	siftExtractor.compute(matImage, vecKeyPoints, matDescriptor);
	if (_isTimeProfile) TIME_END(cv::format("desc-compute-%d", idxImg));

	if (_isCacheCurrentFeature) {
		TIME_BEGIN("Feature Write");
		writeFeature(idxImg, nFeatures, vecKeyPoints, vecFeaturePoints, matDescriptor);
		TIME_END("Feature Write");
	}

	return vecFeaturePoints.size();
}

//void FrameParser::DetectFeaturePoints(int nFeatures, int whichImage) {
//
//	//TODO����Ҫ����  д���������ݺͶ�ȡ�������ݣ��������㻷��
//	//TODO��������ÿ������һ��ͼ����һ��ͼ�����ֱ�ӻ�ȡ��֮��Ĳ�����
//	bool _isTimeProfile = true && isTimeProfile;
//	bool _isShowImage = false && isShowImage;
//	bool _isLogData = true && isLogData;
//
//	if (-1 == whichImage) {
//		for (int idxImg = 0; idxImg < 2; idxImg++)
//			FrameParser::DetectFeaturePoints(nFeatures, idxImg);
//		return;
//	}
//
//	DetectExtractFeatures(
//		nFeatures, matOrignImage[whichImage],
//		vecKeyPoints[whichImage],
//		vecFeaturePoints[whichImage],
//		matDescriptor[whichImage],
//		whichImage == 0 ? preImgIdx : curImgIdx);
//
//}

void FrameParser::matchFeaturePoints() {
	bool _isTimeProfile = true && isTimeProfile;
	bool _isShowImage = true && isShowImage;
	bool _isLogData = true && isLogData;

	cv::BruteForceMatcher<cv::L2<float>> BFMatcher;
	std::vector<cv::DMatch> vecBFMatches;
	cv::Mat matBFMathes;

	//////////////////////////////////////////////////////////////////////////
	//����ƥ����
	if (_isTimeProfile) TIME_BEGIN("BF-Matcher");
	BFMatcher.match(matDescriptor[0], matDescriptor[1], vecBFMatches);
	if (_isTimeProfile) TIME_END("BF-Matcher");

	//////////////////////////////////////////////////////////////////////////
	//ƥ�����֮��ŵ� vecPairPoint[2] ��
	for (auto& match : vecBFMatches) {
		vecPairPointIdx[0].push_back(match.queryIdx);
		vecPairPointIdx[1].push_back(match.trainIdx);

		vecPairPoint[0].push_back(vecFeaturePoints[0][match.queryIdx]);
		vecPairPoint[1].push_back(vecFeaturePoints[1][match.trainIdx]);
	}

	if (true || _isLogData)
		printf("BFMatch.size=%d\n", vecBFMatches.size());

	//////////////////////////////////////////////////////////////////////////
	// �������� ƥ�����飬�ŵ�hahamask��
	validPointsByOpticalFlow(2.0f);

	//��һ�����ص�� ��ֵ
	printf("poin-pair distance\n");
	std::vector<int> vecTmpIdx[2];
	
	vecFiltedMask.reserve(vecPairPoint[0].size());
	vecFiltedMask.clear();

//#pragma omp parallel for
	for (int i = 0; i < vecPairPoint[0].size(); i++) {
		double dist = cv::norm(vecPairPoint[0][i] - vecPairPoint[1][i]);
		if (dist <7.0f){
			vecFiltedMask[i] = false;
			continue;
		}
		vecFiltedMask[i] = true;
	}

	//////////////////////////////////////////////////////////////////////////
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
	bool _isTimeProfile = true && isTimeProfile;
	bool _isShowImage = true && isShowImage;
	bool _isLogData = true && isLogData;

	//////////////////////////////////////////////////////////////////////////
	// ��������ƥ��
	std::vector<cv::Point2f> vecOpticalFound;
	std::vector<uchar>  vecOpticalStatus;
	std::vector<float> vecOpticalErr; 

	if (_isTimeProfile) TIME_BEGIN("optical-flow");
	printf("%d %d %d %d\n", matOrignImage[0].rows,
		matOrignImage[1].rows,
		vecPairPoint[0].size(), vecPairPoint[1].size());

	cv::calcOpticalFlowPyrLK(
		matOrignImage[0], matOrignImage[1], 
		vecPairPoint[0], vecOpticalFound, 
		vecOpticalStatus, vecOpticalErr);
	if (_isTimeProfile) TIME_END("optical-flow");

	/* ����opticalStatusɸһ��vecPairPoint, ɸ��֮���ӽ�set< pair<int,int> ,comp> */
	//////////////////////////////////////////////////////////////////////////
	// ɸѡ�ظ���
	std::set< std::pair<int, int> > setPair;

//#pragma omp parallel for
	for (int idxStatus = 0; idxStatus < vecOpticalStatus.size(); idxStatus++) {
		if (true == vecOpticalStatus[idxStatus]) {
			cv::Point2f
				&p_Pre = vecPairPoint[0][idxStatus],
				&p_BF = vecPairPoint[1][idxStatus],
				&p_OF = vecOpticalFound[idxStatus];
			float dx = abs(p_BF.x - p_OF.x),
				  dy = abs( p_BF.y - p_OF.y),
				  de = vecOpticalErr[idxStatus];
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


////
////int ValidCommonPerspective(std::vector<cv::Point2f> vecPairPoint[2],cv::Mat matFundStatus,cv::Mat matR, cv::Mat matT) {
////	double arrCameraParam[] = { 718.856f, 0, 607.1928f, 0, 718.856f, 185.2157f, 0, 0, 1 };
////	cv::Mat matCameraParam(3, 3, CV_64FC1, arrCameraParam);
////
////	printf("////////////////matR,matT//////\n");
////	std::cout << matR << std::endl;
////	std::cout << matT << std::endl;
////	double Fu, Fv, Cu, Cv;
////	Fu = matCameraParam.at<double>(0, 0);
////	Fv = matCameraParam.at<double>(1, 1);
////	Cu = matCameraParam.at<double>(0, 2);
////	Cv = matCameraParam.at<double>(1, 2);
////
////	std::vector<cv::Mat> vecD3Point[2];
////	for (int j = 0; j < 2; j++) {
////		printf("/////////////////j=%d\n", j);
////		for (int i = 0; i < vecPairPoint[0].size(); i++) {
////			if (matFundStatus.at<uchar>(i, 0) == true) {
////				cv::Mat tmp(3, 1, CV_64FC1);
////				tmp.at<double>(0, 0) = (vecPairPoint[j][i].x - Cu) / Fu; //(u0-Cu)/Fu *** Z
////				tmp.at<double>(1, 0) = (vecPairPoint[j][i].y - Cv) / Fv; //(v0-Cv) / Fv *** Z
////				tmp.at<double>(2, 0) = 1.0f;
////
////				vecD3Point[j].push_back(tmp.clone());
////				if ( j==1 && i<1 )
////				std::cout << tmp << std::endl;
////			}
////		}
////	}
////
////	printf("///////////////R*P+T\n");
////	for (int i = 0; i < vecD3Point[1].size(); i++) {
////		vecD3Point[1][i] = matR * vecD3Point[1][i] + matT;
////		if (  i<1)
////		std::cout << vecD3Point[1][i] << std::endl;
////	}
////
////	//���� �洢���� ֱ�ߵķ�������
////	//�������� (0,0,0) �� (matT)
////
////	//��ʼ���� ����ֱ�߽����λ��
////	printf("////////////////intersection\n");
////	int ret = 0;
////	for (int i = 0; i < vecD3Point[0].size(); i++) {
////		cv::Mat inter = CalcLineIntersection(vecD3Point[0][i], vecD3Point[1][i], matT);
////		ret += inter.at<double>(2, 0) > 0;
////		if (i<1)
////		std::cout << inter << std::endl;
////	}
////	return ret;
////}

//cv::Mat matCanvasFundaTest(400, 400, CV_8UC3);
//
//void draw3D(cv::Point2d base, cv::Mat matR, cv::Mat matT) {
//	cv::Point2d oldBase = base;
//	double arrBase[] = { 0, 0, 1 };
//	cv::Mat baseMat(3, 1, CV_64FC1, arrBase);
//
//	cv::circle(matCanvasFundaTest, base, 2, cv::Scalar(0, 0, 0));
//	cv::line(matCanvasFundaTest, base, base + 40.0f*cv::Point2d(baseMat.at<double>(0, 0), -baseMat.at<double>(2, 0)), cv::Scalar(0, 0, 255), 1.5);
//
//	base = base + 40.0f*cv::Point2d(matT.at<double>(0, 0), -matT.at<double>(2, 0));
//	cv::line(matCanvasFundaTest, oldBase, base, cv::Scalar(255, 0, 255), 2);
//	cv::circle(matCanvasFundaTest, base, 2, cv::Scalar(255, 0, 0));
//	baseMat = matR * baseMat;
//	cv::line(matCanvasFundaTest, base, base + 40.0f* cv::Point2d(baseMat.at<double>(0, 0), -baseMat.at<double>(2, 0)), cv::Scalar(0, 255, 0), 1.5);
//
//}
//
//void ShowFundamentDirection(cv::Mat matE) {
//
//	cv::SVD svd;
//	cv::Mat matS, matU, matVT;
//	svd.compute(matE, matS, matU, matVT, cv::SVD::FULL_UV);
//	if (cv::determinant(matU) < 0)
//		matU = -matU;
//	if (cv::determinant(matVT) < 0)
//		matVT = -matVT;
//	printf("U,S,VT\n");
//	std::cout << matU << std::endl;
//	std::cout << matS << std::endl;
//	std::cout << matVT << std::endl;
//
//	cv::Mat matR[4], matT[4];
//	cv::Mat matW(3, 3, CV_64FC1);
//	matW = 0.0f;
//	matW.at<double>(0, 1) = -1.0f;
//	matW.at<double>(1, 0) = 1.0f;
//	matW.at<double>(2, 2) = 1.0f;
//	for (int i = 0; i < 4; i++) {
//		matR[i] = matU * (i % 2 ? matW.t() : matW) * matVT;
//		matT[i] = (i / 2 ? 1.0f : -1.0f) * matU.col(2);
//
//		matT[i] = -matR[i].inv() * matT[i];
//		matR[i] = matR[i].inv();
//
//		printf("i=%d\n R=\n", i);
//		std::cout << matR[i] << std::endl;
//		printf("T=\n");
//		std::cout << matT[i] << std::endl;
//	}
//
//	matCanvasFundaTest = cv::Scalar(255, 255, 255);
//
//
//	cv::Point2d canvasBase(100, 100);
//	for (int i = 0; i < 4; i++) {
//		cv::Point2d offset((i % 2) * 100, (i / 2) * 100);
//		draw3D(canvasBase + offset, matR[i], matT[i]);
//	}
//	//draw3D(canvasBase + cv::Point2d(200, 200), matRotationDef, matTransformDef);
//	cv::imshow(cv::format("funda"), matCanvasFundaTest);
//}
//
bool FrameParser::computeMotion(MotionState& motion, int minFundamentMatches) {
	bool _isTimeProfile = true && isTimeProfile;
	bool _isShowImage = true && isShowImage;
	bool _isLogData = true|| isLogData;
	bool _isUseFundamentalMatrix = vecPairPoint[0].size() >= minFundamentMatches;


	bool retStatus = true;

	cv::Mat matFundamental, matFundStatus;

	if (_isLogData)
		printf("Optical Pair[0].size=%d Pair[1].size=%d\n", vecPairPoint[0].size(), vecPairPoint[1].size());

	if (_isUseFundamentalMatrix) {
		if (_isTimeProfile) TIME_BEGIN("fundamental matrix");
		//TODO: ���ϣ����ƥ�䲻����Ļ�����Ĭ�� �����򣬲���ת .
		//TODO�����ϣ����ƥ�䲻���㣬���ش���������֡ʶ��ֱ����ǰһ֡�ͺ�һ֡���м�⡣
		//TODO�����ƥ���С�� 15 ��ʹ�� 8POINT������ LMEDS���
		matFundamental = cv::findFundamentalMat(vecPairPoint[0], vecPairPoint[1], matFundStatus, CV_FM_LMEDS);
		if (_isTimeProfile) TIME_END("fundamental matrix");

		cv::Mat matE(3, 3, CV_64FC1);
		matE = CFG_mCameraParameter.t() * matFundamental* CFG_mCameraParameter;

		//Extra Show
		//ShowFundamentDirection(matE.clone());

		if (_isLogData) {
			printf("FundamentalMatrix=");
			std::cout << matFundamental << std::endl;
			printf("EssentialMatrix=");
			std::cout << matE << std::endl;
		}

		//Compute SVD singular vector decomp
		cv::SVD svdComputer;
		cv::Mat matU(3, 3, CV_64FC1), matS(3, 3, CV_64FC1), matVT(3, 3, CV_64FC1);

		if (_isTimeProfile) TIME_BEGIN("SVD Decomp");
		svdComputer.compute(matE, matS, matU, matVT, cv::SVD::FULL_UV);
		if (_isTimeProfile) TIME_END("SVD Decomp");

		//��������������ˣ�����Ĭ��ֱ��
		
		if (cv::sum(matS)[0] < 1.0f) {
			motion.matR = Const::mat33_111.clone();
			motion.matT = Const::mat31_001.clone();
			_isUseFundamentalMatrix = false;
			retStatus = false;
		}
		else {
			if (cv::determinant(matU) < 0)
				matU = -matU;
			if (cv::determinant(matVT) < 0)
				matVT = -matVT;

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

//#pragma omp parallel for
			for (int i = 0; i < 4; i++) {
				matR[i] = matU * (i % 2 ? matW.t() : matW) * matVT;
				matT[i] = (i / 2 ? 1.0f : -1.0f) * matU.col(2);

				matT[i] = -matR[i].inv() * matT[i];
				matR[i] = matR[i].inv();

				if (matT[i].at<double>(2, 0) < 0.0f)  matT[i] = -matT[i];

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
				cv::Mat tmp = (matR[i] * Const::mat31_100);
				compSEL[i] = tmp.at<double>(0, 0);
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

			printf("selectDirectionIdx=%d\n", selectDirectionIdx);
			motion.matR = matR[selectDirectionIdx].clone();
			motion.matT = matT[0].clone();

		}
		
	}
	else {
		motion.matR = Const::mat33_111.clone();
		motion.matT = Const::mat31_001.clone();
		retStatus = false;
		//TODO�� ����˷���ֱ�ߣ������������� ��֡��
	}

	//�ж�һ�� ��� matT�������Ƕ�
	if(true){
		motion.degreeT = Utils::getRodriguesRotation(motion.matT, cv::Mat());
	}

	///���ͼ��� �Ƕ�
	/*if (_isShowImage) {
		DrawFeaturesFlow(matRotation, matTransform);
		}*/
	motion.inited = retStatus;
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

bool FrameParser::writeFeature(int idxImg, int nFeature, std::vector<cv::KeyPoint>& vecKeyPoints, std::vector<cv::Point2f>& vecFeaturePoints, cv::Mat & matDescriptor) {
	cv::FileStorage fs;
	fs.open(cv::format(CFG_sPathFeatureData.c_str(), idxImg, nFeature), cv::FileStorage::WRITE);

	if (fs.isOpened() == false) return false;

	fs << "vecKeyPoints" << vecKeyPoints;
	fs << "vecFeaturePoints" << vecFeaturePoints;
	fs << "matDescriptor" << matDescriptor;

	fs.release();

	return true;
}

bool FrameParser::loadFeature(int idxImg, int nFeature, std::vector<cv::KeyPoint>& vecKeyPoints, std::vector<cv::Point2f>& vecFeaturePoints, cv::Mat & matDescriptor) {
	cv::FileStorage fs;
	fs.open(cv::format(CFG_sPathFeatureData.c_str(), idxImg, nFeature), cv::FileStorage::READ);

	if ( fs.isOpened() == false ) return false;
	
	cv::read(fs["vecKeyPoints"], vecKeyPoints);
	fs["vecFeaturePoints"] >> vecFeaturePoints;
	fs["matDescriptor"] >> matDescriptor;

	fs.release();



	return true;
}

//void FrameParser::SetFeatureState(const FeatureState& fState, int idx) {
//	//fState -> this
//	fState.GetState(vecKeyPoints[idx], vecFeaturePoints[idx], matDescriptor[idx]);
//	return;
//}
//
//void FrameParser::GetFeatureState(FeatureState& fState, int idx) {
//	//this -> fState
//	fState.SetState(vecKeyPoints[idx], vecFeaturePoints[idx], matDescriptor[idx]);
//	return;
//}
//
//
//void FrameParser::GetMotionState(MotionState& mState) {
//	mState.idxImg[0] = preImgIdx;
//	mState.idxImg[1] = curImgIdx;
//	mState.mapPairPoints = mapPairPoints;
//	return;
//}

FrameParser::FrameParser(FeatureState* prePtr, FeatureState* curPtr) {
	isTimeProfile = true;
	isShowImage = true;
	isLogData = true;

	preImgIdx = prePtr->idxImg;
	curImgIdx = curPtr->idxImg;
	
	std::vector<FeatureState*> vecPtr(2);
	vecPtr[0] = prePtr;
	vecPtr[1] = curPtr;

	for (int idx = 0; idx < 2; idx++) {
		vecKeyPoints[idx] = vecPtr[idx]->vecKeyPoints;
		vecFeaturePoints[idx] = vecPtr[idx]->vecFeaturePoints;
		matDescriptor[idx] = vecPtr[idx]->matDescriptor;
		matOrignImage[idx] = vecPtr[idx]->matImage;
	}

	printf("======== FrameParser ========\n");
	printf("Pre:%d Cur:%d\n", preImgIdx, curImgIdx);
	printf("matImg[0]:%d; matImg[1]:%d\n", matOrignImage[0].rows, matOrignImage[1].rows);

}