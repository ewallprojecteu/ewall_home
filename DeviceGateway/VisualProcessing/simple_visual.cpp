#include <iostream>
#include "Communication.h"
#include "SimpleProcessing.h"
#include "time_funcs.h"

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

using namespace std;

int main(int argc, char *argv[])
{
	bool      active;
	double    luminance;
	cv::Mat   procFrame;
	long long frameCounter=-1;

	string serverName = "http://localhost:5984/visual_environment/";
	
//Initialize based on input type
#ifndef NO_DEVICEGATEWAY
	if (argc>2)
	{
		cout<<"Usage: " << argv[0] << " \"server name\"" << endl;
		return -1;
	}
	if (argc==2)
		serverName = argv[1];
	DGwClientAV AVProcess;
	AVProcess.initializeDGwClientAV();
#else //NO_DEVICEGATEWAY
	if (argc<2)
	{
		cout<<"Usage: " << argv[0] << " \"video name\"" << " (\"server name\")" << endl;
		return -1;
	}
	cv::VideoCapture cap(argv[1]);
//	cv::VideoCapture cap(0);
	if (!cap.isOpened())
	{
		cout<<"Video " << argv[1] << " could not be opened." << endl;
		return -1;
	}
#endif //NO_DEVICEGATEWAY

	cout << "Initializing couchdb server: " << serverName << endl;
	Communication couchdb(serverName);
	SimpleProcessing proc;

#ifndef NO_DEVICEGATEWAY
	while (AVProcess.readFrame(procFrame))
#else
	while (cap.read(procFrame))
#endif
	{
		frameCounter++;
		proc.processFrame(procFrame, &active, &luminance);
		stringstream frameSstrean; //Local so that it gets cleared in each loop
		frameSstrean << "Frame: " << frameCounter << ", active:" << (active?"true":"false") << ", luminance:" << luminance;
		if (frameCounter%50==0) 
			cout << frameSstrean.str() << endl;
		couchdb.sendMessage(getMillis(), active, luminance);
        char ret=(char) proc.showFrame(procFrame,frameSstrean.str());
        if (ret == 'q' || ret == 'Q' || ret == 27 /*ESC*/)
            break; //Stop the program
	}
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
	return 0;
}

