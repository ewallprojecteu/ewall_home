#ifndef AR_H
#define AR_H

/*******************************************/
//Don not edit anything below this line!
/*******************************************/

/*******************************************/
#define ZERO_D (1e-18)															//Smallest value of D (against division by zero)
/*******************************************/

/*******************************************/
class AutoRegressiveModel														//AR model class
{
	public:
		//Description: Class constructor
		//Arguments: signalLen - the length of the input signal, Ord - the AR order
		//Return: None
		AutoRegressiveModel(unsigned int signalLen, unsigned int Ord);
		//Description: Class destructor
		//Arguments: None
		//Return: None
		~AutoRegressiveModel();													
		//Description: Calculates the AR coefficients (*Coeffs)
		//Arguments: *Signal - the analyzed signal
		//Return: None
		void Calculate(const double *Signal);
		//Description: Copies the AR coefficients (starting from the second) into the input array. Must be predecessed by void Calculate(const double *Signal);
		//Arguments: *Cfs - array that will hold a copy of the AR coefficients (starting from the second)
		//Return: None
		void Get(double *Cfs);
	protected:
	private:
		unsigned int signalLength;												//The length of the analyzed signal
		unsigned int Order;														//AR order
		double *Coeffs;															//AR coefficients
		double *eB;																//Auxiliary
		double *eF;																//Auxiliary
};
/*******************************************/

#endif
