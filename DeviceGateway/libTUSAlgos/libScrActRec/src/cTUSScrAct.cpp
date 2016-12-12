#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

#include "cTUSScrAct.hpp"
#include "DepthImagePersonDetection.hpp"
#include "time_funcs.h"

using namespace std;


/**
* @ brief 
*	Class constructor
* @param 
* @return 
*/
cTUSScrAct::cTUSScrAct(void)
{
   
    // allocate the detection algorithm
    //actx = new DepthImagePersonDetection(1.2, 0.3, 4);
    actx = new DepthImagePersonDetection(50, 255);
    stBufferLength = 5;
    stBuffer = new depthStatus[stBufferLength];
	for(int i = 0; i < stBufferLength; i++) {
		stBuffer[i] = unknown;
	}
    buffCounter = 0;
    curStatusDet = unknown;
    // time threshold to detain active status, ms
    timeThr = 60000;
 
}

cTUSScrAct::~cTUSScrAct(void)
{
    delete [] stBuffer;
}


/**
* @ brief 
*	Function applying state machine for detain active state if it is in active mode for speciffic time (in ms)
* @param 
* @return 
*/
depthStatus cTUSScrAct::detainActiveState(depthStatus status)
{
    switch( curStatusDet )
    {
        case unknown :
        {
            curStatusDet = status;
            if (status == InRange) {
                inRangeStartTime = getMillis();
            }
        }
        break;
    
        case InRange :
        {
            if (status == InRange) {
                curStatusDet = status;
                inRangeStartTime = getMillis();
            } else { // status NotInRange or unknown
                if ((getMillis() - inRangeStartTime) >= timeThr) {
                    curStatusDet = status;
                } else {
                    curStatusDet = InRange;
                }
                
            }
        }
        break;

        case NotInRange :
        {
            curStatusDet = status;
            if (status == InRange) {
                inRangeStartTime = getMillis();
            }
        }
        break;
    }
	return curStatusDet;
}

/**
* @ brief 
*	Function getting representative status for certein number of frames
* @param
* @return 
*/
depthStatus cTUSScrAct::getReprState(depthStatus curStatus)
{
    
    stBuffer[buffCounter] = curStatus;
    
    // increase counter
	buffCounter++;

    // reset counter
	if (buffCounter == stBufferLength) {
		buffCounter = 0;
	}
    
    depthStatus firstElem = stBuffer[0];
    for (int i = 1; i< stBufferLength; i++) {
        	if (firstElem != stBuffer[i])
		return unknown;
	}
    return firstElem;
}
    
/**
* @ brief 
*	Function applying the recognition algorithm
* @param 
* @return 
*/
ScreenState_t cTUSScrAct::Apply(IplImage *imageDepth)
{
    ScreenState_t ScreenState;
   
    // get screen activity status based on depth
	depthStatus status = ((DepthImagePersonDetection *)actx)->AnalyzeDepth(imageDepth);
    
    // filter screen status, i.e. get representative status for speciffic number of frames
    depthStatus reprStatus = getReprState(status);
        
	// detain screen status if it is in active mode for speciffic time (in ms)
    depthStatus detStatus = detainActiveState(reprStatus);

    if(detStatus == unknown)
        return SCREEEN_NOT_READY;
    
    if(detStatus == InRange)
		return SCREEEN_ACTIVE;

	return SCREEEN_PASSIVE;
}

ScreenState_t cTUSScrAct::Apply(Mat &irMat)
{
    ScreenState_t ScreenState;
   
    // get screen activity status based on ir
	depthStatus status = ((DepthImagePersonDetection *)actx)->AnalyzeIr(irMat);
    
    // filter screen status, i.e. get representative status for speciffic number of frames
    depthStatus reprStatus = getReprState(status);
        
	// detain screen status if it is in active mode for speciffic time (in ms)
    depthStatus detStatus = detainActiveState(reprStatus);

    if(detStatus == unknown)
        return SCREEEN_NOT_READY;
    
    if(detStatus == InRange)
		return SCREEEN_ACTIVE;

	return SCREEEN_PASSIVE;
}

