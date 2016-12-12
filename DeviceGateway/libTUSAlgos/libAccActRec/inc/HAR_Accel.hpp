#ifndef HAR_ACCEL_H
#define HAR_ACCEL_H

#include "AR.h"
#include "svm.h"
#include <vector>
#include <string>
#include <stdlib.h>
#include <opencv2/opencv.hpp>

#include "cTUSAccActRec.hpp"

/*******************************************/
//Don not edit anything below this line!
/*******************************************/

/*******************************************/
using namespace std;
/*******************************************/

/*******************************************/
#if !defined(AR_ORDER)
	#define AR_ORDER (25)														//Default AR order
#endif
#define ALPHA (0.01)															//For roll angle calculation*/
#define SVM_FEAT_MIN (-1.0)														//Minumul feature value
#define SVM_FEAT_MAX (1.0)														//Maximum feature value
#define Malloc(type, n) (type *)malloc((n)*sizeof(type))
/*******************************************/

/*******************************************/
//typedef enum																	//Physical activities
//{
//	NODATA = 0,
//	RESTING, 
//	WALKING, 
//	EXCERCISING, 
//	RUNNING, 
//}Activity_t;
/*******************************************/

/*******************************************/
class HARAccel																	//HARAccel class
{
	public:
		//Description: Class constructor
		//Arguments: signalLen - the length of the input signal, SVMModelFiles - the SVM model files
		//Return: None
		HARAccel(unsigned int signalLen, vector<string> SVMModelFiles);
		//Description: Class destructor
		//Arguments: None
		//Return: None
		~HARAccel();
		//Description: Classifies the the acceleration data into a given activity
		//Arguments: const double *accX - acceleration data in x-axis, const double *accY - acceleration data in x-axis,
		//const double *accZ - acceleration data in z-axis
		//Return: The activity
		Activity_t Classify(const double *accX, const double *accY, \
		const double *accZ);													//Classification
	protected:
	private:
		unsigned int signalLength;												//The length of the analyzed signal
		double *featVector;														//Feature vector
		unsigned int featVectorSize;											//Feature vector size
		AutoRegressiveModel *ARModel;											//AR object pointer	
		cv::Mat KSVMScaleRange;
		svm_node *SVMNode;														//A SVM node pointer
		svm_model *SVMModel;													//The SVM model	ointsr
		//Description: Calculates the mean value
		//Arguments: const double *Signal - the acceleration data in a given axis.
		//Return: The mean value		
		double Mean(const double *Signal);
		//Description: Calculates the signal magnitude area
		//Arguments: const double *Signal - the acceleration data in a given axis, Mu - the measn value of the acceleration adata
		//Return: The signal magnitude area
		double SMA(const double *Signal, double Mu);
		//Description: Builds the feature vector (*FeatVector) from the acceleration data
		//Arguments: const double *accX - acceleration data in x-axis, const double *accY - acceleration data in x-axis,
		//const double *accZ - acceleration data in z-axis
		//Return: None
		void FeatVectorBuild(const double *accX, const double *accY, \
		const double *accZ);															
		//Description: Scales the elements in feature vector (*FeatVector) to be in range (-1; 1) 
		//Arguments: None
		//Return: None													
		void FeatVectorScale(void);	
};
/*******************************************/

#endif
