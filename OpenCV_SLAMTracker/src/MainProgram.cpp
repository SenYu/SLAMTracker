#include "stdafx.h"
#include "TrackRunner.h"


int main(int argc, char* argv[]) {

	// ���������в������������ļ��ȴ������
	Utils::loadCommandLine(argc, argv);

	// ����ģʽ����Ϊ ִ�й켣����
	if (CFG_sModeExecute == "track") {
		TrackRunner runner(CFG_iImageLoadBegin, CFG_iImageLoadEnd);

		runner.initFirstFrame();

		while (true) {

			int idxImgCur = runner.runKeyStep();
			if (idxImgCur > CFG_iImageLoadEnd) break;

			runner.showFrameMotion();
			runner.showTrack();
		}

	}
	// ����ģʽ����Ϊ ������������Ԥ����
	else if (CFG_sModeExecute == "feature") {
	
	
	}

	////////////////////////////////////////////////////////////////////////////
	//// ���������ʼ��
	//CanvasDrawer cDrawer(cv::format("./Output/coordOutput_%s.txt", Utils::GetTimeNow().c_str()));
	//cDrawer.UseGroundTruth(CFG_GroutTruthPosePath);

	////////////////////////////////////////////////////////////////////////////
	//// ������������
	//int preImgIdx = CFG_ImageLoadBegin,
	//	curImgIdx = preImgIdx + 1;
	//bool motionStatus = true;
	//double curDegreeT = 0.0f;
	//double transformScale = -1.0f;
	//FeatureState preFeatureState;
	//cv::Mat matImageList[2];
	//matImageList[1] = cv::imread(cv::format(CFG_ImageLoadPath.c_str(), preImgIdx));

	//FrameParser::DetectExtractFeatures(CFG_nFeatures, matImageList[1], preFeatureState, preImgIdx);
	////FrameParser::DetectExtractFeatures(_nFeatures, matImageList[1], preVecKeyPoints, preVecFeaturePoints, preMatDescriptor);

	//// ��֡�ںϱ�������
	//std::vector<MotionState> vecMotionCache( CFG_ImageLoadEnd + 10 );
	//ScaleEstimator vEstimator;

	////////////////////////////////////////////////////////////////////////////
	//// ֡����ѭ��
	//
	//for (; curImgIdx <= CFG_ImageLoadEnd;) {
	//	
	//	TIME_BEGIN("MainLoop");

	//		printf("\n//////////////////////////////////////////////\n");
	//		printf("/////////// image[%d]-[%d] /////////////\n", preImgIdx, curImgIdx);
	//		std::cout << cv::format(CFG_ImageLoadPath.c_str(), curImgIdx) << std::endl;
	//		printf("//////////////////////////////////////////////\n\n");

	//		cv::Mat matR, matT;

	//		if ( motionStatus == true )
	//			matImageList[0] = matImageList[1];
	//		matImageList[1] = cv::imread(cv::format(CFG_ImageLoadPath.c_str(), curImgIdx));
	//					
	//		FrameParser fparser(matImageList, preImgIdx,curImgIdx);

	//	

	//		//��֡���㻺�� ��ֵ1
	//		TIME_BEGIN("PreFeature Copy1");
	//		fparser.SetFeatureState(preFeatureState);
	//		//fparser.vecKeyPoints[0] = preVecKeyPoints;
	//		//fparser.vecFeaturePoints[0] = preVecFeaturePoints;
	//		//fparser.matDescriptor[0] = preMatDescriptor;
	//		TIME_END("PreFeature Copy1");

	//		//�������
	//		TIME_BEGIN("DetectFeature" );
	//			fparser.DetectFeaturePoints(CFG_nFeatures,1);
	//		TIME_END("DetectFeature");

	//		//����ƥ��
	//		TIME_BEGIN("MatchFeature" );
	//			fparser.MatchFeaturePoints();
	//		TIME_END("MatchFeature");

	//		//�˶�����
	//		TIME_BEGIN("ComputeMotion" );
	//			motionStatus = fparser.ComputePointPairMotion(matR, matT, curDegreeT);
	//			//�ж�һ�� vecMotionCache[pre].degreeT �� ��ǰ curDegreeT;
	//			std::function<double(double)> funcDiff = [&](double limit)->double{
	//				return limit + (curImgIdx - preImgIdx-1)*0.1f*limit;
	//			};
	//			if (curImgIdx > CFG_ImageLoadBegin +1)
	//			if ( CFG_IsLimitRotationDiff && std::abs(vecMotionCache[preImgIdx].degreeT - curDegreeT) > funcDiff(CFG_RotationDiffLimit)) {
	//				printf("===========DegreeT Error==========\n");
	//				printf("pre=%f cur=%f\n", vecMotionCache[preImgIdx].degreeT, curDegreeT);

	//				int ret = IDNO;// MessageBox(NULL, TEXT("�ǣ����ܸ�λ�ƣ�����֡"), TEXT("λ�ƽǶ��쳣"), MB_YESNO);
	//				motionStatus = ret == IDYES;
	//			}
	//		TIME_END("ComputeMotion");


	//		if (motionStatus == false) {
	//			//�˶������쳣,�㲻����, pre����,cur++
	//			printf("================ComputeMotion Error===============\n");
	//			preImgIdx;
	//			curImgIdx++;
	//			cv::waitKey(1);
	//			TIME_END("MainLoop");

	//			continue;
	//		}

	//		//////////////////////////////////////////////////////////////////////////
	//		// ������д ScaleEstimator
	//		TIME_BEGIN("Velocity Estimator");
	//			fparser.GetMotionState(vecMotionCache[curImgIdx]);
	//			vecMotionCache[curImgIdx].SetState(matR, matT, cDrawer.gPointPos, cDrawer.gPointDir);
	//			vecMotionCache[curImgIdx].degreeT = curDegreeT;
	//			vEstimator.UpdateMotion(&vecMotionCache[curImgIdx]);
	//			//TODO ���ٶ�����
	//			transformScale = vEstimator.ComputeScaleTransform(transformScale);
	//			//transformScale = 1.0f;// vEstimator.ComputeScale();
	//		TIME_END("Velocity Estimator");


	//		//��������
	//		TIME_BEGIN("DrawAnimate" );
	//			std::cout << matR << std::endl << matT << std::endl;
	//			fparser.DrawFeaturesFlow(matR, matT);
	//			cDrawer.DrawAnimate(matR, matT, preImgIdx, curImgIdx, transformScale);
	//		TIME_END("DrawAnimate");

	//		if (motionStatus == true) {
	//			//�˶���������, pre++,cur++
	//			preImgIdx = curImgIdx;
	//			curImgIdx++;
	//		}

	//		//��֡���㻺�� ��ֵ2
	//		TIME_BEGIN("PreFeature Copy2 Back");
	//			fparser.GetFeatureState(preFeatureState);
	//		TIME_END("PreFeature Copy2 Back");

	//

	//	TIME_END("MainLoop");
	//	//getchar();
	//}
	//

	//cv::waitKey();
	getchar();
	return 0;
}