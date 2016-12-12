/*=================================================================================
Basic explanation:

    DeviceGateway testing console

Authors: Daniel Denkovski (UKIM) (danield@feit.ukim.edu.mk)

Revision History:
    26.08.2014, ver 0.1
====================================================================================*/

#include "libSensorDeviceWaspMote.h"
#include "libAVDeviceExample.h"
#include "libDeviceGateway.h"

int main(int argc, char* argv[])
{
	IDeviceGateway *devGW = getDeviceGateway();
	if (!devGW)
	{
		cout << "Unable to get DeviceGateway handler\n";
		return -1;
	}	

	int input = 1;

	while(input)
	{
		cout << "\n";								
		cout << "Please input command:\n";
		cout << "0 : Exit!\n";
		cout << "1 : Initialize DeviceGateway\n";
		cout << "2 : Connect SensorDevices to the DeviceGateway\n";
		cout << "3 : Connect an AVDevice to the DeviceGateway\n";
		cout << "5 : Disconnect an AVDevice from the DeviceGateway\n";
		cout << "6 : Get all SensorDevices information\n";
		cout << "7 : Get all AVDevices information\n";
		cout << "8 : Pull measurements from SensorMote\n";
		cout << "9 : Start periodic measurements for SensorMote\n";
		cout << "10 : Reconfigure periodic measurements for SensorMote\n";
		cout << "11 : Stop periodic measurements for SensorMote\n";
		//scanf("%u", &input);
		cin >> input;

		
		if (input==0) break;

		else if (input==1)
		{
			int result = devGW->initializeDeviceGW();
			if (result) cout << "DeviceGateway initialized\n";
			else cout << "DeviceGateway failed to initialize\n";
		}

		else if (input==2)
		{
			string configFile;
			cout << "Configuration File: ";
			cin >> configFile;
			ISensorDevices *sdevices = (ISensorDevices *) (new SensorDevicesWaspMote());//getSensorDevices();

			if (sdevices->setConfigurationFromFile(configFile))
			{			
				sdevices->registerToDGW(devGW);
				devGW->connectSensorDevices((ISensorDevices*)sdevices);	
			}
			else
			{	
				cout << "No configuration details in the provided file or file does not exist\n";
			}	
		}

		else if (input==3)
		{
			string avName;
			cout << "AVDeviceName: ";
			cin >> avName;
			IAVDevice *avdev = (IAVDevice *) (new AVDeviceExample());//getAVDevice();	
			avDeviceInfo devInfo = avDeviceInfo(avName,1221312,"usb","com1",location(1,2,3),"kitchen");
			avdev->initializeDevice(devInfo);
			avdev->registerToDGW(devGW);
			devGW->connectAVDevice(avdev);		
		}
		else if (input==5)
		{
			string moteName;
			cout << "AVDeviceName: ";
			cin >> moteName;
			devGW->disconnectAVDevice(moteName);		
		}

		else if (input==6)
		{
			vector<moteDetails> allSMdetails =  devGW->getAllSensorDevicesInfo();
			vector<moteDetails>::iterator it;
			for (it = allSMdetails.begin(); it != allSMdetails.end(); it++)
			{
				moteDetails mote = (*it);
				cout << "SensorMote " << it - allSMdetails.begin() << ": {" << mote.smoteInfo.sensorMoteID << "," << mote.smoteInfo.serialNumber << "," << mote.smoteInfo.deviceIF << "," << mote.smoteInfo.connectionStr << "," << mote.smoteInfo.room << "," << mote.smoteInfo.numSensors << "}\n";
				vector<sensor>::iterator it1;
				for (it1 = mote.sensorsInfo.begin(); it1 != mote.sensorsInfo.end(); it1++)
				{
					cout << "Sensor " << it1 - mote.sensorsInfo.begin() << ": {" << (*it1).type << "," << (*it1).parentMoteID << "," << (*it1).siUnit << "," << (*it1).minValue << "," << (*it1).maxValue << "," << (*it1).accuracy << "}\n";				
				}
				cout << "\n";								
			}
		}

		else if (input==7)
		{
			vector<avDeviceInfo> allAVs = devGW->getAllAVDevicesInfo();
			vector<avDeviceInfo>::iterator it;
			for (it = allAVs.begin(); it != allAVs.end(); it++)
			{
				cout << "AVDevice " << it - allAVs.begin() << ": {" << (*it).deviceID << "," << (*it).serialNumber << "," << (*it).deviceIF << "," << (*it).connectionStr << "," << (*it).room << "}\n";
			}
		}

		else if (input==8)
		{
			string moteName;
			cout << "SensorMoteName: ";
			cin >> moteName;
			int res = devGW->getMeasurements(moteName);
			/*vector<measurement> allmeasurements = devGW->getMeasurements(moteName);
			vector<measurement>::iterator it;
			for (it = allmeasurements.begin(); it != allmeasurements.end(); it++)
			{
				cout << "Measurement " << it - allmeasurements.begin() << ": {" << (*it).sensorMoteID << "," << (*it).sensType << "," << (*it).value << "," << (*it).timestamp << "}\n";				
			}*/			
		}
		else if (input==9)
		{
			string moteName;
			cout << "SensorMoteName: ";
			cin >> moteName;
			float period; 
			cout << "ReportingPeriod: ";
			cin >> period;
			devGW->startMeasuring(moteName, period);		
		}
		else if (input==10)
		{
			string moteName;
			cout << "SensorMoteName: ";
			cin >> moteName;
			float period; 
			cout << "ReportingPeriod: ";
			cin >> period;
			devGW->reconfigure(moteName, period);		
		}
		else if (input==11)
		{
			string moteName;
			cout << "SensorMoteName: ";
			cin >> moteName;
			devGW->stopMeasuring(moteName);		
		}
	}
	return 0;
}

