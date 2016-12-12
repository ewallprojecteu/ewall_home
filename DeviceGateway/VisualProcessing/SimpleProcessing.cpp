/*=================================================================================
Description: Performs simple visual processing

Authors: Nikolaos Katsarakis (AIT) (nkat@ait.edu.gr)

Revision History:
    13.11.2014, ver 0.1
====================================================================================*/

#include "SimpleProcessing.h"
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/highgui/highgui.hpp>

SimpleProcessing::SimpleProcessing()
{
}

SimpleProcessing::~SimpleProcessing()
{
//	std::cout << "Closing windows" << std::endl;
	cv::destroyAllWindows();
}

int SimpleProcessing::processFrame(const cv::Mat& camFrame, bool *active, double *luminance)
{
	static cv::Mat   frame[2], absDiff, grayFrame;
	static long long frameCounter=-1, latestActiveFrame=-1;
	cv::Scalar       meanDiff;
	double	   sumMeanDiff;
	double	   activationThreshold = 3.0, keepActiveThreshold = 1.0;
	long	     activeTimeout = 100;

	frame[++frameCounter%2]=camFrame.clone();
	cv::cvtColor(frame[frameCounter%2], grayFrame, CV_BGR2GRAY);
	meanDiff = cv::mean(grayFrame);
	*luminance = meanDiff.val[0]*100/255;

	// Skip further processing if first image
	if(frameCounter==0)
	{
		 *active=false;
		 return 0;
	}

	absDiff = cv::abs(frame[0]-frame[1]);
	meanDiff = cv::mean(absDiff);
	sumMeanDiff = (meanDiff.val[0]+meanDiff.val[1]+meanDiff.val[2])*100/3/255;

	if (sumMeanDiff > activationThreshold || (*active == true && sumMeanDiff > keepActiveThreshold))
	{
		 latestActiveFrame = frameCounter;
		 *active = true;
	}
	else if (*active == true && frameCounter-latestActiveFrame > activeTimeout)
		 *active = false;

	//cout << sumMeanDiff << endl;
	//      cout << "DGwClientAVProc: DGwClient pushing frame from AVDevice with ID = " << avDeviceID << "to AV processing !\n";
	return 0;
}

int SimpleProcessing::showFrame(const cv::Mat& camFrame, const std::string& dispStr)
{
	cv::Mat dispFrame=camFrame.clone();
	cv::namedWindow("Simple visual processing",CV_WINDOW_AUTOSIZE);
	cv::putText(dispFrame, dispStr, cv::Point(0,25), CV_FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(200), 3, 8);
	cv::imshow("Simple visual processing", dispFrame);
	return cv::waitKey(10);
}
