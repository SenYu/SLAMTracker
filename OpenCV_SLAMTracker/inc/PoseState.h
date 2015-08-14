#ifndef POSITIONSTATE_H_INCLUDED
#define POSITIONSTATE_H_INCLUDED

#include "stdafx.h"
#include "MotionState.h"
/**
 *	\class PoseState
 *	\brief ��ǰ�˶������¼,�Լ�����ת��(�����֡��.inited����Ϊfalse)
 */
class PoseState
{
public:
	PoseState(int _ImgIdx = -1);
	~PoseState();
public:
	bool inited;
	int idxImg;
	cv::Point3d pointPos;/** \var ��¼��ʱλ�õ� */
	cv::Point3d pointDir;/** \var ��¼��ʱ��ͷ���� */

public:
	/** ���ڸ�����һ���˶�,�����µ�����λ�� */
	PoseState Move(const MotionState& motion);
};

#endif