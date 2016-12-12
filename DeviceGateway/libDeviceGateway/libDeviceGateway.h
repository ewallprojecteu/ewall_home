/*=================================================================================
Basic explanation:

Authors: Daniel Denkovski (UKIM) (danield@feit.ukim.edu.mk)

Revision History:
    25.08.2014, ver 0.1
====================================================================================*/

#ifndef LIBDEVICEGATEWAY_H
#define LIBDEVICEGATEWAY_H

#include "IDeviceGateway.h"
#include "IDGwSockets.h"

class DeviceGateway: public IDeviceGateway
{
public:
	int initializeDeviceGW();

	DeviceGateway();
	
	~DeviceGateway();
	
	int connectSensorDevices(ISensorDevices *sDevice);

	int disconnectSensorDevices(ISensorDevices *sDevice);

	int connectAVDevice(IAVDevice *avdev);

	int disconnectAVDevice(string avDeviceID);

	int startStream (string avDeviceID, streamConfig *conf);

	int reconfigureStream (string avDeviceID, streamConfig *conf);

	int stopStream (string avDeviceID, short streamtype);

	int rotateAVDevice (string avDeviceID, double angle);

	int pushFrame (string avDeviceID, void *frame, streamConfig streamFormat, string timestamp);

	streamConfig getConfiguration (string avDeviceID);

	avDeviceInfo getAVDeviceInfo (string avDeviceID);
	
	vector<avDeviceInfo> getAllAVDevicesInfo();
	
	int startMeasuring (string sensorMoteID, float rate);

	int reconfigure (string sensorMoteID, float rate);

	int stopMeasuring (string sensorMoteID);

	int pushMeasurements (string sensorMoteID, vector<measurement> measurements);

	int getMeasurements (string sensorMoteID);

	vector<sensor> getSensors (string sensorMoteID);

	moteInfo getSensorMoteInfo (string sensorMoteID);
	
	vector<moteDetails> getAllSensorDevicesInfo();

	float getRate(string sensorMoteID);

	int findSensorDevicesByID(string sensorMoteID);

	int findSensorDevicesHndlByID(ISensorDevices *sDevice);

	int findAVDeviceByID(string avDeviceID);

private:
	vector<ISensorDevices*> sensorDevices;
	vector<IAVDevice*> avDevices;
	IDGwSockets *sockComm;
};

#endif
