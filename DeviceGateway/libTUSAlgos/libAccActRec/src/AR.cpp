#include "AR.h"
#include <new>

#define auto

/*******************************************/
AutoRegressiveModel::AutoRegressiveModel(unsigned int signalLen, unsigned int Ord)
{
	if(signalLen < 1)															//Check for correct signal length
	{
		throw "[HAR]:Incorrect input data length.";
	}
	if((Ord + 1) > signalLen)													//Check for correct AR order
	{
		throw "[HAR]:Incorrect AR order.";
	}
	signalLength = signalLen;													//Signal length
	Order = Ord;																//AR Order
	try
	{
		Coeffs = new double[Order + 1];											//Memory allocation
		eB = new double[signalLength];											//Memory allocation
		eF = new double[signalLength];											//Memory allocation
	}
	catch(std::bad_alloc& ba)
	{
		throw "[HAR]:Memory allocation error.";
	}
}
/*******************************************/

/*******************************************/
AutoRegressiveModel::~AutoRegressiveModel(void)
{
	if(Coeffs)
	{
		delete[] Coeffs;														//Delete the allocated memory													
	}
	if(eF)
	{
		delete[] eB;															//Delete the allocated memory
	}
	if(eF)
	{
		delete[] eF;															//Delete the allocated memory
	}
}
/*******************************************/

/*******************************************/
void AutoRegressiveModel::Calculate(const double *Signal)
{
	auto unsigned int i, j;														//Counters
	auto double D = 0.0;														//D
	auto double Mu;																//mu
	auto double Tmp1, Tmp2;														//Auxiliary
	for(i = 0; i < signalLength; i ++)
	{
		eB[i] = Signal[i];														//Initialeze eB
		eF[i] = Signal[i];														//Initialize eF
		D += 2.0 * Signal[i] * Signal[i];										//Initialize d
	}
	D -= Signal[0] * Signal[0] + \
	Signal[signalLength - 1] * Signal[signalLength - 1];						//Initialize d
	if(D == 0.0)
	{
		D = ZERO_D;																//Against division by zero
	}
	Coeffs[0] = 1.0;															//The first coefficient is always one
	for(i = 1; i < Order + 1; i ++)
	{
		Coeffs[i] = 0.0;														//Initialize the rest of coefficients
	}
	for(i = 0; i < Order; i ++)													//Burg's recursion
	{
		Mu = 0.0;																//Initialize mu
		for (j = 0; j < signalLength - i - 1; j ++)
		{
			Mu += eF[j + i + 1] * eB[j];										//Calculate mu
		}
		Mu *= -2.0 / D; 														//Calculate mu
		for (j = 0; j <= ( i + 1 ) / 2; j ++)
		{
			Tmp1 = Coeffs[j] + Mu * Coeffs[i + 1 - j];
			Tmp2 = Coeffs[i + 1 - j] + Mu * Coeffs[j];
			Coeffs[j] = Tmp1;													//Update the coefficients
			Coeffs[i + 1 - j] = Tmp2;											//Update the coefficients
		}			 
		for(j = 0; j < signalLength - i - 1; j ++)
		{
			Tmp1 = eF[j + i + 1] + Mu * eB[j];
			Tmp2 = eB[j] + Mu * eF[j + i + 1];
			eF[j + i + 1] = Tmp1;												//Update eF
			eB[j] = Tmp2;														//Update eB
		}		
		D = (1.0 - Mu * Mu) * D - eF[i + 1] * eF[i + 1] - \
		eB[signalLength - i - 2] * eB[signalLength - i - 2];					//Update D
		if(D == 0.0)
		{
			D = ZERO_D;															//Against division by zero
		}
	}
}
/*******************************************/

/*******************************************/
void AutoRegressiveModel::Get(double *Cfs)
{
	auto unsigned int i;
	for(i = 0; i < Order; i ++)
	{
		*Cfs ++= Coeffs[i + 1];													//Copy
	}
}
/*******************************************/
