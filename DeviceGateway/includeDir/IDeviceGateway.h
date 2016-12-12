/*=================================================================================
Basic explanation:

Authors: Daniel Denkovski (UKIM) (danield@feit.ukim.edu.mk)

Revision History:
    25.08.2014, ver 0.1
====================================================================================*/

#pragma once

#ifndef IDEVICEGATEWAY_H
#define IDEVICEGATEWAY_H

#include "DGWdefs.h"
#include "IAVDevices.h"
#include "ISensorDevices.h"

class IAVDevice;
class ISensorDevices;

class IDeviceGateway
{
public:
	virtual int initializeDeviceGW() = 0;
	
	virtual int connectSensorDevices(ISensorDevices *sDevice) = 0;

	virtual int disconnectSensorDevices(ISensorDevices *sDevice) = 0;

	virtual int connectAVDevice(IAVDevice *avdev) = 0;

	virtual int disconnectAVDevice(string avDeviceID) = 0;

	virtual int startStream(string avDeviceID, streamConfig *conf) = 0;

	virtual int reconfigureStream(string avDeviceID, streamConfig *conf) = 0;

	virtual int stopStream(string avDeviceID, short streamtype) = 0;

	virtual int rotateAVDevice(string avDeviceID, double angle) = 0;

	virtual int pushFrame(string avDeviceID, void *frame, streamConfig streamFormat, string timestamp) = 0;

	//virtual vector<uint8_t> getLastFrame(string avDeviceID) {return 0;};

	virtual streamConfig getConfiguration(string avDeviceID) = 0;

	virtual avDeviceInfo getAVDeviceInfo(string avDeviceID) = 0;

	virtual vector<avDeviceInfo> getAllAVDevicesInfo() = 0;
	
	virtual int startMeasuring(string sensorMoteID, float rate) = 0;

	virtual int reconfigure(string sensorMoteID, float rate) = 0;

	virtual int stopMeasuring(string sensorMoteID) = 0;

	virtual int pushMeasurements(string sensorMoteID, vector<measurement> measurements) = 0;

	virtual int getMeasurements(string sensorMoteID) = 0;

	virtual vector<sensor> getSensors(string sensorMoteID) = 0;

	virtual vector<moteDetails> getAllSensorDevicesInfo() = 0;

	virtual moteInfo getSensorMoteInfo(string sensorMoteID) = 0;

	virtual float getRate(string sensorMoteID) = 0;

	virtual ~IDeviceGateway() = 0;

/*private:
	vector<ISensorMote*> sensorMotes;
	vector<IAVDevice*> avDevices;*/
};

#ifdef __cplusplus
extern "C" {
#endif
//get handler to DeviceGateway
IDeviceGateway *getDeviceGateway();
//release DeviceGateway
void releaseDeviceGateway(const IDeviceGateway *pDeviceGateway);
#ifdef __cplusplus
}
#endif

#endif
