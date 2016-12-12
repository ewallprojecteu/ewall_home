/*=================================================================================
Description: Performs simple visual processing

Authors: Nikolaos Katsarakis (AIT) (nkat@ait.edu.gr)

Revision History:
    13.11.2014, ver 0.1
====================================================================================*/

#ifndef FACEPROCESING_H
#define FACEPROCESING_H

//Remove annoying Visual Studio warnings about unsafe functions
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif //_CRT_SECURE_NO_WARNINGS
#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/video/video.hpp>
#include "tracker.h"
//#include "timeticks.h"
#include "display.h"
#ifndef _WIN32
#include "ExpRecogniser.h" // Only Linux binary version is available
#endif //

class FaceProcessing
{
public:
	FaceProcessing(const std::string& cascadePath = std::string(), int faceWidth=128);
	~FaceProcessing();

	void init();//Initialize, throws exception on error, should be called at start
	int processFrame(const cv::Mat& camFrame, int64 frameTime);
	int showFrame(const cv::Mat& camFrame, const std::string& dispStr = std::string(), const std::string& title = "Face Processing");
	std::vector<targetStruct> getTargets();
	double getFrameAvg();
	int drawTargets(cv::Mat& camFrame, int64 frameTime);
	int getMinFaceSize();
	void setMinFaceSize(int NEW_MIN_FACE_SIZE );

private:
	bool initialized;
	std::vector<targetStruct> _targets;//Tracked targets
	std::string _cascadePath; //Path for face detection classifiers
	int _faceWidth; //Normalized face width
	int64 frameCounter, frgRestartFrame /*Frame when background was last restarted */;
	cv::BackgroundSubtractorMOG2 bkgSeg;
	double frameAvg; //Average frame brightness

	int64 latestTimeDetected; // time in ms of most recent face detection

	cv::CascadeClassifierFix faceCascade; // Face detection cascade
	int modalities;  // Which tracker modalities to use

	int MIN_FACE_SIZE, MAX_FACE_SIZE; // Size of faces for searching, depend on video
	int MIN_FRG_IN_FACE;
	int REPORT_STEP; // Every how many frames to report timing statistics on the console


	int	decFrg; double	scaleFrg; // Foreground scaling variables
	int	decObj; double	scaleObj; // Face scaling variables
	double scaleObj2frg; //Scale from face detections to foreground
	int faceCascadeWidth; // Width of face cascade, used for changing minfacesize
//	timeticks ticks;

	cv::Mat frameForFrg, frameForObj, frameForObjGray, frameGray;
	cv::Mat frg;

	std::vector<cv::Rect> detectedFaces, trackerFaces;
#ifndef _WIN32
	ExpRecogniser recogniser; // Only Linux binary version is available
#endif //

};

#endif //FACEPROCESING_H
