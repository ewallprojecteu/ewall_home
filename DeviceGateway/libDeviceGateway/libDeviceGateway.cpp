/*=================================================================================
Basic explanation:

    Device Gateway cpp code

Authors: Daniel Denkovski (UKIM) (danield@feit.ukim.edu.mk)

Revision History:
    25.08.2014, ver 0.1
====================================================================================*/

#include "libDeviceGateway.h"
#include "string.h"
#include "json_spirit_dgw.h"

static IDeviceGateway *g_dgw = NULL;
static int g_dgwCount = 0;

#include <pthread.h>
pthread_mutex_t m_dgw_mutex;

IDeviceGateway *getDeviceGateway()
{
	if (!g_dgw)
		g_dgw = new DeviceGateway();
	if (g_dgw)
		g_dgwCount++;
	return g_dgw;
}

void releaseDeviceGateway(const IDeviceGateway *pDgw)
{
	if (g_dgw && (pDgw == g_dgw))
	{
		g_dgwCount--;
		if(g_dgwCount == 0){
			delete g_dgw;
			g_dgw = NULL;
		}
	}
}

IDeviceGateway::~IDeviceGateway()
{
}

DeviceGateway::~DeviceGateway()
{
	releaseDeviceGateway(this);
}

DeviceGateway::DeviceGateway()
{
	pthread_mutex_init( &m_dgw_mutex, NULL );
}

int DeviceGateway::initializeDeviceGW()
{
	sensorDevices.clear();
	avDevices.clear();
	sockComm = getDGwSockets();
	sockComm->initializeSocketServer(this);
	cout << "DeviceGateway: DeviceGateway initialized!\n";
	return 1;
}
	
int DeviceGateway::connectSensorDevices(ISensorDevices *sDevice)
{
	vector<moteDetails> mySensorMotes = sDevice->getAllSensorMotesDetails();
	for (int i = 0; i < mySensorMotes.size(); i++)
	{ 
		int index = findSensorDevicesByID(mySensorMotes[i].smoteInfo.sensorMoteID);
		if (index >= 0)
		{
			ISensorDevices* oldSDev = *(sensorDevices.begin() + index);
			oldSDev->deregisterFromDGW();
			delete oldSDev;
			oldSDev = NULL;
			sensorDevices.erase(sensorDevices.begin() + index);		
		}
	}
	sensorDevices.push_back(sDevice);
	cout << "DeviceGateway: " << sensorDevices.size() << " SensorGateways available!\n";
	return 1;
}

int DeviceGateway::disconnectSensorDevices(ISensorDevices *sDevice)
{
	int index = findSensorDevicesHndlByID(sDevice);
	if (index >= 0)
	{
		ISensorDevices* oldSDev = *(sensorDevices.begin() + index);
		oldSDev->deregisterFromDGW();
		delete oldSDev;
		oldSDev = NULL;
		sensorDevices.erase(sensorDevices.begin() + index);	
		return 1;	
	}
	return 0;
}

int DeviceGateway::connectAVDevice(IAVDevice *avdev)
{
	int index = findAVDeviceByID(avdev->getAVDeviceInfo().deviceID);
	if (index >= 0)
	{
		IAVDevice* oldAVDev = *(avDevices.begin() + index);
		oldAVDev->deregisterFromDGW();
		delete oldAVDev;
		oldAVDev = NULL;
		avDevices.erase(avDevices.begin() + index);		
	}
	avDevices.push_back(avdev);
	cout << "DeviceGateway: AVDevice with ID = " << avDevices[avDevices.size()-1]->getAVDeviceInfo().deviceID << " connected!\n";
	cout << "DeviceGateway: " << avDevices.size() << " AVDevices available!\n";
	return 1;
}

int DeviceGateway::disconnectAVDevice(string avDeviceID)
{
	int index = findAVDeviceByID(avDeviceID);
	if (index >= 0)
	{
		IAVDevice* oldAVDev = *(avDevices.begin() + index);
		oldAVDev->deregisterFromDGW();
		delete oldAVDev;
		oldAVDev = NULL;
		avDevices.erase(avDevices.begin() + index);	
		return 1;	
	}
	return 0;
}

int DeviceGateway::startStream (string avDeviceID, streamConfig *conf)
{
	int index = findAVDeviceByID(avDeviceID);
	if (index >= 0)
	{
		IAVDevice* currAVDev = *(avDevices.begin() + index);
		if (currAVDev != NULL)
		{
			cout << "DeviceGateway: AVDevice with ID = " << currAVDev->getAVDeviceInfo().deviceID << " queried to start streaming!\n";
			return currAVDev->startStream(conf);
		}
	}
	return 0;
}

int DeviceGateway::reconfigureStream (string avDeviceID, streamConfig *conf)
{
	int index = findAVDeviceByID(avDeviceID);
	if (index >= 0)
	{
		IAVDevice* currAVDev = *(avDevices.begin() + index);
		if (currAVDev != NULL)
		{
			cout << "DeviceGateway: AVDevice with ID = " << currAVDev->getAVDeviceInfo().deviceID << " queried to reconfigure streaming!\n";
			return currAVDev->reconfigureStream(conf);
		}
	}
	return 0;
}

int DeviceGateway::stopStream (string avDeviceID, short streamtype)
{
	int index = findAVDeviceByID(avDeviceID);
	if (index >= 0)
	{
		IAVDevice* currAVDev = *(avDevices.begin() + index);
		if (currAVDev != NULL)
		{
			cout << "DeviceGateway: AVDevice with ID = " << currAVDev->getAVDeviceInfo().deviceID << " queried to stop streaming!\n";
			return currAVDev->stopStream(streamtype);
		}
	}
	return 0;
}

int DeviceGateway::rotateAVDevice (string avDeviceID, double angle)
{
	int index = findAVDeviceByID(avDeviceID);
	if (index >= 0)
	{
		IAVDevice* currAVDev = *(avDevices.begin() + index);
		if (currAVDev != NULL)
		{
			cout << "DeviceGateway: AVDevice with ID = " << currAVDev->getAVDeviceInfo().deviceID << " queried to rotate!\n";
			return currAVDev->rotateAVDevice(angle);
		}
	}
	return 0;
}

int DeviceGateway::pushFrame (string avDeviceID, void *frame, streamConfig streamFormat, string timestamp)
{
	string sensDeviceID(avDeviceID);
	int msgLength = sizeof(DGwSockMessage);
	int configLength = sizeof(streamFormat);
	int resultsLength = 0;
	string results = streamConfigToJSON(streamFormat);
	
	if ((streamFormat.stream == 1) || (streamFormat.stream == 2) || (streamFormat.stream == 3))
	{
		resultsLength = (streamFormat.resolution.width*streamFormat.resolution.height)*streamFormat.nochs*streamFormat.bytesps;
	}

	char *bufsnd = new char[msgLength + configLength + resultsLength];
 	DGwSockMessage *sndmsg;
	sndmsg = (DGwSockMessage *)bufsnd;
	sndmsg->MsgType = TypePushFrameRsp;
	sndmsg->MsgLen = msgLength + configLength;
	memset((void*)(sndmsg->devID), 0, DevIDLength); 
	sensDeviceID.copy(sndmsg->devID, DevIDLength);

	memcpy(bufsnd + msgLength, (void*)&streamFormat, configLength);
	memcpy(bufsnd + msgLength + configLength, frame, resultsLength);
	pthread_mutex_lock( &m_dgw_mutex );
	sockComm->sendDataToDGwClients(bufsnd, msgLength + configLength + resultsLength);
	pthread_mutex_unlock( &m_dgw_mutex );
	delete [] bufsnd;
	//cout << "DeviceGateway: DeviceGateway pushing frame from AVDevice with ID = " << avDeviceID << " to AV processing !\n";
	return 1;
}

streamConfig DeviceGateway::getConfiguration (string avDeviceID)
{
	int index = findAVDeviceByID(avDeviceID);
	streamConfig conf;
	if (index >= 0)
	{
		IAVDevice* currAVDev = *(avDevices.begin() + index);
		if (currAVDev != NULL)
		{
			cout << "DeviceGateway: DeviceGateway quering requesting configuration information from AVDevice with ID = " << avDeviceID << "!\n";
			return currAVDev->getConfiguration();
		}
	}
	return conf;
}

avDeviceInfo DeviceGateway::getAVDeviceInfo (string avDeviceID)
{
	int index = findAVDeviceByID(avDeviceID);
	avDeviceInfo result;
	if (index >= 0)
	{
		IAVDevice* currAVDev = *(avDevices.begin() + index);
		if (currAVDev != NULL)
		{
			cout << "DeviceGateway: DeviceGateway quering requesting AVDevice information from AVDevice with ID = " << avDeviceID << "!\n";
			return currAVDev->getAVDeviceInfo();
		}
	}	
	return result;
}

vector<avDeviceInfo> DeviceGateway::getAllAVDevicesInfo()
{
	vector<avDeviceInfo> allAVs;
	vector<IAVDevice*>::iterator it;
	for (it = avDevices.begin(); it != avDevices.end(); it++)
		allAVs.push_back((*it)->getAVDeviceInfo());
	cout << "DeviceGateway: DeviceGateway collects all AVDevices details\n";
	return allAVs;
}
		
int DeviceGateway::startMeasuring (string sensorMoteID, float rate)
{
	int index = findSensorDevicesByID(sensorMoteID);
	if (index >= 0)
	{
		ISensorDevices* currSDev = *(sensorDevices.begin() + index);
		if (currSDev != NULL)
		{
			cout << "DeviceGateway: DeviceGateway requesting periodic measurements from SensorMote with ID = " << sensorMoteID << "!\n";
			return currSDev->startMeasuring(sensorMoteID, rate);
		}
	}
	return 0;
}

int DeviceGateway::reconfigure (string sensorMoteID, float rate)
{
	int index = findSensorDevicesByID(sensorMoteID);
	if (index >= 0)
	{
		ISensorDevices* currSDev = *(sensorDevices.begin() + index);
		if (currSDev != NULL)
		{
			cout << "DeviceGateway: DeviceGateway requesting reconfiguration of SensorMote with ID = " << sensorMoteID << "!\n";
			return currSDev->reconfigure(sensorMoteID, rate);
		}
	}
	return 0;
}

int DeviceGateway::stopMeasuring (string sensorMoteID)
{
	int index = findSensorDevicesByID(sensorMoteID);
	if (index >= 0)
	{
		ISensorDevices* currSDev = *(sensorDevices.begin() + index);
		if (currSDev != NULL)
		{
			cout << "DeviceGateway: DeviceGateway queries SensorMote with ID = " << sensorMoteID << " to stop measurements!\n";
			return currSDev->stopMeasuring(sensorMoteID);
		}
		else return 0;
	}
	return 0;
}

int DeviceGateway::pushMeasurements (string sensorMoteID, vector<measurement> measurements)
{
	string sensDeviceID(sensorMoteID);
	string results = measurementsToJSON(measurements);
	int msgLength = sizeof(DGwSockMessage);
	int resultsLength = results.size();
	char *bufsnd = new char[msgLength + resultsLength];
 	DGwSockMessage *sndmsg;
	sndmsg = (DGwSockMessage *)bufsnd;
	sndmsg->MsgType = TypePushMeasRsp;
	sndmsg->MsgLen = msgLength + resultsLength;
	memset((void*)(sndmsg->devID), 0, DevIDLength); 
	sensDeviceID.copy(sndmsg->devID, DevIDLength);

	results.copy(bufsnd + msgLength, resultsLength);
	pthread_mutex_lock( &m_dgw_mutex );
	sockComm->sendDataToDGwClients(bufsnd, msgLength + resultsLength);
	pthread_mutex_unlock( &m_dgw_mutex );
	delete [] bufsnd;
	return 1;
}

int DeviceGateway::getMeasurements (string sensorMoteID)
{
	int index = findSensorDevicesByID(sensorMoteID);
	if (index >= 0)
	{
		ISensorDevices* currSDev = *(sensorDevices.begin() + index);
		if (currSDev != NULL)
		{
			cout << "DeviceGateway: DeviceGateway requests measurements from SensorMote with ID = " << sensorMoteID << "!\n";		return currSDev->getMeasurements(sensorMoteID);
			//return 1;
		}
	}
	return 0;
}

vector<sensor> DeviceGateway::getSensors (string sensorMoteID)
{
	int index = findSensorDevicesByID(sensorMoteID);
	vector<sensor> result;
	result.clear();
	if (index >= 0)
	{
		ISensorDevices* currSDev = *(sensorDevices.begin() + index);
		if (currSDev != NULL)
		{
			cout << "DeviceGateway: DeviceGateway requests for sensors information from SensorMote with ID = " << sensorMoteID << "!\n";
			return currSDev->getSensors(sensorMoteID);
		}
	}
	return result;
}

moteInfo DeviceGateway::getSensorMoteInfo (string sensorMoteID)
{
	int index = findSensorDevicesByID(sensorMoteID);
	moteInfo info;
	if (index >= 0)
	{
		ISensorDevices* currSDev = *(sensorDevices.begin() + index);
		if (currSDev != NULL)
		{
			cout << "DeviceGateway: DeviceGateway requests for SensorMote information from SensorMote with ID = " << sensorMoteID << "!\n";
			return currSDev->getSensorMoteInfo(sensorMoteID);
		}
	}
	return info;
}

vector<moteDetails> DeviceGateway::getAllSensorDevicesInfo()
{
	vector<moteDetails> allMotesSensors;
	vector<ISensorDevices*>::iterator it;
	for (it = sensorDevices.begin(); it != sensorDevices.end(); it++)
	{
		vector<moteDetails> newMotes;
		newMotes = (*it)->getAllSensorMotesDetails();
		allMotesSensors.insert(allMotesSensors.end(), newMotes.begin(), newMotes.end());
	}
	return allMotesSensors;
	cout << "DeviceGateway: DeviceGateway collects all SensorDevices details\n";
}

float DeviceGateway::getRate(string sensorMoteID)
{
	int index = findSensorDevicesByID(sensorMoteID);
	if (index >= 0)
	{
		ISensorDevices* currSDev = *(sensorDevices.begin() + index);
		if (currSDev != NULL)
		{
			cout << "DeviceGateway: DeviceGateway requests current reporting rate information from SensorMote with ID = " << sensorMoteID << "!\n";
			return currSDev->getRate(sensorMoteID);
		}
	}
	return -1;
}

int DeviceGateway::findSensorDevicesByID(string sensorMoteID)
{
	vector<ISensorDevices*>::iterator it;
	for (it = sensorDevices.begin(); it != sensorDevices.end(); it++)
	{
		vector<moteDetails> motesPerDevice = (*it)->getAllSensorMotesDetails();
		vector<moteDetails>::iterator it1;
		for (it1 = motesPerDevice.begin(); it1 != motesPerDevice.end(); it1++)
		{
			if ((it1)->smoteInfo.sensorMoteID == sensorMoteID)
				return (it - sensorDevices.begin());
		}
	}
	return -1;
}

int DeviceGateway::findSensorDevicesHndlByID(ISensorDevices *sDevice)
{
	vector<ISensorDevices*>::iterator it;
	for (it = sensorDevices.begin(); it != sensorDevices.end(); it++)
	{
		if ((*it) == sDevice)
			return (it - sensorDevices.begin());
	}
	return -1;
}

int DeviceGateway::findAVDeviceByID(string avDeviceID)
{
	vector<IAVDevice*>::iterator it;
	for (it = avDevices.begin(); it != avDevices.end(); it++)
	{
		if ((*it)->getAVDeviceInfo().deviceID == avDeviceID)
			return (it - avDevices.begin());
	}
	return -1;
}
