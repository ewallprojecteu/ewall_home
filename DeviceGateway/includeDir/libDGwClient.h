/*=================================================================================
Basic explanation:

Authors: Daniel Denkovski (UKIM) (danield@feit.ukim.edu.mk)

Revision History:
    22.09.2014, ver 0.1
====================================================================================*/

#ifndef LIBDGWCLIENT_H
#define LIBDGWCLIENT_H

#include "IDGwClient.h"
#include "IDGwSockets.h"

class DGwClient: public IDGwClient
{
public:
	int initializeDGwClient();

	DGwClient();
	
	~DGwClient();

	int startStream (string avDeviceID, streamConfig *conf);

	int reconfigureStream (string avDeviceID, streamConfig *conf);

	int stopStream (string avDeviceID, short streamtype);

	int rotateAVDevice (string avDeviceID, double angle);

	int pushFrame (string avDeviceID, void *frameData, string frameConfig) {return 0;};

	//vector<uint8_t> getLastFrame (string avDeviceID);

	string getConfiguration (string avDeviceID);

	string getAVDeviceInfo (string avDeviceID);
	
	string getAllAVDevicesInfo();
	
	int startMeasuring (string sensorMoteID, float rate);

	int reconfigure (string sensorMoteID, float rate);

	int stopMeasuring (string sensorMoteID);

	int pushMeasurements (string sensorMoteID, string measurements) {return 0;};

	string getMeasurements (string sensorMoteID);

	string getSensors (string sensorMoteID);

	string getSensorMoteInfo (string sensorMoteID);
	
	string getAllSensorDevicesInfo();

	float getRate(string sensorMoteID);

private:
	string sensorMotesJson;
	string avDevicesJson;
	IDGwSockets *sockComm;
};

#endif
