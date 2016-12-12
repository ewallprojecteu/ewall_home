

/*=================================================================================
Basic explanation:

    Device Gateway cpp code

Authors: Daniel Denkovski (UKIM) (danield@feit.ukim.edu.mk)

Revision History:
    22.09.2014, ver 0.1
====================================================================================*/

#include "libDGwClientAV2.h"
#include "json_spirit_dgw.h"
#include <cstdio>
#include <cstring>

DGwClientAV2::~DGwClientAV2()
{
}

DGwClientAV2::DGwClientAV2()
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

int DGwClientAV2::initializeDGwClientAV2()
{
	this->initializeDGwClient();
	cout << "DGwClientAV2: DGwClientAV2 initialized!\n";
	return 1;
}
	
int DGwClientAV2::pushFrame(string avDeviceID, void *frameData, string frameConfig)
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

	if (resultsLength > 0)
	{
		if (streamFormat.stream == 1)
		{
			int camIdx = (frameCounter+1)%buffSize; //Select the next buffer to write data
			vector<char> newFrame;
			newFrame.resize(resultsLength);
			memcpy(&newFrame[0], frameData, resultsLength);
			camFrames[camIdx] = newFrame;
			frameCounter++; //After data was written, increase the counter
		}
// TODO: Test the depth
		else if (streamFormat.stream == 2)
		{
			int depthIdx = (depthframeCounter+1)%buffSize; //Select the next buffer to write data
			vector<char> newDepth;
			newDepth.resize(resultsLength);
			memcpy(&newDepth[0], frameData, resultsLength);
			depthFrames[depthIdx] = newDepth;
            		depthframeCounter++; //After data was written, increase the counter
			//cout << "written depth frame " << depthIdx << endl;// << depthFrames[depthIdx](cv::Rect(0,0,5,5)) << endl;
		}
// TODO: Test the audio
		else if (streamFormat.stream == 3)
		{
			int audioIdx = (audioframeCounter+1)%buffSize; //Select the next buffer to write data
			vector<char> newAudio;
			newAudio.resize(resultsLength);
			memcpy(&newAudio[0], frameData, resultsLength);
			audioFrames[audioIdx] = newAudio;
            		audioframeCounter++; //After data was written, increase the counter
			//cout << "written audio frame " << audioIdx << endl;
		}
	}
	
//	cout << "DGwClientAV2: DGwClient pushing frame from AVDevice with ID = " << avDeviceID << "to AV processing !\n";
	return 1;
}

bool DGwClientAV2::readFrame(vector<char> image, long long *timestamp)
{
	int maxTries = 1000; //Number of milliseconds to wait for reading frame
	while (lastRead==frameCounter && maxTries-->0)
		usleep(1000);

	//If still waiting after maxTries
	if (maxTries<=0 && lastRead==frameCounter)
	{
		cout << "Timeout in readFrame" << endl;
		return false;
	}
	//To reach here means lastRead<frameCounter

	//cout << "last read " << lastRead << ", frameCounter" << frameCounter << endl;
	int camIdx = ++lastRead%buffSize;
	image = camFrames[camIdx];
	//cout << "read frame " << camIdx << endl << image(cv::Rect(0,0,5,5)) << endl;
	return true;
}

bool DGwClientAV2::readDepthFrame(vector<char> image, long long *timestamp)
{
	int maxTries = 1000; //Number of milliseconds to wait for reading frame
	while (depthlastRead==depthframeCounter && maxTries-->0)
		usleep(1000);

	//If still waiting after maxTries
	if (maxTries<=0 && depthlastRead==depthframeCounter)
	{
		cout << "Timeout in readFrame" << endl;
		return false;
	}
	//To reach here means depthlastRead<depthframeCounter

	//cout << "depth last read " << depthlastRead << ", depthframeCounter" << depthframeCounter << endl;
	int depthIdx = ++depthlastRead%buffSize;
	image = depthFrames[depthIdx];
	//cout << "read frame " << camIdx << endl << image(cv::Rect(0,0,5,5)) << endl;
	return true;
}

bool DGwClientAV2::readAudioSamples(vector<char> image, long long *timestamp)
{
	int maxTries = 1000; //Number of milliseconds to wait for reading frame
	while (audiolastRead==audioframeCounter && maxTries-->0)
		usleep(1000);

	//If still waiting after maxTries
	if (maxTries<=0 && audiolastRead==audioframeCounter)
	{
		cout << "Timeout in readFrame" << endl;
		return false;
	}
	//To reach here means audiolastRead<audioframeCounter

	//cout << "audio last read " << audiolastRead << ", audioframeCounter" << audioframeCounter << endl;
	int audioIdx = ++audiolastRead%buffSize;
	image = audioFrames[audioIdx];
	//cout << "read frame " << camIdx << endl << image(cv::Rect(0,0,5,5)) << endl;
	return true;
}
