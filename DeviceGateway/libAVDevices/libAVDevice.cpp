/*=================================================================================
Basic explanation:

    Device Gateway cpp code

Authors: Daniel Denkovski (UKIM) (danield@feit.ukim.edu.mk)

Revision History:
    25.08.2014, ver 0.1
====================================================================================*/

#include "libAVDevice.h"
#include "json_spirit_dgw.h"

IAVDevice::~IAVDevice()
{
}

AVDevice::~AVDevice()
{
}

AVDevice::AVDevice()
{
}

int AVDevice::initialize(avDeviceInfo devInfo)
{
	deviceInfo = devInfo;
	cout << "AVDevices: AVDevice with ID = " << deviceInfo.deviceID << " initialized!\n";
	return 1;
}
	
int AVDevice::registerToDGW(IDeviceGateway *dgw)
{
	devGW = dgw;
	cout << "AVDevices: AVDevice with ID = " << deviceInfo.deviceID << " registered to DeviceGateway!\n";
	return 1;
}

int AVDevice::deregisterFromDGW()
{
	//delete devGW;
	devGW = NULL;
	cout << "AVDevices: AVDevice with ID = " << deviceInfo.deviceID << " deregistered from DeviceGateway!\n";
	return 1;
}

int AVDevice::pushFrame (void *frame, streamConfig streamFormat, string timestamp)
{
	cout << "AVDevices: AVDevice with ID = " << deviceInfo.deviceID << " pushed frame to DeviceGateway!\n";
	return devGW->pushFrame(deviceInfo.deviceID, frame, streamFormat, timestamp);
}

int AVDevice::setConfigurationFromFile(string fileName)
{
    ifstream moteifs(fileName.c_str());
    string jsonStr;
    std::stringstream buffer;
    buffer << moteifs.rdbuf();
    jsonStr = buffer.str();
    moteifs.close();

    cout << "Getting configuration from file\n";
    if (!jsonStr.empty())
    {
	avDeviceInfo kinectInfo;
	kinectInfo = JsonToAVDeviceInfo(jsonStr);
	deviceInfo = kinectInfo;
	initializeDevice(deviceInfo);
	return 1;
    }
    cout << "Failed to configure\n";

    return 0;  
}
