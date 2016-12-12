/*=================================================================================
Basic explanation:

    DeviceGateway testing console

Authors: Daniel Denkovski (UKIM) (danield@feit.ukim.edu.mk)

Revision History:
    26.08.2014, ver 0.1
    18.10.2014, ver 0.2
====================================================================================*/

#include "libSensorDeviceBlueZ.h"
#include "libDeviceGateway.h"
#include <unistd.h>
#include <time.h>

int main(int argc, char* argv[])
{
	IDeviceGateway *devGW = getDeviceGateway();
	if (!devGW)
	{
		cout << "Unable to get DeviceGateway handler\n";
		return -1;
	}	

	int result = devGW->initializeDeviceGW();
	if (result) cout << "DeviceGateway initialized\n";
	else cout << "DeviceGateway failed to initialize\n";
	
	string configFile = "motes.cfg";
	SensorDevicesBlueZ *sdeviceBlueZ = new SensorDevicesBlueZ();

	/*if (sdeviceBlueZ->setConfigurationFromFile(configFile))
	{*/
		sdeviceBlueZ->registerToDGW(devGW);
		devGW->connectSensorDevices((ISensorDevices*)sdeviceBlueZ);	
	/*}
	else
	{	
		cout << "No configuration details in the provided file or file does not exist\n";
	}*/

	vector<moteDetails> motes;
	sdeviceBlueZ->initializeDevice(motes);

	while (1)
	{	
		int input;
		cout << "0 to quit" << endl;
		cin >> input;
		if (input == 0)
		{
			sdeviceBlueZ->stopBlueZ();
			break;
		}
	}

	return 0;
}

