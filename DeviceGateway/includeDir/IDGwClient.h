/*=================================================================================
Basic explanation:

Authors: Daniel Denkovski (UKIM) (danield@feit.ukim.edu.mk)

Revision History:
    22.09.2014, ver 0.1
====================================================================================*/

#pragma once

#ifndef IDGWCLIENT_H
#define IDGWCLIENT_H

#include "DGWdefs.h"

class IDGwClient
{
public:
	virtual int initializeDGwClient() = 0;

	virtual int startStream(string avDeviceID, streamConfig *conf) = 0;

	virtual int reconfigureStream(string avDeviceID, streamConfig *conf) = 0;

	virtual int stopStream(string avDeviceID, short streamtype) = 0;

	virtual int rotateAVDevice(string avDeviceID, double angle) = 0;

	virtual int pushFrame(string avDeviceID, void *frameData, string frameConfig) {return 0;};

	//virtual vector<uint8_t> getLastFrame(string avDeviceID) = 0;

	virtual string getConfiguration(string avDeviceID) = 0;

	virtual string getAVDeviceInfo(string avDeviceID) = 0;

	virtual string getAllAVDevicesInfo() = 0;
	
	virtual int startMeasuring(string sensorMoteID, float rate) = 0;

	virtual int reconfigure(string sensorMoteID, float rate) = 0;

	virtual int stopMeasuring(string sensorMoteID) = 0;

	virtual int pushMeasurements(string sensorMoteID, string measurements) {return 0;};

	virtual string getMeasurements(string sensorMoteID) = 0;

	virtual string getSensors(string sensorMoteID) = 0;

	virtual string getAllSensorDevicesInfo() = 0;

	virtual string getSensorMoteInfo(string sensorMoteID) = 0;

	virtual float getRate(string sensorMoteID) = 0;

	virtual ~IDGwClient() = 0;

};

#endif
