#include "Communication.h"
#include "FaceProcessing.h"
#include "time_funcs.h"
#include "videoCaptureAdv.h"
#include "display.h"
#include <iostream>
#include <iomanip>

//Define NO_DEVICEGATEWAY to read video from file without using Device Gateway 
//#define NO_DEVICEGATEWAY

#ifndef NO_DEVICEGATEWAY
#include "libDGwClientAV.h"
#include "DGWdefs.h"
#else //NO_DEVICEGATEWAY
#include <opencv2/highgui/highgui.hpp>
#endif //NO_DEVICEGATEWAY

#ifdef _WIN32
#include <Windows.h> //For IsDebuggerPresent()
#endif //

#define JUSTRECORD 0 //0:tracking, 1:only record video

using namespace std;


inline double setExp(cv::VideoCapture& cap, double newexp)
{
	cv::Mat tmp;
	int expsetframes=0;
	cap.set(CV_CAP_PROP_EXPOSURE, newexp-1);
	while (cap.get(CV_CAP_PROP_EXPOSURE)!=newexp)
	{
		cap >> tmp;
		expsetframes++;
	}
	cout << "Exposure set to " << newexp << " after " << expsetframes << " frames\n";
	for (int i=0;i<10;i++)
		cap >> tmp;
	return newexp;
}


inline double setGain(cv::VideoCapture& cap, double newgain)
{
	cv::Mat tmp;
//	int gainsetframes=0;
	cap.set(CV_CAP_PROP_GAIN, newgain);
	//while (cap.get(CV_CAP_PROP_GAIN)!=newgain && gainsetframes<10)
	//{
	//	cap >> tmp;
	//	gainsetframes++;
	//}
	//cout << "Gain set to " << newgain << " after " << gainsetframes << " frames\n";
	for (int i=0;i<10;i++)
		cap >> tmp;
	return newgain;
}

int process(int argc, char *argv[])
{
	cv::Mat   procFrame;
	int64 frameCounter = -1, frgRestartFrame = -1;
	bool      running = false, captured = false;

//	string serverName = "http://localhost:5984/face_processing/";
	string serverName = "localhost";
	string ageGenderData = "AgeGenderData.bin";

	//Configuration variables
	bool showForeground=false, showHelp=false; // Whether to display foreground and help
	int showStatistics=1; // 0: disable, 1: embed, 2: new window
	int DISP_FRAMES=4;    // Every how many frames to display
	int pause=10;         // for pausing the output
	int skipFrames=20;    // How many frames to skip at start

//Initialize based on input type
#ifndef NO_DEVICEGATEWAY
	if (argc == 1)
	{
		cout<<"Usage: " << argv[0] << " \"server name\" (\"display every n frames [" << DISP_FRAMES << "]\")" << endl;
		return -1;
	}
	if (argc > 1)
		serverName = argv[1];
	if (argc > 2)
		DISP_FRAMES = atoi(argv[2]);
	DGwClientAV AVProcess;
	AVProcess.initializeDGwClient();
#else //NO_DEVICEGATEWAY
	int DESIRED_WIDTH = 1920;
	int DESIRED_HEIGHT = 1080;
	double DESIRED_FPS = 30.0;

	if (argc<2)
	{
		cerr<<"Usage: " << argv[0] << " \"video name\"" << " (\"server name\") (\"display every n frames\")" << endl;
		cerr<<"Try " << argv[0] << " 0" << " (\"server name\") to use the default webcam" << endl;
		return -1;
	}
	if (argc>2)
		serverName = argv[2];
	if (argc > 3)
		DISP_FRAMES = atoi(argv[3]);

	int width, height;
	double fps;
	double gain0, gain;

	cv::VideoCaptureAdv cap;
	cap.open(argv[1]);
	if (!cap.isOpened())
	{
		cerr<<"Video " << argv[1] << " could not be opened." << endl;
		return -1;
	}

	cap.set(CV_CAP_PROP_FRAME_WIDTH, (double)DESIRED_WIDTH);
	cap.set(CV_CAP_PROP_FRAME_HEIGHT, (double)DESIRED_HEIGHT);
	cap.set(CV_CAP_PROP_FPS, DESIRED_FPS);
	width = (int)cap.get(CV_CAP_PROP_FRAME_WIDTH);
	height = (int)cap.get(CV_CAP_PROP_FRAME_HEIGHT);

	fps = cap.get(CV_CAP_PROP_FPS);
	setDisplayParams(fps, 20);

	cap.set(CV_CAP_PROP_SETTINGS,0); //Show camera properties window
	cap.set(CV_CAP_PROP_GAIN,127.0);
	gain0 = cap.get(CV_CAP_PROP_GAIN);
//	exposure = exposure0 = setExp(cap,-5);
	gain=gain0;

	printf("Camera video is %dx%d @%6.2ffps\n", width, height, fps);
//	cout << "exposure0: " << exposure0 << endl;
	cout << "gain0: " << gain0 << endl;

#endif //NO_DEVICEGATEWAY

	cout << "Dispalying every " << DISP_FRAMES << " frames" << endl;
	cout << "Initializing couchdb server: " << serverName << endl;
	Communication couchdb(serverName);
	FaceProcessing proc;

	int64 startTime = getMillis(), frameTime = -1000000; // timestamp in ms of current frame
	running = true;

	while (running)
	{
#ifndef NO_DEVICEGATEWAY
		captured = AVProcess.readFrame(procFrame);
		frameTime = getMillis(); // TODO: Get time from gateway
#else //NO_DEVICEGATEWAY
		captured = cap.read(procFrame);
		frameTime = startTime + (int64) cap.get(CV_CAP_PROP_POS_MSEC);
#endif //NO_DEVICEGATEWAY
		if (!captured)
		{
			running = false;
			break;
		}
		frameCounter++;
		try {
			proc.processFrame(procFrame, frameTime);
		} catch (exception &ex) {
			cerr << ex.what() << "\n";
			break;
		}

		if (frameCounter%DISP_FRAMES==0)
		{
			proc.drawTargets(procFrame, frameTime);
			stringstream frameSstream; //Local so that it gets cleared in each loop
			int minFaceSize = proc.getMinFaceSize();
			frameSstream << "Frame: " << frameCounter << ", minFaceSize (change with +,-): " << minFaceSize;
			if (frameCounter%(DISP_FRAMES*50)==0) 
				cout << frameSstream.str() << endl;
			char ret;//=(char) proc.showFrame(procFrame, frameSstream.str());
			if (ret == 'q' || ret == 'Q' || ret == 27 /*ESC*/)
				break; //Stop the program
			else if (ret == '-')
				proc.setMinFaceSize(minFaceSize-5);
			else if (ret == '+')
				proc.setMinFaceSize(minFaceSize+5);
		}
		couchdb.sendMessage(frameTime, proc.getTargets());
	}
	return 0;
}

int main(int argc, char *argv[])
{
	int ret = process(argc, argv);
#ifdef _WIN32
	//Pause if run within Visual Studio
	if (IsDebuggerPresent())
	{
		cv::destroyAllWindows();
		cout << "Press Enter to exit ";
		getchar();
	}
#endif //WIN32
	cout << "Exiting..." << endl;
	return ret;
}

