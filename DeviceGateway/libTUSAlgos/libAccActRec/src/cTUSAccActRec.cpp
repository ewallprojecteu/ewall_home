#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>

#include "cTUSAccActRec.hpp"
#include "HAR_Accel.hpp"
#include "HAR_Track.hpp"

using namespace std;


/**
* @ brief 
*	Class constructor
* @param 
* @return 
*/
cTUSAccActRec::cTUSAccActRec(vector<string> models, int windowlen)
{
    // State vars init
   
    // Allocate Buffers
	HAR=new HARAccel(windowlen, models);
	track = new Track(5,5); 
	xx = new double[windowlen];
	yy = new double[windowlen];
	zz = new double[windowlen];

	CurrentActivity = NODATA;
	BuffState = 0;
	this->windowlen = windowlen;
}		

/**
* @ brief 
*	Class constructor
* @param 
* @return 
*/
cTUSAccActRec::~cTUSAccActRec(void)
{
    // Clear context
	delete (HARAccel*)HAR;
	delete (Track*)track; 
	delete xx;
	delete yy;
	delete zz;
}	

/**
* @ brief 
*	Returnig the current activity 
* @param 
* @return 
*/
Activity_t cTUSAccActRec::GetActivity(void)
{
	return (Activity_t)CurrentActivity;
}

/**
* @ brief 
*	Function applying the recognition algorithm
* @param 
* @return 
*/
Activity_t cTUSAccActRec::Apply(double *accX, double *accY,  double *accZ, unsigned int datalen)
{
    
	Activity_t Activity = NODATA;
	int k = 0;

	while(datalen){	

		xx[BuffState] = accX[k];
		yy[BuffState] = accY[k];
		zz[BuffState] = accZ[k];
		BuffState++; k++; datalen--;

		if(BuffState == windowlen) {
			
			BuffState = 0;

			// clasify the current activity
			Activity=((HARAccel*) HAR)->Classify(accX, accY, accZ);
	
			// filter temporal variations
			Activity = (Activity_t)((Track*)track)->Filter(Activity);

			CurrentActivity = (int)Activity;
		}
	}
    return Activity;
}
