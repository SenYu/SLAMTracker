#include "stdafx.h"
#include "TrackRunner.h"


int main(int argc, char* argv[]) {

	// ���������в������������ļ��ȴ������
	Utils::loadCommandLine(argc, argv);

	// ����ģʽ����Ϊ ִ�й켣����
	if (CFG_sModeExecute == "track") {
		TrackRunner runner(CFG_iImageLoadBegin, CFG_iImageLoadEnd);

		runner.initFirstFrame();
		int cnt = 0;
		while (runner.hasNext()) {

			int idxImgCur = runner.runKeyStep();
			
			cnt++;
		}
		runner.lm();
		cv::waitKey();

	}
	// ����ģʽ����Ϊ ������������Ԥ����
	else if (CFG_sModeExecute == "feature") {
#pragma omp parallel for
		for (int idx = CFG_iImageLoadBegin; idx <= CFG_iImageLoadEnd; idx++) {
			
			FeatureState fs(idx);
			fs.detect(CFG_iMaxFeatures);
			printf("FeatureDetect[%d]-\n",idx);
		}
	
	}

	getchar();
	return 0;
}