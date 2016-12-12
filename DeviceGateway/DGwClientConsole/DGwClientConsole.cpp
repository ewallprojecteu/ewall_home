/*=================================================================================
Basic explanation:

    DeviceGateway testing console

Authors: Daniel Denkovski (UKIM) (danield@feit.ukim.edu.mk)

Revision History:
    26.08.2014, ver 0.1
====================================================================================*/

#include "libDGwClient.h"

int main(int argc, char* argv[])
{
	IDGwClient *devGWClient = (IDGwClient *)(new DGwClient());//getDGwClient();
	if (!devGWClient)
	{
		cout << "Unable to get DGwClient handler\n";
		return -1;
	}	

	int input = 1;

	while(input)
	{
		cout << "\n";								
		cout << "Please input command:\n";
		cout << "0 : Exit!\n";
		cout << "1 : Initialize DGwClient\n";
		/*cout << "2 : Connect a SensorMote to the DeviceGateway\n";
		cout << "3 : Connect an AVDevice to the DeviceGateway\n";
		cout << "4 : Disconnect a SensorMote from the DeviceGateway\n";
		cout << "5 : Disconnect an AVDevice from the DeviceGateway\n";*/
		cout << "6 : Get all SensorDevices information\n";
		cout << "7 : Get all AVDevices information\n";
		cout << "8 : Pull measurements from SensorMote\n";
		cout << "9 : Start periodic measurements for SensorMote\n";
		cout << "10 : Reconfigure periodic measurements for SensorMote\n";
		cout << "11 : Stop periodic measurements for SensorMote\n";
		cout << "12 : Get configuration of AVDevice\n";
		cout << "13 : Rotate AVDevice\n";
		cout << "14 : Get AVDevice information\n";
		cout << "15 : Get SensorMote information\n";
		cout << "16 : Get sensors for Sensormote\n";
		cout << "17 : Get reporting rate of Sensormote\n";
		//scanf("%u", &input);
		cin >> input;

		if (input==0) break;

		else if (input==1)
		{
			int result = devGWClient->initializeDGwClient();
			if (result) cout << "DGwClient initialized\n";
			else cout << "DGwClient failed to initialize\n";
		}

		/*else if (input==2)
		{
			string moteName;
			cout << "SensorMoteName: ";
			cin >> moteName;
			ISensorMote *smote = getSensorMote();
			moteInfo smoteInfo = moteInfo(moteName,1221312,"usb","com1","11:22:33:44:55:66",location(1,2,3),"kitchen",3);
			vector<sensor> sensors;
			sensors.push_back(sensor(_TEMPERATURE,moteName,"°C",-30,50,1));
			sensors.push_back(sensor(_HUMIDITY,moteName,"%RH",0,100,1));
			sensors.push_back(sensor(_LUMINOCITY,moteName,"lux",0,1000,1));

			smote->initializeDevice(smoteInfo, sensors);	
			smote->registerToDGW(devGW);
			devGW->connectSensorMote(smote);		
		}

		else if (input==3)
		{
			string avName;
			cout << "AVDeviceName: ";
			cin >> avName;
			IAVDevice *avdev = getAVDevice();	
			avDeviceInfo devInfo = avDeviceInfo(avName,1221312,"usb","com1",location(1,2,3),"kitchen");
			avdev->initializeDevice(devInfo);
			avdev->registerToDGW(devGW);
			devGW->connectAVDevice(avdev);		
		}

		else if (input==4)
		{
			string moteName;
			cout << "SensorMoteName: ";
			cin >> moteName;
			devGW->disconnectSensorMote(moteName);		
		}
		else if (input==5)
		{
			string moteName;
			cout << "AVDeviceName: ";
			cin >> moteName;
			devGW->disconnectAVDevice(moteName);		
		}*/

		else if (input==6)
		{
			string result = devGWClient->getAllSensorDevicesInfo();
			
			/*vector<moteDetails> allSMdetails = devGWClient->getAllSensorDevicesInfo();
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
			}*/
		}

		else if (input==7)
		{
			string result = devGWClient->getAllAVDevicesInfo();
			/*vector<avDeviceInfo> allAVs = devGWClient->getAllAVDevicesInfo();
			vector<avDeviceInfo>::iterator it;
			for (it = allAVs.begin(); it != allAVs.end(); it++)
			{
				cout << "AVDevice " << it - allAVs.begin() << ": {" << (*it).deviceID << "," << (*it).serialNumber << "," << (*it).deviceIF << "," << (*it).connectionStr << "," << (*it).room << "}\n";
			}*/
		}

		else if (input==8)
		{
			string moteName;
			cout << "SensorMoteName: ";
			cin >> moteName;

			string result = devGWClient->getMeasurements(moteName);
			/*vector<measurement> allmeasurements = devGWClient->getMeasurements(moteName);
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
			devGWClient->startMeasuring(moteName, 5);		
		}
		else if (input==10)
		{
			string moteName;
			cout << "SensorMoteName: ";
			cin >> moteName;
			float period; 
			cout << "ReportingPeriod: ";
			cin >> period;
			devGWClient->reconfigure(moteName, period);		
		}
		else if (input==11)
		{
			string moteName;
			cout << "SensorMoteName: ";
			cin >> moteName;
			devGWClient->stopMeasuring(moteName);		
		}
		else if (input==12)
		{
			string moteName;
			cout << "AVDeviceName: ";
			cin >> moteName;
			devGWClient->getConfiguration(moteName);		
		}
		else if (input==13)
		{
			string moteName;
			cout << "AVDeviceName: ";
			cin >> moteName;
			devGWClient->rotateAVDevice(moteName, 5);		
		}
		else if (input==14)
		{
			string moteName;
			cout << "AVDeviceName: ";
			cin >> moteName;
			devGWClient->getAVDeviceInfo(moteName);		
		}
		else if (input==15)
		{
			string moteName;
			cout << "SensorMoteName: ";
			cin >> moteName;
			devGWClient->getSensorMoteInfo(moteName);		
		}
		else if (input==16)
		{
			string moteName;
			cout << "SensorMoteName: ";
			cin >> moteName;
			devGWClient->getSensors(moteName);		
		}
		else if (input==17)
		{
			string moteName;
			cout << "SensorMoteName: ";
			cin >> moteName;
			devGWClient->getRate(moteName);		
		}
	}
	return 0;
}

