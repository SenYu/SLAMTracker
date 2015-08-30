#ifndef MOTIONSTATE_H_INCLUDED
#define MOTIONSTATE_H_INCLUDED

#include "stdafx.h"

/**
 *	\class MotionState
 *	\brief �˶�״̬���洢��֡��任����
 */
class MotionState
{
public:
	MotionState();
	~MotionState();
protected:
	

	int idxImg[2]; /** \var ǰ��Ա�����ͼƬ�ı�� */
	bool inited; /** \var �Ƿ��ʼ�� */
	cv::Mat matR, matT; /** \var ��ת��λ�ƾ��� */

	double scale; /** �߶�(���������) */

	/** \var ǰ����֡�Ķ�Ӧ��ӳ��
	 *	\brief ƽ��ÿ֡ 500����ԣ�ÿ����8Bytes,һ�� 8K, 4000֡ 32MB.���Խ���
	 */
	std::map<cv::Point2f, cv::Point2f, Utils::PointComp<float>> mapPairPoints;
	
public:
	

	/** Getter/Setter���� */
	Const::Error errType;

	int getIdxImg(int idx) const { return idxImg[idx]; }
	int& setIdxImg(int idx, int val) { return idxImg[idx] = val; }

	bool getInited() const { return inited; }
	bool setInited(bool val) { return inited = val; }

	std::map<cv::Point2f, cv::Point2f, Utils::PointComp<float>> getMapPairPointsConst() const { return mapPairPoints; }
	std::map<cv::Point2f, cv::Point2f, Utils::PointComp<float>>& getMapPairPointsRef() { return mapPairPoints; }
	int setMapPairPoints(std::map<cv::Point2f, cv::Point2f, Utils::PointComp<float>> val) { mapPairPoints = val; return mapPairPoints.size(); }

	cv::Mat getMatRConst() const { return matR; }
	cv::Mat& getMatRRef() { return matR; }
	void setMatR(cv::Mat& val) { matR = val; }

	cv::Mat getMatTConst() const { return matT; }
	cv::Mat& getMatTRef() { return matT; }
	void setMatT(cv::Mat& val) { matT = val; }

	double getScale() const { return scale; }
	void setScale(double val, bool isMulti = false) { scale = val; if (isMulti) scale = scale / (idxImg[1]-idxImg[0]); }

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