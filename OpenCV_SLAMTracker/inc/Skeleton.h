#ifndef SKELETON_H_INCLUDED
#define SKELETON_H_INCLUDED

#include "stdafx.h"
#include "PoseState.h"
#include "MotionState.h"
/** 
 *	\class Skeleton
 *	\brief �Ǽ��Ż�ϵͳ��Ŀǰ�ѷ���
 */
class Skeleton
{
public:
	Skeleton();
	~Skeleton();
public:
	std::map< std::pair<int, int>, cv::Mat>  mapT;
	std::vector<cv::Mat> vecX;
	std::vector<cv::Mat> vecE;
public:
	void initData(std::vector<PoseState>& vecPoses, std::vector< std::map<int, MotionState>>& motionLink);
	double calcDiff();
	void merge(double step = 1e-5);
	void fixMatrix();
	double calcError();
};

#endif