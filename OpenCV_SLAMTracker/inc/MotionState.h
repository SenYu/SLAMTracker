#ifndef MOTIONSTATE_H_INCLUDED
#define MOTIONSTATE_H_INCLUDED

#include "stdafx.h"

/**
 *	\class MotionState
 *	\brief �˶�״̬���洢��֡��任����, δ��ʼ����.inited����Ϊfalse
 */
class MotionState
{
public:
	MotionState();
	~MotionState();
public:
	
	int idxImg[2]; /** \var ǰ��Ա�����ͼƬ�ı�� */
	bool inited; /** \var �Ƿ��ʼ�� */
	cv::Mat matR, matT; /** \var ��ת��λ�ƾ��� */
	double degreeR, degreeT; /** \var matR��MatT����ת�� */
	double scale; /** �߶�(���������) */

	/** \var ǰ����֡�Ķ�Ӧ��ӳ��
	 *	\brief ƽ��ÿ֡ 500����ԣ�ÿ����8Bytes,һ�� 8K, 4000֡ 32MB.���Խ���
	 */
	std::map<cv::Point2f, cv::Point2f, Utils::PointComp<float>> mapPairPoints;

public:
	/**	\fn ���degreeR��degreeT, δ����(-100)�����㣬����ֱ�ӷ���
	 *	\param ��ѡ"R" �� "T"
	 */
	double getDegree(const std::string& str);
	/** \fn ostream�����Ԫ����
	*	\brief �����ʽ
	*		"MotionState[%d-%d]"
	*		"matR:[]"
	*		"matT:[]"
	*		"scale:%f;1.65f/%f"
	*		"pair:%d"
	*/
	friend std::ostream& operator<<(std::ostream& out, const MotionState& ms);
};

#endif