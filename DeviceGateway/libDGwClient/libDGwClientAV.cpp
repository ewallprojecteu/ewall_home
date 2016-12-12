

/*=================================================================================
Basic explanation:

    Device Gateway cpp code

Authors: Daniel Denkovski (UKIM) (danield@feit.ukim.edu.mk)

Revision History:
    22.09.2014, ver 0.1
====================================================================================*/

#include "libDGwClientAV.h"
#include "json_spirit_dgw.h"
#include <opencv2/imgproc/imgproc.hpp>
#include <cstdio>
#include <cstring>

DGwClientAV::~DGwClientAV()
{
}

DGwClientAV::DGwClientAV()
{
	// common to all channels
    	buffSize=10;
	
    	//RGB channel
    	lastRead = frameCounter=-1;
	camFrames.resize(buffSize);
	timeStamps.resize(buffSize);
    
    	//Depth channel
    	depthlastRead = depthframeCounter=-1;
	depthFrames.resize(buffSize);
	depthtimeStamps.resize(buffSize);
    
    	//Audio channel
    	audiolastRead = audioframeCounter=-1;
	audioFrames.resize(buffSize);
	audiotimeStamps.resize(buffSize);

}

int DGwClientAV::initializeDGwClientAV()
{
	this->initializeDGwClient();
	cout << "DGwClientAV: DGwClientAV initialized!\n";
	return 1;
}
	
int DGwClientAV::pushFrame(string avDeviceID, void *frameData, string frameConfig)
{
	streamConfig streamFormat = JsonToStreamConfig(frameConfig);
	int resultsLength = 0;

	if ((streamFormat.stream == 1) || (streamFormat.stream == 2) || (streamFormat.stream == 3))
	{
		resultsLength = (streamFormat.resolution.width*streamFormat.resolution.height)*streamFormat.nochs*streamFormat.bytesps;
		//RGB format as unsigned char
		//depth format as uint16_t
		//audio format as int32_t
	}

	cv::Size frameRes = cv::Size(streamFormat.resolution.width,streamFormat.resolution.height);
	if (resultsLength > 0)
	{
		if (streamFormat.stream == 1)
		{
			int camIdx = (frameCounter+1)%buffSize; //Select the next buffer to write data
			cv::cvtColor(cv::Mat(frameRes,CV_8UC3,frameData) , camFrames[camIdx], CV_RGB2BGR);
			frameCounter++; //After data was written, increase the counter
		}
// TODO: Test the depth
		else if (streamFormat.stream == 2)
		{
			int depthIdx = (depthframeCounter+1)%buffSize; //Select the next buffer to write data
            		depthFrames[depthIdx]=cv::Mat(frameRes,CV_16UC1,frameData).clone();
            		depthframeCounter++; //After data was written, increase the counter
			//cout << "written depth frame " << depthIdx << endl;// << depthFrames[depthIdx](cv::Rect(0,0,5,5)) << endl;
		}
// TODO: Test the audio
		else if (streamFormat.stream == 3)
		{
			int audioIdx = (audioframeCounter+1)%buffSize; //Select the next buffer to write data
            		// Krasimir: This is the approach that audio is 256x1 and 4 channels matrix
            		//audioFrames[audioIdx]=cv::Mat(frameRes,CV_32SC4 ,frameData).clone();
            
            		// Krasimir: This is the approach that audio is 256x4 and 1 channels matrix
            		frameRes = cv::Size(streamFormat.resolution.width, streamFormat.resolution.height*streamFormat.nochs);
            		audioFrames[audioIdx]=cv::Mat(frameRes,CV_32SC1 ,frameData).clone();
            		audioframeCounter++; //After data was written, increase the counter
			//cout << "written audio frame " << audioIdx << endl;
		}
	}
	
//	cout << "DGwClientAV: DGwClient pushing frame from AVDevice with ID = " << avDeviceID << "to AV processing !\n";
	return 1;
}

bool DGwClientAV::readFrame(cv::Mat& image, long long *timestamp)
{
	int maxTries = 1000; //Number of milliseconds to wait for reading frame
	while (lastRead==frameCounter && maxTries-->0)
		usleep(1000);

	//If still waiting after maxTries
	if (maxTries<=0 && lastRead==frameCounter)
	{
		cout << "Timeout in readFrame" << endl;
		image = cv::Mat();
		return false;
	}
	//To reach here means lastRead<frameCounter

	//cout << "last read " << lastRead << ", frameCounter" << frameCounter << endl;
	int camIdx = ++lastRead%buffSize;
	image = camFrames[camIdx].clone();
	//cout << "read frame " << camIdx << endl << image(cv::Rect(0,0,5,5)) << endl;
	return true;
}

bool DGwClientAV::readDepthFrame(cv::Mat& image, long long *timestamp)
{
	int maxTries = 1000; //Number of milliseconds to wait for reading frame
	while (depthlastRead==depthframeCounter && maxTries-->0)
		usleep(1000);

	//If still waiting after maxTries
	if (maxTries<=0 && depthlastRead==depthframeCounter)
	{
		cout << "Timeout in readFrame" << endl;
		image = cv::Mat();
		return false;
	}
	//To reach here means depthlastRead<depthframeCounter

	//cout << "depth last read " << depthlastRead << ", depthframeCounter" << depthframeCounter << endl;
	int depthIdx = ++depthlastRead%buffSize;
	image = depthFrames[depthIdx].clone();
	//cout << "read frame " << camIdx << endl << image(cv::Rect(0,0,5,5)) << endl;
	return true;
}

bool DGwClientAV::readAudioSamples(cv::Mat& image, long long *timestamp)
{
	int maxTries = 1000; //Number of milliseconds to wait for reading frame
	while (audiolastRead==audioframeCounter && maxTries-->0)
		usleep(1000);

	//If still waiting after maxTries
	if (maxTries<=0 && audiolastRead==audioframeCounter)
	{
		cout << "Timeout in readFrame" << endl;
		image = cv::Mat();
		return false;
	}
	//To reach here means audiolastRead<audioframeCounter

	//cout << "audio last read " << audiolastRead << ", audioframeCounter" << audioframeCounter << endl;
	int audioIdx = ++audiolastRead%buffSize;
	image = audioFrames[audioIdx].clone();
	//cout << "read frame " << camIdx << endl << image(cv::Rect(0,0,5,5)) << endl;
	return true;
}
