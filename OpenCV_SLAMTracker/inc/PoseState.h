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
	int idxImg; /** \var ��ǰ֡��� */
	cv::Point3d pos;/** \var ��¼��ʱλ�õ� */
	cv::Point3d dir;/** \var ��¼��ʱ��ͷ���� */

public:
	/** \fn ���ڸ�����һ���˶�,�����µ�����λ�� */
	PoseState move(const MotionState& motion);
	/** \fn ostream�����Ԫ����
	*	\brief �����ʽ
	*		"PoseState[%d]"
	*		"Pos:[]"
	*		"Dir:[]"
	*/
	friend std::ostream& operator<<(std::ostream& out, const PoseState& ps);
};

#endif