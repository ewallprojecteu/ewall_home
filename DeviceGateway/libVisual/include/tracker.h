#ifndef TRACKER_H
#define TRACKER_H

#include <iostream>
#include <vector>
#include "target.h"
#include "VJ.h"

#define FACE_RECTS_SIZE 5
#define USE_PF_TRACKER //Define to use PF tracker, otherwise using camshift tracker

//Include highgui to close windows in camshift tracker
#ifndef USE_PF_TRACKER
#include <opencv2/highgui/highgui.hpp>
#endif

#define TRACKER_CAMSHIFT  1
#define TRACKER_PF        2

//Tracking modalities for PF tracker
#define CUE_FACE             1
#define CUE_COLOR	        (1<<1)
#define CUE_MOTION          (1<<2)
#define CUE_FACEMOTION      (CUE_FACE  | CUE_MOTION )
#define CUE_FACECOLOR	    (CUE_FACE  | CUE_COLOR  )
#define CUE_COLORMOTION     (CUE_COLOR | CUE_MOTION )
#define CUE_FACECOLORMOTION (CUE_FACE  | CUE_COLOR | CUE_MOTION)


//#define	FACING_SPREAD			.05		// If particle spread is below this, we assume person is facing camera
#define	FACING_SPREAD			.085		// If particle spread is below this, we assume person is facing camera


void gaussianObjectModel(targetStruct *target);
void gaussianMeasurementObjectModel(targetStruct *target);
void gaussianUniformObjectModel(targetStruct *target);

int updatePFtracker(std::vector<targetStruct> & targets, int64 frameTime);
void initPFtracker(std::vector<faceDetection> faces, targetStruct *target);
void drawParticles(cv::InputOutputArray _Irgb, particle *particles, int numParticles=NP, int dec=1);
void drawState(cv::InputOutputArray _Irgb, targetStruct target, int dec=1);

void face2rects(cv::Rect faceRect, cv::Rect limit, cv::Rect out[FACE_RECTS_SIZE], double expansion, double xConstrain, double yExpand);
void face2rectsMotion(cv::Rect faceRect, cv::Rect limit, cv::Rect out[FACE_RECTS_SIZE]);

void killTargets(std::vector<targetStruct> &targets, int64 frameTime);

int initFaceTracker(int trackerType, std::string& trackerName, int modalities = 0, const std::string& cascadeName = "haarcascade_frontalface_alt.xml", const std::string& serverName = "localhost");
int trackFaces(const cv::Mat& _YCrCb, const cv::Mat& _frg, const std::vector<cv::Rect>& newFaces, int64 frameTime, std::vector<targetStruct>& targets);
void drawStatistics(cv::Mat &Irgb, int64 frameTime, cv::Rect statRect);
void clearStatistics();


#define NUM_GENDERS 2

#endif /*TRACKER_H*/