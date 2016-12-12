#ifndef TRACK_H
#define TRACK_H

/*******************************************/
class Track																		//Track class
{
	public:
		//Description: Class constructor
		//Arguments: nStates - the number of possible states, buffLen - the length of the tracking bufer
		//Return: None
		Track(unsigned int nStates, unsigned int buffLen);
		//Description: Class destructor
		//Arguments: None
		//Return: None
		~Track();
		//Description: Resets the tracking filter
		//Arguments: None
		//Return: None
		void Reset(void);
		//Description: Tracking filter
		//Arguments: newData - the new data
		//Return: The output data
		unsigned int Filter(unsigned int newData);
	protected:		
	private:
		unsigned int numStates;													//The number of states
		unsigned int buffLength;												//The buffer length
		unsigned int currentBuffLength;											//The current buffer length	
		unsigned int *buff;														//The tracking buffer
		unsigned int *BuffPtr;													//Input pointer of the buffer
		unsigned int *n;														//The histogram frequencies
};
/*******************************************/

#endif
