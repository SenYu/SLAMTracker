#include "PoseState.h"
#include "MotionState.h"

PoseState::PoseState(int _ImgIdx) :
inited(false),
idxImg(_ImgIdx)
{
}


PoseState::~PoseState()
{
}

PoseState PoseState::move(const MotionState& motion) {

	//У��motion�뵱ǰPose�Ƿ�ƥ��
	if (idxImg != motion.idxImg[0]) {
		throw std::exception(cv::format("��ǰPose[%d]��Motion[%d-%d]��ƥ��",idxImg,motion.idxImg[0],motion.idxImg[1]).c_str());
	}

	PoseState retPose(motion.idxImg[1]);
	
	//cv::Point3תcv::Mat(3,1)
	double arrLocalPos[] = { pos.x, pos.y, pos.z };
	double arrLocalDir[] = { dir.x, dir.y, dir.z };
	cv::Mat matLocalPos(3, 1, CV_64FC1, arrLocalPos),
			matLocalDir(3, 1, CV_64FC1, arrLocalDir);

	if (CFG_bIsLogGlobal)
	std::cout << matLocalDir << std::endl;
	if (CFG_bIsLogGlobal)
	std::cout << matLocalPos << std::endl;

	cv::Mat matTmpRotate;
	Utils::getRodriguesRotation(matLocalDir, matTmpRotate);

	//matLocalPos = matLocalPos + matTmpRotate * motion.matT * 1.65 / motion.scale;

	if (CFG_bIsNotConsiderAxisY) {
		cv::Mat tT = motion.matT.clone();
		tT.at<double>(1, 0) = 0.0f;
		tT *= 1.0f / cv::norm(tT);
		matLocalPos = matLocalPos + dir3 * tT *  1.65f / motion.scale;
	}
	else {
		matLocalPos = matLocalPos + dir3 *motion.matT *  1.65f / motion.scale;
	}
	


	//������Y��
	if (CFG_bIsNotConsiderAxisY) {
		cv::Mat matRC = motion.matR * Const::mat31_001;

		matRC.at<double>(1, 0) = 0.0f;
		matRC = matRC / cv::norm(matRC);

		Utils::getRodriguesRotation(matRC, matRC);
		
		matLocalDir = matRC * matLocalDir;
		matLocalDir.at<double>(1, 0) = 0.0f;
		matLocalDir = matLocalDir / cv::norm(matLocalDir);

		dir3 = dir3 * matRC;
		matRC = dir3 * Const::mat31_001;
		matRC.at<double>(1, 0) = 0.0f;
		Utils::getRodriguesRotation(matRC, matRC);
		dir3 = matRC.clone();

	}
	//����Y��
	else {
		matLocalDir = motion.matR * matLocalDir;
		dir3 = dir3 * motion.matR ;
	}
	if (CFG_bIsLogGlobal)
	std::cout << matLocalDir << std::endl;
	if (CFG_bIsLogGlobal)
	std::cout << matLocalPos << std::endl;

	retPose.idxImg = motion.idxImg[1];
	retPose.dir = cv::Point3d((cv::Vec<double, 3>)matLocalDir);
	retPose.pos = cv::Point3d((cv::Vec<double, 3>)matLocalPos);
	//retPose.dir3 = motion.matR * matTmpRotate;
	retPose.dir3 = dir3.clone();
	retPose.inited = inited;

	return retPose;
}

std::ostream& operator<<(std::ostream& out, const PoseState& ps) {
	out << "PoseState[" << ps.idxImg << "]" << std::endl;
	out << "Pos: " << ps.pos << std::endl;
	out << "Dir: " << ps.dir << std::endl;
	return out;
}