/*=================================================================================
Basic explanation:

Authors: Daniel Denkovski (UKIM) (danield@feit.ukim.edu.mk)

Revision History:
    26.08.2014, ver 0.1
====================================================================================*/

#ifndef LIBSENSORDEVICES_H
#define LIBSENSORDEVICES_H

#include "ISensorDevices.h"

class SensorDevices: public ISensorDevices
{
public:
	int initialize(vector<moteDetails> sensorMotes);

	~SensorDevices();
	
	SensorDevices();

	int addSensorMote(moteDetails sensorMote);

	int deleteSensorMote(string sensorMoteID);
	
	int registerToDGW(IDeviceGateway *dgw);

	int deregisterFromDGW();

	int pushMeasurements (string sensorMoteID, vector<measurement> measurements);

	std::vector<sensor> getSensors (string sensorMoteID);

	moteInfo getSensorMoteInfo (string sensorMoteID);

	float getRate(string sensorMoteID);

	int setRate(string sensorMoteID, float repPeriod);

	vector<moteDetails> getAllSensorMotesDetails();

	int findSensorMoteByID(string sensorMoteID);

	moteDetails getSensorMoteDetails(int index);

	int setConfigurationFromFile(string fileName);

private:
	vector<moteDetails> mySensorMotes;
	vector<float> reportingPeriods; //in seconds;
	IDeviceGateway *devGW;
};

#endif
