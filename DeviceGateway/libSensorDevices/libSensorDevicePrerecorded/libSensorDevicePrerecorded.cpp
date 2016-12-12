/*=================================================================================
Basic explanation:

    Device Gateway cpp code

Authors: Daniel Denkovski (UKIM) (danield@feit.ukim.edu.mk)

Revision History:
    25.08.2014, ver 0.1
====================================================================================*/

#include "libSensorDevicePrerecorded.h"
#include "time_funcs.h"
#include "json_spirit_dgw.h"
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <sstream>
#include <iostream>

pthread_t hthreadPrRec;

inline void *threadRunPrRec(void *object)
{
	SensorMotePrerecorded* mysm = (SensorMotePrerecorded*)object;
	vector<moteDetails> motes = mysm->getAllSensorMotesDetails();
	string name = motes[0].smoteInfo.sensorMoteID;	
	while (mysm->getRate(name) > 0)
	{
		mysm->pushMeasurements(name, mysm->getMeasurementsPR(name));
		usleep((unsigned int)(mysm->getRate(name)*1000000));
	}
}

SensorMotePrerecorded::~SensorMotePrerecorded()
{
}

SensorMotePrerecorded::SensorMotePrerecorded()
{
}


int SensorMotePrerecorded::initializeDevice(vector<moteDetails> sensorMotes)
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

int SensorMotePrerecorded::startMeasuring (string sensorMoteID, float reportPeriod)
{
	int index = findSensorMoteByID(sensorMoteID);
	if (index >= 0)
	{
		moteDetails mote = getSensorMoteDetails(index);
		float repPeriod = getRate(sensorMoteID);

		if (repPeriod == 0)
		{	
			//repPeriod = reportPeriod;
			setRate(sensorMoteID,reportPeriod);
			cout << "SensorDevices: SensorMote with ID = " << mote.smoteInfo.sensorMoteID << " started measurements with period of " << getRate(sensorMoteID) << "!\n";
			//start periodic measurement on sensor mote with ID = mote.smoteInfo.sensorMoteID
			pthread_create(&hthreadPrRec, NULL, &threadRunPrRec, this);
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

int SensorMotePrerecorded::reconfigure (string sensorMoteID, float reportPeriod)
{
	int index = findSensorMoteByID(sensorMoteID);
	if (index >= 0)
	{
		moteDetails mote = getSensorMoteDetails(index);
		float repPeriod = getRate(sensorMoteID);

		if (repPeriod > 0)
		{
			//repPeriod = reportPeriod;
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

int SensorMotePrerecorded::stopMeasuring (string sensorMoteID)
{
	int index = findSensorMoteByID(sensorMoteID);
	if (index >= 0)
	{
		moteDetails mote = getSensorMoteDetails(index);
		float repPeriod = getRate(sensorMoteID);

		if (repPeriod > 0)
		{	
			//repPeriod = 0;
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

vector<measurement> SensorMotePrerecorded::getMeasurementsPR (string sensorMoteID)
{
	vector<measurement> newMeasurements;
	int index = findSensorMoteByID(sensorMoteID);
	if (index >= 0)
	{
		moteDetails mote = getSensorMoteDetails(index);
		//get new measurements from sensor mote with ID = mote.smoteInfo.sensorMoteID
		//this is just an example
		if (!dataFile.is_open())
		{
			return newMeasurements;
		}

		long long timedata;
		double data[mote.sensorsInfo.size()];
		if (!(dataFile >> timedata))
		{
			dataFile.close();
			return newMeasurements;
		}
		else
		{
			vector<sensor>::iterator it;
			for (it = mote.sensorsInfo.begin(); it != mote.sensorsInfo.end(); it++)
			{
				double data = 0;
				if (!(dataFile >> data))
				{
					dataFile.close();
					return newMeasurements;
				}
				else
				{
					newMeasurements.push_back(measurement(mote.smoteInfo.sensorMoteID, (*it).type, data, timedata));
				}
			}
		}
		cout << "SensorDevices: SensorMote with ID = " << mote.smoteInfo.sensorMoteID << " reports new measurements!\n";
		return newMeasurements;
	}
	cout << "SensorDevices: SensorMote with ID = " << sensorMoteID << " does not exist!\n";
	return newMeasurements;
}

int SensorMotePrerecorded::openDataFile(string fileName)
{
	if (!dataFile.is_open())
	{
		dataFile.open(fileName.c_str());
		if (!dataFile.is_open())
			return 0;
	}
	return 1;
}

int SensorMotePrerecorded::closeDataFile()
{
	if (dataFile.is_open())
	{
		dataFile.close();
		return 1;
	}
	else
		return 0;
}
