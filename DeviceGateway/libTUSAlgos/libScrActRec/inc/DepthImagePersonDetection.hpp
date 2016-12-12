#ifndef __TDEPTHIMAGEPERSONDETECTION__
#define __TDEPTHIMAGEPERSONDETECTION__

#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <opencv2/opencv.hpp>
#include <stdint.h>

using namespace std;
using namespace cv;

enum depthStatus
{
	NotReady,
	NotInRange,
	InRangeMoving,
	InRange,
	unknown = 255
};

class DepthImagePersonDetection
{
public:
	int Width, Height;
	double _thresholdForInRangeDetection;  // 7; // percent differnence
	double _movingCoefThreshold; // 10;
	double _sizeThresholdInPixels; //10; // percent from picture

	double _distanceToAnalyze;
	double _distranceThreshold;
	double _ir_threshold, _blob_size_threshold, _use_ir;

	int _numberOfFramesToAnalyze;
	int _resizeFactor;
	int _resizedWidth;
	int _resizedHeight;
	int *percentRatiosInFrames;
	cv::Point *centerPointInFrames;
	double *frameCoefs;
	depthStatus *frameStatuses;

	depthStatus depthImageStatus;

	DepthImagePersonDetection(double ir_threshold, double blob_size_threshold)
	{
	    _ir_threshold = ir_threshold;
	    _blob_size_threshold = blob_size_threshold;
            _use_ir = 1;
	    erosion_size = 1;	
            erosion_element = getStructuringElement(MORPH_RECT, Size(2*erosion_size + 1, 2*erosion_size + 1), Point(erosion_size,erosion_size));
            resized = Mat(Size(160, 106),CV_8UC1);
            binmask = Mat(Size(160, 106),CV_8UC1);
	}

	DepthImagePersonDetection(double distance, double distThreshold, int resizeFactor):
	 Width(640), Height(480), _thresholdForInRangeDetection(7), _movingCoefThreshold(10), _sizeThresholdInPixels(10) 

	{
		_numberOfFramesToAnalyze = 8;
		_distanceToAnalyze = distance;
		_distranceThreshold = distThreshold;
		_resizeFactor = resizeFactor;
                _use_ir = 0;

		_resizedWidth = Width / resizeFactor;
		_resizedHeight = Height / resizeFactor;

		percentRatiosInFrames = new int[_numberOfFramesToAnalyze];
		centerPointInFrames = new Point[_numberOfFramesToAnalyze];
		frameStatuses = new depthStatus[_numberOfFramesToAnalyze];
		frameCoefs = CalcFrameCoeficients();

		for (int i = 0; i < _numberOfFramesToAnalyze; i++)
		{
			percentRatiosInFrames[i] = 0;
			centerPointInFrames[i] = Point(0, 0);
			frameStatuses[i] = NotReady;
		}

		depthImageStatus = NotReady;
	}

	~DepthImagePersonDetection()
	{
		delete[] percentRatiosInFrames;
		delete[] centerPointInFrames;
		delete[] frameCoefs;
		delete[] frameStatuses;
	}

	depthStatus AnalyzeDepth(IplImage *);
	depthStatus AnalyzeIr(Mat &imageIr);

private:
        Mat eroded,erosion_element;
        int erosion_size;
	Mat  resized;
	Mat  binmask;
	void FrameStatusDecision();
	void ShiftValueRegisters();
	bool CheckSurroundingPixelsDiff(uint16_t *, int, int, double, int, int);
	depthStatus curStatus;

	double *CalcFrameCoeficients()
	{
		double *frameCoefs = new double[_numberOfFramesToAnalyze];

		frameCoefs[0] = 0.02;
		frameCoefs[1] = 0.04;
		frameCoefs[2] = 0.07;
		frameCoefs[3] = 0.12;
		frameCoefs[4] = 0.17;
		frameCoefs[5] = 0.24;
		frameCoefs[6] = 0.31;

		return frameCoefs;
	}
};

#endif
