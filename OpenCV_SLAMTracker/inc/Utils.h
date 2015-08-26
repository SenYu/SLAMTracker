#ifndef UTILS_H_INCLUDED
#define UTILS_H_INCLUDED

#include "stdafx.h"
#include "Constant.h"

/** \var ��������ʱ��ͳ��(ms),���̰߳�ȫ */
extern std::map<std::string, double> mapTBegin; 

/** \def ʱ��ͳ�ƿ�ʼ���� */
#define TIME_BEGIN(cstr) {\
	std::string STR = std::string(cstr);\
	if (CFG_bIsLogGlobal)\
	std::cout << "-----------Time_Begin[" << STR << "]:" << std::endl;\
	if (mapTBegin.find(STR) == mapTBegin.end()) mapTBegin.insert(make_pair(STR, clock()));\
	else mapTBegin.find(STR)->second = clock();\
}

/** \def ʱ��ͳ�ƽ������� */
#define TIME_END(cstr) {\
	std::string STR = std::string(cstr);\
	if (mapTBegin.find(STR) == mapTBegin.end())\
	if (CFG_bIsLogGlobal)\
	std::cout << "-----------Time_End[" << STR << "] ERROR" << std::endl;\
	else \
	if (CFG_bIsLogGlobal)\
	std::cout << "-----------Time_End[" << STR << "]:(" << clock() - mapTBegin.find(STR)->second << ")ms" << std::endl; \
}

/**
 *	\namespace ͨ�ù�����
 */
namespace Utils {
	/** \fn ��� \var rtDir(3,1,CV_F64C1) ����� [0,0,1]' �������ת���� \var matRotatoin */
	double getRodriguesRotation(cv::Mat rtDir, cv::Mat& matRotation, cv::Mat orignDir = Const::mat31_001,double ratio = 1.0f);

	/** \fn ���Command������ѡ�� */
	std::string getCmdOption(char ** begin, char ** end, const std::string & option);

	/** \fn �ж� \var option ��Command������ѡ���Ƿ���� */
	bool isCmdOptionExists(char** begin, char** end, const std::string& option);

	/** \fn ���س��������в��� */
	bool loadCommandLine(int argc, char* argv[]);

	/** \fn ���� \var format ���������ǰʱ�� */
	std::string getTimeNow(std::string format = "%Y_%m_%d_%H_%M_%S");

	/** \fn cv::Mat(3,1,CV_F64C1) to cv::Point3d */
	cv::Point3d transform(cv::Mat& mat31);

	template<class _Type>
	class PointComp {
	public:
		bool operator()(const cv::Point_<_Type> &a, const cv::Point_<_Type> &b) const{
			return a.x != b.x ? (a.x < b.x) : (a.y < b.y);
		}

		bool operator()(const cv::Point3_<_Type> &a, const cv::Point3_<_Type> &b) const{
			return a.x != b.x ? (a.x < b.x) : (a.y != b.y ? (a.y < b.y) : (a.z < b.z););
		}
	};

	/** \fn ���ó�ʼ�� */
	template<class _Type>
	_Type configDefault(_Type defaultVal, cv::FileNode& fn) {
		if (fn.empty() == true)
			return defaultVal;
		else {
			_Type ret;
			fn >> ret;
			return ret;
		}
	}
};

#endif // UTILS_H_INCLUDED