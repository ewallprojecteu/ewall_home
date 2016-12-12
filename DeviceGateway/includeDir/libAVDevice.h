/*=================================================================================
Basic explanation:

Authors: Daniel Denkovski (UKIM) (danield@feit.ukim.edu.mk)

Revision History:
    25.08.2014, ver 0.1
====================================================================================*/

#ifndef LIBAVDEVICE_H
#define LIBAVDEVICE_H

#include "IAVDevices.h"


class AVDevice: public IAVDevice
{
public:
	int initialize(avDeviceInfo devInfo);

	~AVDevice();
	
	AVDevice();
	
	int registerToDGW(IDeviceGateway *dgw);

	int deregisterFromDGW();

	int pushFrame (void *frame, streamConfig streamFormat, std::string timestamp);

	inline streamConfig getConfiguration () { return currentConfiguration;};

	inline avDeviceInfo getAVDeviceInfo () { return deviceInfo;};

	int setConfigurationFromFile(string fileName);

	avDeviceInfo deviceInfo;

	streamConfig currentConfiguration; //the current configuration of the stream;

	IDeviceGateway *devGW;

private:

};

#endif
