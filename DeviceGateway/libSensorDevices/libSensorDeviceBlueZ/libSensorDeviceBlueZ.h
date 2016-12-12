/*=================================================================================
Basic explanation:

Authors: Daniel Denkovski (UKIM) (danield@feit.ukim.edu.mk)

Revision History:
    26.08.2014, ver 0.1
====================================================================================*/

#ifndef LIBSENSORDEVICEBLUEZ_H
#define LIBSENSORDEVICEBLUEZ_H

#include "libSensorDevices.h"

class SensorDevicesBlueZ: public SensorDevices
{
public:
	int initializeDevice(vector<moteDetails> sensorMotes);

	~SensorDevicesBlueZ();
	
	SensorDevicesBlueZ();

	int startMeasuring (string sensorMoteID, float reportPeriod);

	int reconfigure (string sensorMoteID, float reportPeriod);

	int stopMeasuring (string sensorMoteID);

	int getMeasurements (string sensorMoteID);

	int stopBlueZ();
};

#endif
