/*=================================================================================
Basic explanation:

Authors: Daniel Denkovski (UKIM) (danield@feit.ukim.edu.mk)

Revision History:
    25.08.2014, ver 0.1
====================================================================================*/

#pragma once

#ifndef IAVDEVICES_H
#define IAVDEVICES_H

#include "DGWdefs.h"
#include "IDeviceGateway.h"

class IDeviceGateway;

class IAVDevice
{
public:
	virtual int initializeDevice(avDeviceInfo devInfo) = 0;
	
	virtual int registerToDGW(IDeviceGateway *dgw) = 0;

	virtual int deregisterFromDGW() = 0;

	virtual int startStream (streamConfig *conf) {return 0;};

	virtual int reconfigureStream (streamConfig *conf) {return 0;};

	virtual int stopStream (short streamtype) {return 0;};

	virtual int rotateAVDevice (double angle) {return 0;};

	virtual int pushFrame (void *frame, streamConfig streamFormat, std::string timestamp) {return 0;};

	virtual streamConfig getConfiguration () = 0; //{ return currentConfiguration;};

	virtual avDeviceInfo getAVDeviceInfo () = 0; //{ return deviceInfo;};

	virtual ~IAVDevice() = 0;
};

#endif
