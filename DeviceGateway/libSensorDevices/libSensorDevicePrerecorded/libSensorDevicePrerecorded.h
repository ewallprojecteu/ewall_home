/*=================================================================================
Basic explanation:

Authors: Daniel Denkovski (UKIM) (danield@feit.ukim.edu.mk)

Revision History:
    26.08.2014, ver 0.1
====================================================================================*/

#ifndef LIBSENSORDEVICEPRERECORDED_H
#define LIBSENSORDEVICEPRERECORDED_H

#include "libSensorDevices.h"
#include <fstream>

class SensorMotePrerecorded: public SensorDevices
{
public:
	int initializeDevice(vector<moteDetails> sensorMotes);

	~SensorMotePrerecorded();
	
	SensorMotePrerecorded();

	int startMeasuring (string sensorMoteID, float reportPeriod);

	int reconfigure (string sensorMoteID, float reportPeriod);

	int stopMeasuring (string sensorMoteID);

	std::vector<measurement> getMeasurementsPR (string sensorMoteID);

	int openDataFile(string fileName);
	
	int closeDataFile();

private:
	ifstream dataFile;
};

#endif
