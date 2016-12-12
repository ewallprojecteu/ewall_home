#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <cmath>
#include <stdint.h>

#include "DepthImagePersonDetection.hpp"
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;




depthStatus DepthImagePersonDetection::AnalyzeIr(Mat &imageIr)
{
 
    //double min, max;
    //cv::minMaxLoc(temp, &min, &max);		
    //cout << "Min " << min << " max "<< max << endl;
    
    // Resize the image since we dont need a lot of processing	
    Mat temp(Size(160, 106),CV_8UC1);
    imageIr.convertTo(temp, CV_8UC1, 1/256.0f); 
    resize(temp,resized,resized.size(),0,0,INTER_LINEAR);

    // Do thresholding 
    threshold(resized, binmask, _ir_threshold, 1, THRESH_BINARY);
    
    // Apply erosion or dilation on the image
    erode(binmask, binmask, erosion_element);
    dilate(binmask, binmask, erosion_element);
    Scalar totalactivepix = sum(binmask);
    
    //imshow("Show window",255*binmask);
    //char c = waitKey(25);

    if(totalactivepix[0] > _blob_size_threshold){
	//cout << "Presence Detected" <<endl;
	return InRange;
    }
    else{
	//cout << "Presence Not Detected" <<endl;
        return NotInRange;
    }
}


depthStatus DepthImagePersonDetection::AnalyzeDepth(IplImage *imageDepth)
{
	
        int width = DepthImagePersonDetection::Width;
              

	/*calc confident level - is needed because of depth values imperfection.*/
	int pixelDistanceForConfidentLevelCalc = 5;
	double depthConfidentLevel = 20; // % difference -current pixel - surrounding pixels
	int confCalcMethod = 0;		     // options are 0 and 1. Method 0 is Simpler. Method 1 is more accurate and slower for bigger distances.

	IplImage *destination = cvCreateImage
		(cvSize((int)(_resizedWidth), (int)(_resizedHeight)), imageDepth->depth, imageDepth->nChannels);

	cvResize(imageDepth, destination, CV_INTER_NN);

	uint16_t *dataPtr = (uint16_t *)destination->imageData;

	int curFramePoints = 0;
	int curMaxX = 0;
	int curMaxY = 0;
	int curMinX = _resizedWidth;
	int curMinY = _resizedHeight;

	int endPointY = _resizedHeight - pixelDistanceForConfidentLevelCalc;
	int endPointX = _resizedWidth - pixelDistanceForConfidentLevelCalc;

	int minVal = 99999;
	int maxVal = 0;

	for (int i = pixelDistanceForConfidentLevelCalc; i < endPointY; i++)
	{
		for (int j = pixelDistanceForConfidentLevelCalc; j < endPointX; j++)
		{
			int curValue = dataPtr[i *_resizedWidth + j];
			if (curValue != 0)
			{
				if (minVal > curValue)
				{
					minVal = curValue;
				}
				if (maxVal < curValue)
				{
					maxVal = curValue;
				}
			}
			// kinect formula
			//https://sites.google.com/site/leekinectproject/project-updates/kinect-depthcaliberation
			
			double curDistance = curValue / 1000.0;//0.1236 * tan(curValue / 2842.5 + 1.1863);//1.0 / (curValue * -0.0030711016 + 3.3309495161);
			if ((curDistance >= (_distanceToAnalyze - _distranceThreshold)) && (curDistance <= (_distanceToAnalyze + _distranceThreshold)))
			{
				if (CheckSurroundingPixelsDiff(dataPtr, i, j, depthConfidentLevel, pixelDistanceForConfidentLevelCalc, confCalcMethod) == true)
				{
					curFramePoints++;
					if (i < curMinY)
					{
						curMinY = i;
					}
					if (i > curMaxY)
					{
						curMaxY = i;
					}
					if (j < curMinX)
					{
						curMinX = j;
					}
					if (j > curMaxY)
					{
						curMaxX = j;
					}
				}
			}
		}
	}

	percentRatiosInFrames[_numberOfFramesToAnalyze - 1] = curFramePoints * 100 / (_resizedHeight * _resizedWidth);

	centerPointInFrames[_numberOfFramesToAnalyze - 1] = Point((curMaxX - curMinX), (curMaxY - curMinY));

	FrameStatusDecision();

	ShiftValueRegisters();

	return frameStatuses[_numberOfFramesToAnalyze - 2];
}

double ditanceBetweenPoints(Point p1, Point p2)
{
	int distX = p1.x - p2.x;
	int distY = p1.y - p2.y;

	double result = sqrt((distX *distX) + (distY *distY));
	return result;
}

void DepthImagePersonDetection::FrameStatusDecision()
{ 
	double interFramesDistane = 0;
	double interFramesSizeDiff = 0;
	Point curPoint = centerPointInFrames[_numberOfFramesToAnalyze - 1];
	double curSize = percentRatiosInFrames[_numberOfFramesToAnalyze - 1];

	if (percentRatiosInFrames[_numberOfFramesToAnalyze - 1] < _sizeThresholdInPixels)
	{
		frameStatuses[_numberOfFramesToAnalyze - 1] = NotInRange;
	}
	else
	{
		for (int i = 0; i < _numberOfFramesToAnalyze - 1; i++)
		{
			//calculate distance
			interFramesDistane += (frameCoefs[i] * ditanceBetweenPoints(curPoint, centerPointInFrames[i]));
			//calculate sizeBetweenFrames
			interFramesSizeDiff += (frameCoefs[i] * abs(curSize - percentRatiosInFrames[i]));
		}

		if (interFramesDistane > _movingCoefThreshold)
		{
			frameStatuses[_numberOfFramesToAnalyze - 1] = InRangeMoving;
		}
		else
		{
			if (interFramesSizeDiff < _thresholdForInRangeDetection)
			{
				frameStatuses[_numberOfFramesToAnalyze - 1] = InRange;
			}
			else
			{
				frameStatuses[_numberOfFramesToAnalyze - 1] = NotReady;
			}
		}
	}
}

void DepthImagePersonDetection::ShiftValueRegisters()
{
	for (int i = 0; i < _numberOfFramesToAnalyze - 1; i++)
	{
		percentRatiosInFrames[i] = percentRatiosInFrames[i + 1];
		centerPointInFrames[i] = centerPointInFrames[i + 1];
		frameStatuses[i] = frameStatuses[i + 1];
	}
	percentRatiosInFrames[_numberOfFramesToAnalyze - 1] = 0;
	centerPointInFrames[_numberOfFramesToAnalyze - 1] = Point(0, 0);
	frameStatuses[_numberOfFramesToAnalyze - 1] = NotReady;
}

bool DepthImagePersonDetection::CheckSurroundingPixelsDiff(uint16_t *imgData, int i, int j, double depthConfidentLevel, int pixelDistance, int method)
{
	bool result = false;

	int width = _resizedWidth;// DepthImagePersonDetection::Width;

	int curPixelValue = imgData[i * width + j];
	int sumSurroundingElements = 0;
	double averageSurroundingElements = 0;

	switch (method)
	{
		case 0:   //check in 8 direction in distance == pixelDistance
		{
			sumSurroundingElements += imgData[(i - pixelDistance) * width + (j - pixelDistance)];
			sumSurroundingElements += imgData[(i - pixelDistance) * width + (j + pixelDistance)];
			sumSurroundingElements += imgData[(i - pixelDistance) * width + (j)];
			sumSurroundingElements += imgData[(i + pixelDistance) * width + (j - pixelDistance)];
			sumSurroundingElements += imgData[(i + pixelDistance) * width + (j + pixelDistance)];
			sumSurroundingElements += imgData[(i + pixelDistance) * width + (j)];
			sumSurroundingElements += imgData[(i)* width + (j - pixelDistance)];
			sumSurroundingElements += imgData[(i)* width + (j + pixelDistance)];

			averageSurroundingElements = sumSurroundingElements /= 8.0;
		}
		break;

		case 1:
		{
			for (int y = -pixelDistance; y <= pixelDistance; y++)
			{
				for (int x = -pixelDistance; x <= pixelDistance; x++)
				{
					sumSurroundingElements += imgData[(i + y) * width + (j + x)];
				}
			}

			averageSurroundingElements = sumSurroundingElements /= ((double)(pixelDistance * pixelDistance));
		}
		break;

		default:
			break;
	}

	double difference = depthConfidentLevel * curPixelValue / 100.0;
	if (abs(averageSurroundingElements - curPixelValue) < difference)
	{
		result = true;
	}

	return result;
}
