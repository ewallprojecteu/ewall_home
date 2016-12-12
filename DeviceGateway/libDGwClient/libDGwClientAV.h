/*=================================================================================
Basic explanation:

Authors: Daniel Denkovski (UKIM) (danield@feit.ukim.edu.mk)

Revision History:
    22.09.2014, ver 0.1
====================================================================================*/

#ifndef LIBDGwClientAV_H
#define LIBDGwClientAV_H

#include "libDGwClient.h"
#include <opencv2/core/core.hpp>

class DGwClientAV: public DGwClient
{
public:
	int initializeDGwClientAV();

	DGwClientAV();
	
	~DGwClientAV();

	int pushFrame (string avDeviceID, void *frameData, string frameConfig);

	//Function to return frames that are being written by pushFrame
	bool readFrame(cv::Mat& image, long long *timestamp=NULL);

	bool readDepthFrame(cv::Mat& image, long long *timestamp=NULL);

	bool readAudioSamples(cv::Mat& image, long long *timestamp=NULL);

private:
	int buffSize;                   //Size of circular buffers
	vector<cv::Mat> camFrames;      //Circular buffer for camera frames
	vector<long long> timeStamps;   //Corresponding timestamps
	long long frameCounter;         //Number of last written RGB frame
	long long lastRead;             //Number of last read RGB frame
	
	vector<cv::Mat> depthFrames;      	//Circular buffer for camera frames
	vector<long long> depthtimeStamps;   	//Corresponding timestamps
    	long long depthframeCounter;         	//Number of last written depth frame
	long long depthlastRead;                //Number of last read depth frame
    
	vector<cv::Mat> audioFrames;      	//Circular buffer for camera frames
	vector<long long> audiotimeStamps;   	//Corresponding timestamps
    	long long audioframeCounter;         	//Number of last written audio frame
	long long audiolastRead;                //Number of last read depth frame

    };

#endif
