/*=================================================================================
Basic explanation:

    DeviceGateway testing console

Authors: Daniel Denkovski (UKIM) (danield@feit.ukim.edu.mk)

Revision History:
    26.08.2014, ver 0.1
====================================================================================*/

//#include "libSensorDeviceExample.h"
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

	int result = devGW->initializeDeviceGW();
	if (result) cout << "DeviceGateway initialized\n";
	else cout << "DeviceGateway failed to initialize\n";

	IAVDevice *avdev = (IAVDevice *) (new AVKinect());//getAVDevice();	
	avDeviceInfo devInfo = avDeviceInfo("kinect",1221312,"usb","com1",location(1,2,3),"kitchen");
	avdev->initializeDevice(devInfo);
	avdev->registerToDGW(devGW);
	devGW->connectAVDevice(avdev);	

	int input = 1;

	while(input)
	{
		cout << "\n";								
		cout << "Please input command:\n";
		cout << "0 : Exit!\n";
		cout << "1 : Start Kinect RGB\n";
		cout << "2 : Reconfigure Kinect RGB\n";
		cout << "3 : Stop Kinect RGB\n";
		cout << "4 : Start Kinect depth\n";
		cout << "5 : Reconfigure Kinect depth\n";
		cout << "6 : Stop Kinect depth\n";
                cout << "7 : Start Kinect Audio\n";
		cout << "8 : Stop Kinect Audio\n";

		//scanf("%u", &input);
		cin >> input;
		
		if (input==0) break;

		else if (input==1)
		{
			frameResolution res = frameResolution(640,480);
			streamConfig conf = streamConfig(res,3,25,1,1,1);
			int result = avdev->startStream(&conf);
			if (result) cout << "Kinect started video streaming\n";
			else cout << "Kinect did not start video streaming\n";	
		}
		else if (input==2)
		{
			frameResolution res = frameResolution(640,480);
			streamConfig conf = streamConfig(res,3,25,1,1,1);
			int result = avdev->reconfigureStream(&conf);
			if (result) cout << "Kinect reconfigured video streaming\n";
			else cout << "Kinect did not reconfigure video streaming\n";	
		}
		else if (input==3)
		{
			int result = avdev->stopStream(1);
			if (result) cout << "Kinect stopped RGB streaming\n";
			else cout << "Kinect did not stop RGB streaming\n";	
		}
		else if (input==4)
		{
			frameResolution res = frameResolution(640,480);
			streamConfig conf = streamConfig(res,1,25,1,2,2);
			int result = avdev->startStream(&conf);
			if (result) cout << "Kinect started depth streaming\n";
			else cout << "Kinect did not start depth streaming\n";	
		}
		else if (input==5)
		{
			frameResolution res = frameResolution(640,480);
			streamConfig conf = streamConfig(res,1,25,1,2,2);
			int result = avdev->startStream(&conf);
			if (result) cout << "Kinect reconfigured depth streaming\n";
			else cout << "Kinect did not reconfigure depth streaming\n";		
		}
		else if (input==6)
		{
			int result = avdev->stopStream(2);
			if (result) cout << "Kinect stopped depth streaming\n";
			else cout << "Kinect did not stop depth streaming\n";	
		}
		else if (input==7)
		{
			frameResolution res = frameResolution(480,1);
			streamConfig conf = streamConfig(res,4,16000,2,3,4);
			int result = avdev->startStream(&conf);
			if (result) cout << "Kinect started audio streaming\n";
			else cout << "Kinect did not start audio streaming\n";	
		}
		else if (input==8)
		{
			int result = avdev->stopStream(3);
			if (result) cout << "Kinect stopped audio streaming\n";
			else cout << "Kinect did not stop audio streaming\n";		
		}

	}
	return 0;
}

