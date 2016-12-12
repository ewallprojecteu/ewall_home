/*=================================================================================
Basic explanation:

Authors: Daniel Denkovski (UKIM) (danield@feit.ukim.edu.mk)

Revision History:
    26.08.2014, ver 0.1
====================================================================================*/

#ifndef LIBSENSORDEVICEARDUINO_H
#define LIBSENSORDEVICEARDUINO_H

#include "libSensorDevices.h"

class SensorDevicesArduino: public SensorDevices
{
public:
	int initializeDevice(vector<moteDetails> sensorMotes);

	~SensorDevicesArduino();
	
	SensorDevicesArduino();

	inline int getConnID() { return connID; }

private:
	int connID;
};

#endif
