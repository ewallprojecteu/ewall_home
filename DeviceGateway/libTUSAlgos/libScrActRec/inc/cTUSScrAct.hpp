
#ifndef __CTUSSCRACT__
#define __CTUSSCRACT__

#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include "DepthImagePersonDetection.hpp"

using namespace std;
#include <opencv2/opencv.hpp>

typedef enum{
	SCREEEN_PASSIVE = 0,
	SCREEEN_ACTIVE,
	SCREEEN_NOT_READY
}ScreenState_t;

/*! \class cTUSScrAct
    \brief Encapsulating the main screen activation algorithm.

    A more detailed class description.
*/

class cTUSScrAct{

public:
	/**
	* @ brief 
	*	Class constructor
	* @param 
	* @return 
	*/
    cTUSScrAct(void);
    

	/**
	* @ brief 
	*	Class destructor
	* @param 
	* @return 
	*/
    ~cTUSScrAct(void);
        
	/**
	* @ brief 
	*	Function applying the recognition algorithm using Depth image
	* @param 
	* @return 
	*/
   ScreenState_t Apply(IplImage *imageDepth);

	/**
	* @ brief 
	*	Function applying the recognition algorithm using IR image
	* @param 
	* @return 
	*/
   ScreenState_t Apply(cv::Mat &irMat);

private:
	void *actx;
	/**
	* @ brief 
	*	Function applying state machine for detain active state if it is in active mode for speciffic time (in ms)
	* @param 
	* @return 
	*/
	depthStatus detainActiveState(depthStatus status);
	/**
	* @ brief 
	*	Function getting representative status for certein number of frames
	* @param 
	* @return 
	*/
	depthStatus getReprState(depthStatus curStatus);
	int stBufferLength;
	depthStatus *stBuffer;
	int buffCounter;
	depthStatus curStatusDet;
	int timeThr;
	long long inRangeStartTime;
};

#endif
