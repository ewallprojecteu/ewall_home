/*=================================================================================
Basic explanation:

    Device Gateway cpp code

Authors: Daniel Denkovski (UKIM) (danield@feit.ukim.edu.mk)

Revision History:
    25.08.2014, ver 0.1
====================================================================================*/

#include "libAVDeviceExample.h"

AVDeviceExample::~AVDeviceExample()
{
}

AVDeviceExample::AVDeviceExample()
{
}

int AVDeviceExample::initializeDevice(avDeviceInfo devInfo)
{
	initialize(devInfo);
}
	
int AVDeviceExample::startStream (streamConfig *conf)
{
	cout << "AVDevices: AVDeviceExample with ID = " << deviceInfo.deviceID << " started streaming with rate = " << conf->rate << "Hz and resolution of " << conf->resolution.width << "x" << conf->resolution.height << "!\n";
	return 1;
}

int AVDeviceExample::reconfigureStream (streamConfig *conf)
{
	cout << "AVDevices: AVDeviceExample with ID = " << deviceInfo.deviceID << " reconfigured streaming to rate = " << conf->rate << "Hz and resolution of " << conf->resolution.width << "x" << conf->resolution.height << "!\n";
	return 1;
}

int AVDeviceExample::stopStream (short streamtype)
{
	cout << "AVDevices: AVDeviceExample with ID = " << deviceInfo.deviceID << " stopped streaming!\n";
	return 1;
}

int AVDeviceExample::rotateAVDevice (double angle)
{
	cout << "AVDevices: AVDeviceExample with ID = " << deviceInfo.deviceID << " rotated for " << angle << " degrees!\n";
	return 1;
}

int AVDeviceExample::pushFrame (void *frame, streamConfig streamFormat, string timestamp)
{
	cout << "AVDevices: AVDeviceExample with ID = " << deviceInfo.deviceID << " pushed frame to DeviceGateway!\n";
	return devGW->pushFrame(deviceInfo.deviceID, frame, streamFormat, timestamp);
}
