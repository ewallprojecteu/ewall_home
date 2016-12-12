#include "HAR_Accel.hpp"
#include <math.h>
#include <new>
#include "SVM_sr.h"



#define auto
//#define USE_STATIC_SCALERANGE

/*******************************************/
HARAccel::HARAccel(unsigned int signalLen, vector<string> SVMModelFiles)
{
	string KSVMScaleData = SVMModelFiles.at(0);
    string SVMModelData = SVMModelFiles.at(1);
	signalLength = signalLen;													//Signal dataLength
	featVectorSize = 3 * AR_ORDER + 5;											//Features vector size
	try
	{
		ARModel = new AutoRegressiveModel(signalLen, AR_ORDER);					//New AR model
		featVector = new double[featVectorSize];								//Memory allocation
	}
	catch(std::bad_alloc& ba)
	{
		throw "[HAR]:Memory allocation error.";
	}


	// Load scaling for SVM
	cv::FileStorage KSVMScaleDataFile(KSVMScaleData, cv::FileStorage::READ);
	if (!KSVMScaleDataFile.isOpened())  
		throw "[HAR] ERROR: Unable to open Scale data file";
    KSVMScaleDataFile["KSVMScaleRange"] >> KSVMScaleRange;
	KSVMScaleDataFile.release();

	// Load SVM
	SVMNode = NULL;																//Null
	SVMNode = Malloc(svm_node, featVectorSize + 1);								//SVM node memory allocation
	if(SVMNode == NULL)															//Allocation error
		throw "[HAR]:Memory allocation error.";

	SVMModel = svm_load_model(SVMModelData.c_str());							//Load the SVM model		
	if(SVMModel == NULL)															//Allocation error
		throw "[HAR]:Unable to open SVM models file.";
}
/*******************************************/

/*******************************************/
HARAccel::~HARAccel(void)
{
	if(ARModel)
	{
		delete ARModel;															//Delete the allocated memory
	}
	if(featVector)
	{
		delete[] featVector;													//Delete the allocated memory
	}
	free(SVMNode);																//Free the SVM node
	svm_free_and_destroy_model(&SVMModel);										//Free and destroy the SVM model
}
/*******************************************/

/*******************************************/
Activity_t HARAccel::Classify(const double *accX, const double *accY, const double *accZ)
{
	auto unsigned int i;
	Activity_t Activity;
	FeatVectorBuild(accX, accY, accZ);											//Build the feature vector
	FeatVectorScale();															//Scale the feature vector
	for(i = 0; i < featVectorSize; i++)
	{
		SVMNode[i].index = i + 1;												//Set index
		SVMNode[i].value = featVector[i];										//Copy the feature vector
	}
	SVMNode[featVectorSize].index = -1;											//Termination
	Activity = (Activity_t)((int)svm_predict(SVMModel, SVMNode));					//The SVM classification
	return (Activity);															//Return the activity
}
//===========================================

/*******************************************/
double HARAccel::Mean(const double *Signal)
{
	auto unsigned int i;														//Counter	
	double Mu = 0.0;															//Reset
	for(i = 0; i < signalLength; i ++)
	{
		Mu += *Signal ++;														//Accumulate
	}
	Mu /= signalLength;															//Average
	return (Mu);
}
/*******************************************/

/*******************************************/
double HARAccel::SMA(const double *Signal, double Mu)
{
	auto unsigned int i;														//Counter
	auto double Tmp;															//Auxiliary
	double SMArea = 0.0;														//Clear
	for(i = 0; i < signalLength; i ++)
	{
		Tmp = Signal[i] - Mu;													//Subtract the mean
		if(Tmp < 0.0)
		{
			Tmp *= -1.0;														//Absolute value
		}
		SMArea += Tmp;															//Accumulate
	}
	SMArea /= signalLength;														//Average
	return (SMArea);
}
/*******************************************/

/*******************************************/
void HARAccel::FeatVectorBuild(const double *accX, const double *accY, const double *accZ)
{
	auto double *featVectorPtr = &featVector[0];								//Pointer to the feature vector
	auto double Mu[3];															//Means
	auto double Den;															//Denominator for inverse tangents
	ARModel->Calculate(accX);													//Calculate the AR coefficients for x data
	ARModel->Get(featVectorPtr);												//Copy the AR coefficients in feature vector
	featVectorPtr += AR_ORDER;													//Update the pointer
	ARModel->Calculate(accY);													//Calculate the AR coefficients for y data
	ARModel->Get(featVectorPtr);												//Copy the AR coefficients in feature vector
	featVectorPtr += AR_ORDER;													//Update the pointer	
	ARModel->Calculate(accZ);													//Calculate the AR coefficients for z data
	ARModel->Get(featVectorPtr);												//Copy the AR coefficients in feature vector
	featVectorPtr += AR_ORDER;													//Update the pointer	
	Mu[0] = Mean(accX);															//Calculate the mean for x data
	Mu[1] = Mean(accY);															//Calculate the mean for y data
	Mu[2] = Mean(accZ);															//Calculate the mean for z data
	*featVectorPtr ++= SMA(accX, Mu[0]);										//Add the SMA for x data
	*featVectorPtr ++= SMA(accY, Mu[1]);										//Add the SMA for y data
	*featVectorPtr ++= SMA(accZ, Mu[2]);										//Add the SMA for z data	
	Den = sqrt(Mu[2] * Mu[2] + ALPHA * Mu[0] * Mu[0]);							//Calculate the denominator
	if(Mu[2] < 0.0)
	{
		Den *= -1.0;															//Sign function	
	}
	*featVectorPtr ++= atan(Mu[1] / Den);										//Roll angle
	*featVectorPtr = atan((-1.0 * Mu[0]) / sqrt(Mu[1] * Mu[1] + \
	Mu[2] * Mu[2]));															//Pitch angle	
}
/*******************************************/

/*******************************************/
void HARAccel::FeatVectorScale(void)
{

#ifdef USE_STATIC_SCALERANGE
	auto unsigned int i;
	for(i = 0; i < featVectorSize; i++)
	{
		featVector[i] = SVM_FEAT_MIN + (SVM_FEAT_MAX - SVM_FEAT_MIN) * \
		(featVector[i] - sr[i][0]) / (sr[i][1]-sr[i][0]);						//Scale the feature vector
	}
#else
	
	int scaleSize = KSVMScaleRange.rows;
	float y_min, y_max, value;
    
	// for each element of the feature
	for (unsigned int i = 0; i < featVectorSize; i++) {
		y_min = KSVMScaleRange.at<float>(i,0);
		y_max = KSVMScaleRange.at<float>(i,1);
		value = featVector[i];
		if(value == y_min)
			value = SVM_FEAT_MIN;
		else if(value == y_max)
			value = SVM_FEAT_MAX;
		else 
            value = SVM_FEAT_MIN + (SVM_FEAT_MAX-SVM_FEAT_MIN) *(value - y_min)/(y_max-y_min);
		featVector[i] = value;
	}

#endif
}
/*******************************************/
