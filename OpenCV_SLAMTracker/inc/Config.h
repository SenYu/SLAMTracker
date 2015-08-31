#ifndef CONFIG_H_INCLUDED
#define CONFIG_H_INCLUDED

#include "stdafx.h"

/**
 * \brief ȫ�������ļ�
 */

extern std::string CFG_sPathConfigFile; /** \var string:�����ļ���ȡ·�� */
extern std::string CFG_sPathPoseGroutTruth; /** \var string:λ��GroundTruth�ļ���ȡ·�� */
extern std::string CFG_sPathImageLoad; /** \var string:���ݼ�ͼ�����ж�ȡ·�� {idx:%06d} */
extern std::string CFG_sPathFeatureData; /** \var string:��������Ԥ�������ݶ�ȡ·�� {idx:%d} {nfeature:%d} */
extern bool CFG_bIsUseCacheFeature; /** \var bool:�Ƿ�ʹ�ñ���Ԥ���������� */
extern std::string CFG_sModeExecute; /** \var string:����ִ��ģʽ {track/feature} */
extern bool CFG_bIsCacheCurrentFeature; /** \var bool:�Ƿ񻺴浱ǰ�������� */
extern bool CFG_bIsLimitRotationDiff; /** \var bool:�Ƿ�����λ����ת�Ƕ����� */
extern int CFG_iImageLoadBegin; /** \var int:��ȡͼ����ʼ��� */
extern int CFG_iImageLoadEnd; /** \var int:��ȡͼ�������� */
extern int CFG_iMaxFeatures; /** \var int:�������ƥ������,SiftǰK������ */
extern bool CFG_bIsUseGroundTruthDistance; /** \var bool:�Ƿ�ʹ��GroundTruth��λ��ģ�� */
extern bool CFG_bIsNotConsiderAxisY; /** \var bool:�Ƿ����������Ƶ�Y(����)�� */
extern double CFG_dDrawFrameStep; /** \var double:�������ʱ��λ����(meter)�����ص㳤�� */
extern double CFG_dScaleRatioLimitBottom; /** \var double:�߶ȱ任�����½� */
extern double CFG_dScaleRatioLimitTop; /** \var double:�߶ȱ任�����Ͻ� */
extern double CFG_dScaleRatioErrorDefault; /** \var double:�߶ȱ任�쳣��Ĭ��λ��ģ�� */
extern double CFG_dRotationDiffLimit; /** \var double:λ����ת�Ƕ���������ֵ(degree) */
extern double CFG_dScaleInvIncreaseDiffLimit; /** \var double:λ��(�߶ȵ���)�������� */
extern cv::Mat CFG_mCameraParameter; /** \var ����ڲ������� */
extern int CFG_iDequeFrameNumber; /** \var int:��ʷ�����е�֡������ */
extern double CFG_dOpticalFlowThreshold; /** \var double:����������ֵ */
extern bool CFG_bIsLogGlobal; /** \var bool:ȫ��log����/�ر� */
extern int CFG_iPreAverageFilter; /** \var int:ǰ���ֵ�˲���֡�� */
extern std::string CFG_sDataName; /** \var string:���ݼ����� */
#endif