/*=================================================================================
Basic explanation:

Authors: Daniel Denkovski (UKIM) (danield@feit.ukim.edu.mk)

Revision History:
    25.08.2014, ver 0.1
====================================================================================*/

#pragma once

#ifndef ISENSORDEVICES_H
#define ISENSORDEVICES_H

#include "DGWdefs.h"
#include "IDeviceGateway.h"

class IDeviceGateway;

class ISensorDevices
{
public:
	virtual int initializeDevice(vector<moteDetails> sensorMotes) {return 0;};
	
	virtual int registerToDGW(IDeviceGateway *dgw) = 0;

	virtual int deregisterFromDGW() = 0;

	virtual int addSensorMote(moteDetails sensorMote) = 0;

	virtual int deleteSensorMote(string sensorMoteID) = 0;

	virtual int startMeasuring (string sensorMoteID, float reportPeriod) {return 0;};

	virtual int reconfigure (string sensorMoteID, float reportPeriod) {return 0;};

	virtual int stopMeasuring (string sensorMoteID) {return 0;};

	virtual int pushMeasurements (string sensorMoteID, std::vector<measurement> measurements) {return 0;};

	virtual int getMeasurements (string sensorMoteID) {return 0;};

	virtual std::vector<sensor> getSensors (string sensorMoteID) = 0;

	virtual moteInfo getSensorMoteInfo (string sensorMoteID) = 0;

	virtual float getRate(string sensorMoteID) = 0;

	virtual vector<moteDetails> getAllSensorMotesDetails() = 0;

	virtual int setConfigurationFromFile(string fileName) = 0;

	virtual ~ISensorDevices() = 0;
};

#endif
