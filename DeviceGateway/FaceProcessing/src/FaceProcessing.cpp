/*=================================================================================
Description: Performs face processing

Authors: Nikolaos Katsarakis (AIT) (nkat@ait.edu.gr)

Revision History:
    17.02.2015, ver 0.1
====================================================================================*/

#include "FaceProcessing.h"
#include <opencv2/imgproc/imgproc.hpp>
#include "genderReco.hpp"
//#include "timeticks.h"
#include <iostream>
#include <fstream>
#include <stdexcept>

using namespace std;
using namespace cv;

/* Required global variables for PF */
Mat	IyCrCb; //Input image in YCrb colorspace from main
//Mat	frgFull; //Full size foreground mask from main
Mat Igray; //Grayscale image from main
Mat	Irgb; //For displaying debug images


FaceProcessing::FaceProcessing(const string& cascadePath, int faceWidth)
{
	initialized = false;
	if (cascadePath.length()!=0)
		_cascadePath = cascadePath;
	else
	{
		char *opencvPath = getenv("OPENCV_DIR");
		if (opencvPath == NULL)
		{
			cout << "OPENCV_DIR environmental variable not found, loading haar cascades from current folder\n";
			_cascadePath = string("./");
		}
		else
		{
	//		cascadepath = string(opencvPath)+"/../../../data/haarcascades/"; // For OpenCV < 2.4.9
			_cascadePath = string(opencvPath)+"/../../../sources/data/haarcascades/";
		}
	}

	if (faceWidth>0)
		_faceWidth = faceWidth;
	else
		_faceWidth = 128;
/*	ticks.capture=0;ticks.background=0;ticks.colorconvert=0;
	ticks.facedetect=0;ticks.facetrack=0;ticks.eyedetect=0;
	ticks.drawing=0;*/
	latestTimeDetected = LLONG_MIN; // time in ms of most recent face detection

	modalities = CUE_FACECOLORMOTION;     // Which tracker modalities to use
	MIN_FRG_IN_FACE = 10;

	REPORT_STEP = 200; // Every how many frames to report timing statistics on the console
	frameCounter = frgRestartFrame = 0;
}

void FaceProcessing::init()
{

	/*Load Face sizes*/
	std::ifstream ifs( "settings.txt", ios::binary);
	if (!ifs.is_open())
		throw std::runtime_error("Can not load settings from settings.txt");

	std::string line;
	while (std::getline(ifs, line))
	{
		std::istringstream iss(line);
		if (line[0]=='#' || line[0]=='\r'|| line[0]=='\n')
			continue;
		if (!(iss >> MIN_FACE_SIZE >> MAX_FACE_SIZE)) { break; } // error
	}
	cout << "Searching for faces between " << MIN_FACE_SIZE << " and " << MAX_FACE_SIZE << "pixels\n";
	/*Finished loading face sizes*/

	// Foreground scaling variables
	decFrg = (int)pow(2.0,floor(log((double)MIN_FACE_SIZE/MIN_FRG_IN_FACE)/log(2.0)));
	scaleFrg = 1.0/decFrg;
	
	//Fix for getOriginalWindowSize() on old type cascade classifiers, opencv 2.3.0-2.4.7(currently)
	CascadeClassifierFix cascadeLeftEye, cascadeRightEye;
	if (!faceCascade.load(_cascadePath + "haarcascade_frontalface_alt.xml") ||
		!cascadeLeftEye.load(_cascadePath + "haarcascade_mcs_lefteye.xml") ||
		!cascadeRightEye.load(_cascadePath + "haarcascade_mcs_righteye.xml"))
	{
		throw std::runtime_error("Can not load face/eye cascades from " + _cascadePath);
	}

	cout << "Loaded frontalface_alt, mcs_lefteye, mcs_righteye haar cascades from  " << _cascadePath << endl;

	// Set the desired face width. Face height depends on cropping type
//	setGenderClassifiers(faceCascade, cascadeLeftEye, cascadeRightEye);
//	setGenderFaceWidth(_faceWidth);

	Size s = faceCascade.getOriginalWindowSize();
	if (s.width<=0)
	{
		cout << "Warning: face cascade width was " << s.width << ", setting to 20\n";
		s.width = 20;
	}
	faceCascadeWidth = s.width;
	decObj = (int)pow(2.0,floor(log((double)MIN_FACE_SIZE/faceCascadeWidth)/log(2.0)));
	scaleObj = 1.0/decObj;
	scaleObj2frg = decObj*scaleFrg; //Scale from face detections to foreground
	cout << "decFrg " << decFrg << ", scaleFrg " << scaleFrg << ", decObj " << decObj << ", scaleObj " << scaleObj << endl;

	string trackerName;
	if (initFaceTracker(TRACKER_PF, trackerName, modalities, _cascadePath + "haarcascade_frontalface_alt.xml", "localhost")<0)
		throw std::runtime_error("Can not initialize face tracker " + trackerName
				+ ", cascade: " + _cascadePath  + "haarcascade_frontalface_alt.xml");
	initialized = true;
}

int FaceProcessing::getMinFaceSize()
{
	return MIN_FACE_SIZE;
}

void FaceProcessing::setMinFaceSize(int NEW_MIN_FACE_SIZE )
{
	MIN_FACE_SIZE = NEW_MIN_FACE_SIZE<faceCascadeWidth?faceCascadeWidth:NEW_MIN_FACE_SIZE;
	MAX_FACE_SIZE = MIN_FACE_SIZE*5;
	cout << "Searching for faces between " << MIN_FACE_SIZE << " and " << MAX_FACE_SIZE << "pixels\n";
	// Foreground scaling variables
	decFrg = (int)pow(2.0,floor(log((double)MIN_FACE_SIZE/MIN_FRG_IN_FACE)/log(2.0)));
	scaleFrg = 1.0/decFrg;
	decObj = (int)pow(2.0,floor(log((double)MIN_FACE_SIZE/faceCascadeWidth)/log(2.0)));
	scaleObj = 1.0/decObj;
	scaleObj2frg = decObj*scaleFrg; //Scale from face detections to foreground
	//cout << "decFrg " << decFrg << ", scaleFrg " << scaleFrg << ", decObj " << decObj << ", scaleObj " << scaleObj << endl;
}

FaceProcessing::~FaceProcessing()
{
//	std::cout << "Closing windows" << std::endl;
	cv::destroyAllWindows();
}

int FaceProcessing::processFrame(const cv::Mat& camFrame, int64 frameTime)
{
	if (!initialized)
		init();


	int64	start_ticks=cv::getTickCount();
	resize(camFrame, frameForFrg, Size(), scaleFrg, scaleFrg, INTER_NEAREST);
	resize(camFrame, frameForObj, Size(), scaleObj, scaleObj, INTER_NEAREST);
	cvtColor(frameForObj, frameForObjGray, CV_BGR2GRAY );
	frameAvg = mean(frameForObjGray).val[0];
	cvtColor(camFrame, frameGray, CV_BGR2GRAY );

//	ticks.colorconvert+=cv::getTickCount()-start_ticks;

	//	showFrame(frameForFrg,"","foreground");
	//	showFrame(frameForObj,"","Face");

	if (frameCounter-frgRestartFrame < 50)
		bkgSeg(frameForFrg, frg, 1.0/(frameCounter-frgRestartFrame+1));
	else
		bkgSeg(frameForFrg, frg, .00005);

	threshold(frg,frg,250,255,THRESH_BINARY);
	dilate(frg, frg, Mat(), Point(-1,-1), 1);
	erode(frg, frg, Mat(), Point(-1,-1), 1);
//		ticks.background+=cv::getTickCount()-start_ticks;

	start_ticks=cv::getTickCount();
	if (frameCounter>29 && (_targets.size()==0 || frameCounter%10==0))
		faceCascade.detectMultiScale(frameForObjGray, detectedFaces, 1.1, 3, CV_HAAR_SCALE_IMAGE, Size(), Size((int)(MAX_FACE_SIZE*scaleObj),(int)(MAX_FACE_SIZE*scaleObj)));
	else
		detectedFaces.clear();
	if (detectedFaces.size() > 0)
		latestTimeDetected = frameTime;
	//	isTracking = true;
	//if (isTracking && faces.size() == 0)
	//{
	//	latestFrameTracked = frameCounter;
	//	isTracking = false;
	//}
//		ticks.facedetect+=cv::getTickCount()-start_ticks;

	start_ticks=cv::getTickCount();
	int numfaces=detectedFaces.size();

	Mat frameForObjFrg;
	trackerFaces.clear();
	//Track faces that have some foreground
	for (int i=numfaces-1;i>=0;i--)
	{
		Rect frgRect=Rect(cvRound(detectedFaces[i].x*scaleObj2frg), cvRound(detectedFaces[i].y*scaleObj2frg), 
			cvRound(detectedFaces[i].width*scaleObj2frg), cvRound(detectedFaces[i].height*scaleObj2frg));
		//			Mat frgTgt=frg.clone();
		//			rectangle(frgTgt,frgRect,Scalar(127));
		Scalar faceFrg = cv::mean(frg(frgRect));
		if (faceFrg[0] > 90)
			trackerFaces.push_back(Rect(detectedFaces[i].x*decObj, detectedFaces[i].y*decObj, detectedFaces[i].width*decObj, detectedFaces[i].height*decObj));
		else
			detectedFaces.erase(detectedFaces.begin()+i);
	}

	//Decimated tracker

	// Restore foreground to proper size for tracker
	cv::resize(frg,frameForObjFrg,frameForObj.size(),0,0,CV_INTER_NN);
	IyCrCb=Irgb=frameForObj;
	Igray=frameForObjGray;
//	cout<<"tracking"<<endl;
	trackFaces(frameForObj,frameForObjFrg,detectedFaces, frameTime, _targets);

	//Full Res tracker
	// Restore foreground to proper size for tracker
	//			cv::resize(frg,frgFull,frame.size(),0,0,CV_INTER_NN);
	//			IyCrCb=Irgb=frame;
	//			Igray=frameGray;
	//			trackFaces(IyCrCb,frgFull,trackerFaces, frameCounter, targets);
//		ticks.facetrack+=cv::getTickCount()-start_ticks;

//	cout<<"recognizing"<<endl;
#ifndef _WIN32
	recogniser.recogniseExpressions(frameGray, _targets, decObj); // Only Linux binary version is available
#endif //
	frameCounter++;
	return 0;
}

int FaceProcessing::showFrame(const cv::Mat& camFrame, const std::string& dispStr, const std::string& title)
{
	cv::Mat dispFrame=camFrame.clone();
	cv::namedWindow(title,CV_WINDOW_AUTOSIZE);
	cv::putText(dispFrame, dispStr, cv::Point(0,25), CV_FONT_HERSHEY_SIMPLEX, 0.8, cv::Scalar(200), 3, 8);
	cv::imshow(title, dispFrame);
	return cv::waitKey(10);
}

int FaceProcessing::drawTargets(cv::Mat& camFrame, int64 frameTime)
{
	for (int i=0; i < (int)_targets.size(); i++)
		drawTarget(camFrame, _targets, i, frameTime /*for drawing target distance*/, decObj);
	return 0;
}

vector<targetStruct> FaceProcessing::getTargets()
{
	return _targets;
}

double FaceProcessing::getFrameAvg()
{
	return frameAvg;
}
