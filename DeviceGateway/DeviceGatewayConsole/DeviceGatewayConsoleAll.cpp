/*=================================================================================
Basic explanation:

    DeviceGateway testing console

Authors: Daniel Denkovski (UKIM) (danield@feit.ukim.edu.mk)

Revision History:
    26.08.2014, ver 0.1
    18.10.2014, ver 0.2
====================================================================================*/

#ifndef WASPMOTES //ARDUINO
#include "libSensorDeviceArduino.h"
#else //WASPMOTE
#include "libSensorDeviceWaspMote.h"
#endif
#include "libAVKinect.h"
#include "libDeviceGateway.h"
#include "libSensorDeviceBlueZ.h"
#include <stdlib.h>

int main(int argc, char* argv[])
{
	if (argc > 1)
	{
		int number = atoi(argv[1]);
		cout << "number = " << number << endl;
		IDeviceGateway *devGW = getDeviceGateway();
		if (!devGW)
		{
			cout << "Unable to get DeviceGateway handler\n";
			return -1;
		}	

		int result = devGW->initializeDeviceGW();
		if (result) cout << "DeviceGateway initialized\n";
		else cout << "DeviceGateway failed to initialize\n";

		if (number & 1)
		{
#ifndef WASPMOTES //ARDUINO
			cout << "Arduinos in this configuration\n";
			string configFileArduino = "motesArduino.cfg";
			SensorDevicesArduino *sdeviceArduino = new SensorDevicesArduino();

			if (sdeviceArduino->setConfigurationFromFile(configFileArduino))
			{			
				sdeviceArduino->registerToDGW(devGW);
				devGW->connectSensorDevices((ISensorDevices*)sdeviceArduino);	
			}
			else
			{	
				cout << "No configuration details in the provided file or file does not exist\n";
			}
#else //WASPMOTE
			cout << "Waspmotes in this configuration\n";
			string configFileWasp = "motes.cfg";
			SensorDevicesWaspMote *sdeviceWaspmote = new SensorDevicesWaspMote();

			if (sdeviceWaspmote->setConfigurationFromFile(configFileWasp))
			{			
				sdeviceWaspmote->registerToDGW(devGW);
				devGW->connectSensorDevices((ISensorDevices*)sdeviceWaspmote);	
			}
			else
			{	
				cout << "No configuration details in the provided file or file does not exist\n";
			}
#endif
		}

		if (number & 2)
		{
			SensorDevicesBlueZ *sdeviceBlueZ = new SensorDevicesBlueZ();

			sdeviceBlueZ->registerToDGW(devGW);
			devGW->connectSensorDevices((ISensorDevices*)sdeviceBlueZ);	

			vector<moteDetails> motes;
			sdeviceBlueZ->initializeDevice(motes);
		}

		if (number & 4)
		{
			string configFileKinect = "kinect.cfg";
			AVKinect *avdev = new AVKinect();
			if (avdev->setConfigurationFromFile(configFileKinect))
			{			
				avdev->registerToDGW(devGW);
				devGW->connectAVDevice((IAVDevice*)avdev);	
			}

			string kinectName = "kinect";

			frameResolution res = frameResolution(640,480);
			streamConfig conf1 = streamConfig(res,1,25,1,1,1);
			result = devGW->startStream(kinectName, &conf1);
			if (result) cout << "Kinect started RGB streaming\n";
			else cout << "Kinect did not start RBG streaming\n";

			streamConfig conf2 = streamConfig(res,1,25,1,2,2);
			result = devGW->startStream(kinectName, &conf2);
			if (result) cout << "Kinect started depth streaming\n";
			else cout << "Kinect did not start depth streaming\n";
		}

		while (1)
		{	
			int input;
			cout << "0 to quit" << endl;
			cin >> input;
			if (input == 0)
			{
				break;
			}
		}
		return 0;
	}
}

