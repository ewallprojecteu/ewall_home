#include "videoCaptureAdv.h"
#include "tools.h"

using namespace std;

namespace cv
{
VideoCaptureAdv::VideoCaptureAdv()
{
	isLive=false;
	capTicks=startTicks=-10000000;
	framerate=-1;
	posFrames=0;
}

bool VideoCaptureAdv::open(const string& filename)
{
	if (!isOpened())
	{
		if (filename.length()<=3) //Open specified device id (0-999)
		{
			//Will open default camera if invalid char, as atoi will return 0
			return open(atoi(filename.c_str()));
		}
		cap = cvCreateFileCapture(filename.c_str());
	}
	// Fix buggy framerates in Capture###.wmv videos
	if (isOpened())
	{
		string ext;
		string videoname=getName(filename, &ext);
		if (videoname.compare(0,7,"Capture")==0 && ext.compare(0,3,"wmv")==0 )
			framerate=24;
	}
	return isOpened();
}

bool VideoCaptureAdv::open(int device)
{
	if (!isOpened())
	{
		cout << "Opening device " << device << endl;
		cap = cvCreateCameraCapture(device);
		isLive=isOpened();
	}
	return isOpened();
}

bool VideoCaptureAdv::grab()
{
	capTicks=getTickCount();
	posFrames++;
	if (startTicks<0) startTicks=capTicks;
	return cvGrabFrame(cap) != 0;
}

bool VideoCaptureAdv::set(int propId, double value)
{
	return cvSetCaptureProperty(cap, propId, value) != 0;
}

double VideoCaptureAdv::get(int propId)
{
	static double tickFreq = getTickFrequency(); //tick frequency
	double tmp = cvGetCaptureProperty(cap, propId);
	//Correct handling of live cameras
	if (isLive)
	{
		// Fix fps retrieval
		if (propId == CV_CAP_PROP_FPS)
		{
			if (framerate<0)
			{
				double tmp1=getTickFrequency();
				std::cout << "Please wait, estimating camera framerate\n";
				Mat tmp;
				for (int i=0;i<20;i++)
					read(tmp);
				int64 start_ticks=capTicks;
				int count=100;
				for (int i=0;i<count;i++)
					read(tmp);
				framerate = count*tickFreq/(capTicks-start_ticks);
				startTicks=capTicks;
				posFrames=0;
			}
			return framerate;
		}
		// Fix timestamp retrieval
		else if (propId == CV_CAP_PROP_POS_MSEC)
			return (capTicks-startTicks)*1000/tickFreq;
		// Fix frame position retrieval
		else if (propId == CV_CAP_PROP_POS_FRAMES)
			return (double) posFrames;
	}
	// Fix buggy video framerates
	if (propId == CV_CAP_PROP_FPS && framerate>0)
		return framerate;
	if (propId == CV_CAP_PROP_POS_MSEC && framerate>0)
	{
		double test=cvGetCaptureProperty(cap, CV_CAP_PROP_POS_MSEC);

		return cvGetCaptureProperty(cap, CV_CAP_PROP_POS_FRAMES)*1000/framerate;
	}

	return cvGetCaptureProperty(cap, propId);
}
}
