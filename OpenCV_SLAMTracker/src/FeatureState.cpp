#include "FeatureState.h"


FeatureState::FeatureState(int _ImgIdx) :
inited(false)
{
	loadImage(_ImgIdx);
}


FeatureState::~FeatureState()
{
}

int FeatureState::detect(int nFeatures) {
	//չ���ڲ����ݶ���
	return detectExtractFeatures(nFeatures, matImage, vecKeyPoints, vecFeaturePoints, matDescriptor, idxImg);
}

int FeatureState::detectExtractFeatures(
	int nFeatures, cv::Mat& matImage,
	std::vector<cv::KeyPoint>& vecKeyPoints,
	std::vector<cv::Point2f>& vecFeaturePoints,
	cv::Mat & matDescriptor,
	int idxImg) {

	bool _isTimeProfile = true;
	bool _isShowImage = false;
	bool _isLogData = true;
	bool _isUseCacheFeature = CFG_bIsUseCacheFeature;
	bool _isCacheCurrentFeature = CFG_bIsCacheCurrentFeature;

	/**
	 *	������� CFG_bIsUseCacheFeature ,�ͳ��Ե���loadFeature��ȡ����
	 *	�����ȡ�ɹ� return, 
	 *	��ȡʧ�� ���δ��ȡ����,��������
	 */
	if (_isUseCacheFeature) {
		TIME_BEGIN("Feature Read");
		bool ret = loadFeature(idxImg, nFeatures, vecKeyPoints, vecFeaturePoints, matDescriptor);
		TIME_END("Feature Read");
		//��ȡ�ɹ��򷵻��������������ɹ���������
		if (ret == true)
			return vecFeaturePoints.size();
		else
			_isUseCacheFeature = false;
	}

	//Sift���������
	cv::SiftFeatureDetector siftDetector(nFeatures);
	if (_isTimeProfile) TIME_BEGIN(cv::format("image-detect-%d", idxImg));
	siftDetector.detect(matImage, vecKeyPoints);
	if (_isTimeProfile) TIME_END(cv::format("image-detect-%d", idxImg));

	
	//KeyPointת����Point2f
	vecFeaturePoints.clear();

	//������ Harris�ǵ�
	//cv::Mat matGray;
	//cv::cvtColor(matImage, matGray, CV_BGR2GRAY);
	//cv::goodFeaturesToTrack(matGray, vecFeaturePoints, 3000, 0.01, 10);
	//vecFeaturePoints.clear();

	for (auto& kpt : vecKeyPoints)
		vecFeaturePoints.push_back(kpt.pt);
	if (CFG_bIsLogGlobal)
	if (_isLogData)
		printf("detect keypoint.size=%d\n", vecFeaturePoints.size());

	// 128ά����������ȡ
	cv::SiftDescriptorExtractor siftExtractor(nFeatures);
	if (_isTimeProfile) TIME_BEGIN(cv::format("desc-compute-%d", idxImg));
	siftExtractor.compute(matImage, vecKeyPoints, matDescriptor);
	if (_isTimeProfile) TIME_END(cv::format("desc-compute-%d", idxImg));

	/**
	 *	���������Ҫ�󻺴��Ѽ�������, ���� ��ǰ�����ݲ����Ƕ�ȡ�Ի���,����д��
	 */
	if (_isCacheCurrentFeature && _isUseCacheFeature == false) {
		TIME_BEGIN("Feature Write");
		writeFeature(idxImg, nFeatures, vecKeyPoints, vecFeaturePoints, matDescriptor);
		TIME_END("Feature Write");
	}

	return vecFeaturePoints.size();
}

bool FeatureState::loadImage(int _ImgIdx) {
	if (_ImgIdx < 0) return false;
	idxImg = _ImgIdx;
	std::string imgPath = cv::format(CFG_sPathImageLoad.c_str(), idxImg);
	matImage = cv::imread(imgPath);

	// �����ȡͼ��ʧ��
	if (matImage.rows == 0 || matImage.cols == 0) {
		std::string error = "Image Load Error: ";
		throw std::exception( (error + imgPath).c_str() );
	}

	inited = true;
	return inited;
}

std::ostream& operator<<(std::ostream& out, const FeatureState& fs) {

	out << "FeatureState["<<fs.idxImg <<"]" << std::endl;
	out << "Image:"<<fs.matImage.rows << "*" << fs.matImage.cols << std::endl;
	out << "FeaturePoints: " << fs.vecFeaturePoints.size() << std::endl;
	return out;
}


bool FeatureState::writeFeature(int idxImg, int nFeature, std::vector<cv::KeyPoint>& vecKeyPoints, std::vector<cv::Point2f>& vecFeaturePoints, cv::Mat & matDescriptor) {
	cv::FileStorage fs;
	fs.open(cv::format(CFG_sPathFeatureData.c_str(), idxImg, nFeature), cv::FileStorage::WRITE);

	if (fs.isOpened() == false) return false;

	fs << "vecKeyPoints" << vecKeyPoints;
	fs << "vecFeaturePoints" << vecFeaturePoints;
	fs << "matDescriptor" << matDescriptor;

	fs.release();

	return true;
}

bool FeatureState::loadFeature(int idxImg, int nFeature, std::vector<cv::KeyPoint>& vecKeyPoints, std::vector<cv::Point2f>& vecFeaturePoints, cv::Mat & matDescriptor) {
	cv::FileStorage fs;
	fs.open(cv::format(CFG_sPathFeatureData.c_str(), idxImg, nFeature), cv::FileStorage::READ);

	if (fs.isOpened() == false) return false;

	cv::read(fs["vecKeyPoints"], vecKeyPoints);
	fs["vecFeaturePoints"] >> vecFeaturePoints;
	fs["matDescriptor"] >> matDescriptor;

	fs.release();



	return true;
}