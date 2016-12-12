/*=================================================================================
Basic explanation:

    DeviceGateway testing console

Authors: Daniel Denkovski (UKIM) (danield@feit.ukim.edu.mk)

Revision History:
    26.08.2014, ver 0.1
    18.10.2014, ver 0.2
====================================================================================*/

#include "libSensorDeviceWaspMote.h"
#include "libSensorDeviceArduino.h"
#include "libSensorDevicePrerecorded.h"
#include "libAVKinect.h"
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
		cout << "2 : Connect Arduino sensor devices to the DeviceGateway\n";
		cout << "3 : Connect WaspMote sensor devices to the DeviceGateway\n";
		cout << "4 : Connect prerecorded sensor data to the DeviceGateway\n";
		cout << "5 : Connect Kinect AVDevice to the DeviceGateway\n";
		cout << "6 : Get all SensorDevices information\n";
		cout << "7 : Get all AVDevices information\n";
		cout << "8 : Pull measurements from SensorMote\n";
		cout << "9 : Start periodic measurements for SensorMote\n";
		cout << "10 : Reconfigure periodic measurements for SensorMote\n";
		cout << "11 : Stop periodic measurements for SensorMote\n";
		cout << "12 : Start video streaming for AV device\n";
		cout << "13 : Stop video streaming for AV device\n";
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
			SensorDevicesArduino *sdeviceArduino = new SensorDevicesArduino();

			if (sdeviceArduino->setConfigurationFromFile(configFile))
			{			
				sdeviceArduino->registerToDGW(devGW);
				devGW->connectSensorDevices((ISensorDevices*)sdeviceArduino);	
			}
			else
			{	
				cout << "No configuration details in the provided file or file does not exist\n";
			}	
		}

		else if (input==3)
		{
			string configFile;
			cout << "Configuration File: ";
			cin >> configFile;
			SensorDevicesWaspMote *sdeviceWM = new SensorDevicesWaspMote();

			if (sdeviceWM->setConfigurationFromFile(configFile))
			{			
				sdeviceWM->registerToDGW(devGW);
				devGW->connectSensorDevices((ISensorDevices*)sdeviceWM);	
			}
			else
			{	
				cout << "No configuration details in the provided file or file does not exist\n";
			}	
		}
		else if (input==4)
		{
			string configFile;
			cout << "Configuration File: ";
			cin >> configFile;
			SensorMotePrerecorded *smote = new SensorMotePrerecorded();
			if (smote->setConfigurationFromFile(configFile))
			{			
				smote->registerToDGW(devGW);
				devGW->connectSensorDevices((ISensorDevices*)smote);
				string resultsFile;
				cout << "Results File: ";
				cin >> resultsFile;
				if (!smote->openDataFile(resultsFile))
					cout << "File does not exist\n";	
			}
			else
			{	
				cout << "No configuration details in the provided file or file does not exist\n";
			}	
		}
		else if (input==5)
		{
			string configFile;
			cout << "Configuration File: ";
			cin >> configFile;
			AVKinect *avdev = new AVKinect();
			if (avdev->setConfigurationFromFile(configFile))
			{			
				avdev->registerToDGW(devGW);
				devGW->connectAVDevice((IAVDevice*)avdev);	
			}			
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
		else if (input==12)
		{
			string kinectName;
			cout << "KinectName: ";
			cin >> kinectName;
			int RGBOrDepth;
			cout << "RGB streaming (1) or depth streaming (2) or audio (3): ";
			cin >> RGBOrDepth;
			if (RGBOrDepth == 1)
			{
				frameResolution res = frameResolution(640,480);
				streamConfig conf = streamConfig(res,1,25,1,RGBOrDepth,1);
				int result = devGW->startStream(kinectName, &conf);
				if (result) cout << "Kinect started RGB streaming\n";
				else cout << "Kinect did not start RBG streaming\n";
			}
			else if (RGBOrDepth == 2)
			{
				frameResolution res = frameResolution(640,480);
				streamConfig conf = streamConfig(res,1,25,1,RGBOrDepth,2);
				int result = devGW->startStream(kinectName, &conf);
				if (result) cout << "Kinect started depth streaming\n";
				else cout << "Kinect did not start depth streaming\n";
			}
			else if (RGBOrDepth == 3)
			{
				frameResolution res = frameResolution(640,480);
				streamConfig conf = streamConfig(res,4,16000,2,RGBOrDepth,4);
				int result = devGW->startStream(kinectName, &conf);
				if (result) cout << "Kinect started audio streaming\n";
				else cout << "Kinect did not start audio streaming\n";
			}
		}
		else if (input==13)
		{
			string kinectName;
			cout << "KinectName: ";
			cin >> kinectName;
			int RGBOrDepth;
			cout << "RGB streaming (1) or depth streaming (2) or audio (3): ";
			cin >> RGBOrDepth;
			if (RGBOrDepth == 1)
			{
				int result = devGW->stopStream(kinectName,RGBOrDepth);
				if (result) cout << "Kinect stopped RGB streaming\n";
				else cout << "Kinect did not stop RGB streaming\n";
			}	
			else if (RGBOrDepth == 2)
			{
				int result = devGW->stopStream(kinectName,RGBOrDepth);
				if (result) cout << "Kinect stopped depth streaming\n";
				else cout << "Kinect did not stop depth streaming\n";			
			}
			else if (RGBOrDepth == 3)
			{
				int result = devGW->stopStream(kinectName,RGBOrDepth);
				if (result) cout << "Kinect stopped audio streaming\n";
				else cout << "Kinect did not stop audio streaming\n";			
			}
		}
	}
	return 0;
}

