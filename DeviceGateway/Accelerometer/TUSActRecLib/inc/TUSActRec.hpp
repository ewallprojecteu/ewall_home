
#ifndef __TUSACTREC__
#define __TUSACTREC__

#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <opencv2/opencv.hpp>

using namespace std;

// Emotions
typedef enum{
    NODATA = 0,
    RESTING, 
    WALKING, 
    EXCERCISING, 
    RUNING, 
}Activity_t;


/*! \class TUSActRec
    \brief Encapsulating the activity type recognition algorithm.

    A more detailed class description.
*/

class cTUSActRec{

public:
	/**
	* @ brief 
	*	Class constructor
	* @param 
	* @return 
	*/
    cTUSActRec(vector<string> models, int windowlen);
    
	/**
	* @ brief 
	*	Class constructor
	* @param 
	* @return 
	*/
    cTUSActRec(char *models, char *path2DB);

	/**
	* @ brief 
	*	Class destructor
	* @param 
	* @return 
	*/
    ~cTUSActRec(void);
        
	/**
	* @ brief 
	*	Function applying the recognition algorithm
	* @param 
	* @return 
	*/
	Activity_t Apply(double *accX, double *accY,  double *accZ, unsigned int datalen);

	/**
	* @ brief 
	*	Returnig the current activity 
	* @param 
	* @return 
	*/
	Activity_t GetActivity(void);

private:
    void *HAR;
    void *track;
	double *xx, *yy, *zz;
	unsigned int CurrentActivity,BuffState,windowlen;
};


#endif
