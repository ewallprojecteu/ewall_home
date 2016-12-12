#include "HAR_Track.hpp"
#include <new>

#define auto

/*******************************************/
Track::Track(unsigned int nStates, unsigned int buffLen)
{
	numStates = nStates;														//Set the number of states
	try
	{
		n = new unsigned int[numStates];										//Memory allocation
	}
	catch(std::bad_alloc& ba)
	{
		throw "[HAR]:Memory allocation error.";
	}
	buffLength = buffLen;														//Set the length of the buffer
	try
	{
		buff = new unsigned int[buffLength];									//Memory allocation
	}
	catch(std::bad_alloc& ba)
	{
		throw "[HAR]:Memory allocation error.";
	}
	Reset();																	//Reset the tracking filetr	
}
/*******************************************/

/*******************************************/
Track::~Track(void)
{
	if(buff)
	{
		delete[] buff;															//Delete the allocated memory
	}
	if(n)
	{
		delete[] n;																//Delete the allocated memory
	}
}
/*******************************************/

/*******************************************/
void Track::Reset(void)
{
	currentBuffLength = 0;														//Reset the buffer length
	BuffPtr = &buff[0];															//Reset the pointer
}
/*******************************************/

/*******************************************/
unsigned int Track::Filter(unsigned int newData)
{
	auto unsigned int i, j;														//Counters
	unsigned int Out;															//Output
	*BuffPtr = newData;
	if(BuffPtr >= &buff[buffLength - 1])
	{
		BuffPtr = &buff[0];														//Wrap around	
	}
	else
	{
		BuffPtr ++;																//Next
	}
	if(currentBuffLength < buffLength)
	{
		currentBuffLength ++;													//Increment the current length
	}
	if(currentBuffLength < 3)
	{
		return (newData);														//Return the new element
	}
	for(j = 0; j < numStates; j ++)
	{
		n[j] = 0;																//Reset all frequencies
	}
	for(i = 0; i < currentBuffLength; i ++)										//Every element in the buffer
	{
		for(j = 0; j < numStates; j ++)											//Every state
		{
			if(buff[i] == j)
			{
				n[j] ++;														//Increment the bin in the histogram
			}
		}
	}
	Out = 0;																	//Clear the maximum 
	for(j = 0; j < numStates; j ++)
	{
		if(n[j] > Out)
		{
			Out = n[j];															//Find the maximal bin
		}
	}
	for(j = 0; j < numStates - 1; j ++)
	{
		if(n[j] == n[j + 1])													//If there are two or more equal bins 
		{
			return (newData);													//Return the new element	
		}
	}
	return (Out);																//Output
}
/*******************************************/
