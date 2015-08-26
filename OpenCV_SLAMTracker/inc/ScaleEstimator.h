#ifndef VELOCITYESTIMATOR_H_INCLUDED
#define VELOCITYESTIMATOR_H_INCLUDED

#include "stdafx.h"
#include "MotionState.h"
/**
*	\class ScaleEstimator
*	\brief ���ڵ�ƽ����ĳ߶ȹ���
*/
class ScaleEstimator
{
public:
	ScaleEstimator();
	~ScaleEstimator();

private:
	std::vector<MotionState*> ptrMotion; /** \var �˶�״̬����[2]��ָ������ */
	cv::Mat matPointUVW[2]; /** \var ƥ���� */
	cv::Mat matDirXYZ[2]; /** \var ƥ���Ե����߷��� */
	cv::Mat matIntersection; /** \var �����õ��� ��ά�ռ佻�� */
public:
	/**
	 *	\fn ʹ��motion���¹����� ��Ҫ������
	 */
	bool updateMotion(MotionState* ptrCurMotion);

	/**
	 *	\fn ����ƥ�����г߶�����
	 */
	double computeScaleTransform();

	/**
	 *	\fn ���ǲ���
	 *	\brief �Ѽ��ϵ����ά�ռ��У��Ƚ���������ͷ��̬��ͬʱ����������
	 *	\return ��ͬʱ�����ĵ���
	 */
	int triangulate();
private:
	/**
	 *	\fn ������֪��ά��,�����ƽ��߶�
	 *	\brief Ŀǰֻ�ұȽϴ��yֵ
	 */
	double calcScaleRatio(int flag = 0);

	/**
	 *	\fn ת��ƥ����(map)����Ҫ�����ݸ�ʽ(cv::Mat),��������
	 */
	int getPairPoints2();

	/**
	 *	\fn ���ݸ���������ֱ�ߺ�ֱ����һ��,������ֱ�������Ľ���
	 *	\param d1 ֱ��1����Ĭ��λ��[0,0,0]^T
	 *	\param d2 ֱ��2����
	 *	\param p2 ֱ��2λ��
	 *	\return ip1 ���ؽ���
	 */
	double calcLineIntersection(cv::Mat d1, cv::Mat d2, cv::Mat p2, cv::Mat& ip1);

	/**
	*	\fn �Ѷ�ά���ת������άֱ�߷���
	*/
	cv::Mat transformIn2Coord(int pntNum, int preIdx = 0, int curIdx = 1);
};

#endif
