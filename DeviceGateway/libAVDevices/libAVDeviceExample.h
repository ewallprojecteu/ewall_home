/*=================================================================================
Basic explanation:

Authors: Daniel Denkovski (UKIM) (danield@feit.ukim.edu.mk)

Revision History:
    25.08.2014, ver 0.1
====================================================================================*/

#ifndef LIBAVDEVICEEXAMPLE_H
#define LIBAVDEVICEEXAMPLE_H

#include "libAVDevice.h"

class AVDeviceExample: public AVDevice
{
public:
	int initializeDevice(avDeviceInfo devInfo);

	~AVDeviceExample();
	
	AVDeviceExample();
	
	int startStream (streamConfig *conf);

	int reconfigureStream (streamConfig *conf);

	int stopStream (short streamtype);

	int rotateAVDevice (double angle);

	int pushFrame (void *frame, streamConfig streamFormat, std::string timestamp);
};

#endif
