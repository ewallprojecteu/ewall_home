/*=================================================================================
Basic explanation:

    Device Gateway cpp code

Authors: Daniel Denkovski (UKIM) (danield@feit.ukim.edu.mk)

Revision History:
    25.08.2014, ver 0.1
====================================================================================*/

#include "libSensorDeviceExample.h"
#include "time_funcs.h"
#include "json_spirit_dgw.h"
#include <pthread.h>
#include <unistd.h>
#include <time.h>

pthread_t hthread;

void *threadRun(void *object)
{
	ISensorDevices* mysm = (ISensorDevices*)object;
	/*while (mysm->getRate() > 0)
	{
		//usleep((unsigned int)(mysm->getRate()*1000000));
		//mysm->pushMeasurements(mysm->getMeasurements());
	}*/
}

inline ISensorDevices::~ISensorDevices()
{
}

SensorDevicesExample::~SensorDevicesExample()
{
}

SensorDevicesExample::SensorDevicesExample()
{
}

int SensorDevicesExample::initializeDevice(vector<moteDetails> sensorMotes)
{
	int ret = initialize(sensorMotes);
	if (ret == 1)
	{
		cout << "SensorDevices: Initialization success. " << sensorMotes.size() << " sensor motes registered!\n";
	}
	else
	{
		cout << "SensorDevices: Initialization failed!\n";
		return 0;
	}
	return 1;
}

int SensorDevicesExample::startMeasuring (string sensorMoteID, float reportPeriod)
{
	int index = findSensorMoteByID(sensorMoteID);
	if (index >= 0)
	{
		moteDetails mote = getSensorMoteDetails(index);
		float repPeriod = getRate(sensorMoteID);

		if (repPeriod == 0)
		{	
			setRate(sensorMoteID,reportPeriod);
			cout << "SensorDevices: SensorMote with ID = " << mote.smoteInfo.sensorMoteID << " started measurements with period of " << getRate(sensorMoteID) << "!\n";
			//start periodic measurement on sensor mote with ID = mote.smoteInfo.sensorMoteID
			return 1;
		}
		else
		{
			cout << "SensorDevices: SensorMote with ID = " << mote.smoteInfo.sensorMoteID << " already measuring with period of " << repPeriod << "!\n";
			return 0;
		}
	}
	cout << "SensorDevices: SensorMote with ID = " << sensorMoteID << " does not exist!\n";
	return 0;
}

int SensorDevicesExample::reconfigure (string sensorMoteID, float reportPeriod)
{
	int index = findSensorMoteByID(sensorMoteID);
	if (index >= 0)
	{
		moteDetails mote = getSensorMoteDetails(index);
		float repPeriod = getRate(sensorMoteID);

		if (repPeriod > 0)
		{
			setRate(sensorMoteID,reportPeriod);
			cout << "SensorDevices: SensorMote with ID = " << mote.smoteInfo.sensorMoteID << " reconfigured measurement period to " << getRate(sensorMoteID) << "!\n";
			//reconfigure period on sensor mote with ID = mote.smoteInfo.sensorMoteID
			return 1;
		}
		else 
		{
			cout << "SensorDevices: SensorMote with ID = " << mote.smoteInfo.sensorMoteID << " still not started!\n";
			return 0;
		}
	}
	cout << "SensorDevices: SensorMote with ID = " << sensorMoteID << " does not exist!\n";
	return 0;
}

int SensorDevicesExample::stopMeasuring (string sensorMoteID)
{
	int index = findSensorMoteByID(sensorMoteID);
	if (index >= 0)
	{
		moteDetails mote = getSensorMoteDetails(index);
		float repPeriod = getRate(sensorMoteID);

		if (repPeriod > 0)
		{	
			setRate(sensorMoteID,0);
			cout << "SensorDevices: SensorMote with ID = " << mote.smoteInfo.sensorMoteID << " stopped measurements!\n";
			//stop periodic measurements on sensor mote with ID = mote.smoteInfo.sensorMoteID
			return 1;
		}
		else
		{
			cout << "SensorDevices: SensorMote with ID = " << mote.smoteInfo.sensorMoteID << " not started!\n";
			return 0;
		}
	}
	cout << "SensorDevices: SensorMote with ID = " << sensorMoteID << " does not exist!\n";
	return 0;
}

int SensorDevicesExample::getMeasurements (string sensorMoteID)
{
	int index = findSensorMoteByID(sensorMoteID);
	if (index >= 0)
	{
		moteDetails mote = getSensorMoteDetails(index);
		//get new measurements from sensor mote with ID = mote.smoteInfo.sensorMoteID
		//this is just an example
		vector<sensor>::iterator it;
		for (it = mote.sensorsInfo.begin(); it != mote.sensorsInfo.end(); it++)
		{
			//query for measurements from each sensor
		}
		cout << "SensorDevices: SensorMote with ID = " << mote.smoteInfo.sensorMoteID << " queries for measurements!\n";
		return 1;
	}
	cout << "SensorDevices: SensorMote with ID = " << sensorMoteID << " does not exist!\n";
	return 0;
}
