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
protected:
	bool inited;
	Const::CErrType errType;
public:
	/** \fn ���ڸ�����һ���˶�,�����µ�����λ�� */
	PoseState move(const MotionState& motion);

	/** ����getter/setter public */
	int idxImg; /** \var ��ǰ֡��� */
	cv::Point3d pos;/** \var ��¼��ʱλ�õ� */
	cv::Point3d dir;/** \var ��¼��ʱ��ͷ���� */
	cv::Mat dir3; /** ��eye(3,3)��ת�����ڵľ�ͷ���� */

	int getErrType() const { return (int)errType; }
	void setErrType(int val);
	void setErrType(Const::CErrType val) { setErrType((int)val); }

	bool getInited() const { return inited; }
	void setInited(bool val) { inited = val; }

	static PoseState calcAverage(std::vector<PoseState>& vecPS);

	/** \fn ostream�����Ԫ����
	*	\brief �����ʽ
	*		"PoseState[%d]"
	*		"Pos:[]"
	*		"Dir:[]"
	*/
	friend std::ostream& operator<<(std::ostream& out, const PoseState& ps);
};

#endif