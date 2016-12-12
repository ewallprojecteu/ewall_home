/*=================================================================================
Basic explanation:

Authors: Daniel Denkovski (UKIM) (danield@feit.ukim.edu.mk)

Revision History:
    26.08.2014, ver 0.1
====================================================================================*/

#ifndef LIBSENSORDEVICEWASPMOTE_H
#define LIBSENSORDEVICEWASPMOTE_H

#include "libSensorDevices.h"

class SensorDevicesWaspMote: public SensorDevices
{
public:
	int initializeDevice(vector<moteDetails> sensorMotes);

	~SensorDevicesWaspMote();
	
	SensorDevicesWaspMote();

	inline int getConnID() { return connID; };

private:
	int connID;
	
};

#endif
