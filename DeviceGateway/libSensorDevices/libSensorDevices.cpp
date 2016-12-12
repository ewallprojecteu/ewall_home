/*=================================================================================
Basic explanation:

    Device Gateway cpp code

Authors: Daniel Denkovski (UKIM) (danield@feit.ukim.edu.mk)

Revision History:
    25.08.2014, ver 0.1
====================================================================================*/

#include "libSensorDevices.h"
#include "json_spirit_dgw.h"

inline ISensorDevices::~ISensorDevices()
{
}

SensorDevices::~SensorDevices()
{
	mySensorMotes.clear();
	reportingPeriods.clear();	
}

SensorDevices::SensorDevices()
{
	mySensorMotes.clear();
	reportingPeriods.clear();
}

int SensorDevices::initialize(vector<moteDetails> sensorMotes)
{
	if (sensorMotes.size() > 0)
	{
		vector<moteDetails>::iterator it;
		it = sensorMotes.begin();
		string connStr = (it)->smoteInfo.connectionStr;
		for (it = sensorMotes.begin() + 1; it != sensorMotes.end(); it++)
		{
			if ((it)->smoteInfo.connectionStr != connStr)
			{
				cout << "SensorDevices: Initialization failed. All motes should share the same physical connection!\n";
				return 0;
			}
		}
		mySensorMotes = sensorMotes;	
		reportingPeriods.resize(mySensorMotes.size(),0);
		cout << "SensorDevices: Initialization success. " << mySensorMotes.size() << " sensor motes registered!\n";
	}
	else
	{
		cout << "SensorDevices: Initialization failed. No motes provided!\n";
		return 0;
	}
	return 1;
}

int SensorDevices::addSensorMote(moteDetails sensorMote)
{
	vector<moteDetails>::iterator it;
	it = mySensorMotes.begin();
	int dist = distance(mySensorMotes.begin(), it);
	string connStr = "";
	if (dist > 0)
	         connStr = (it)->smoteInfo.connectionStr;

	if (connStr != sensorMote.smoteInfo.connectionStr) 
	{
		cout << "SensorDevices: Adding failed. All motes should share the same physical connection!\n";
		return 0;		
	}
	else
	{	
		int index = findSensorMoteByID(sensorMote.smoteInfo.sensorMoteID);
		if (index >= 0)
		{
			moteDetails mote = *(mySensorMotes.begin() + index);
			cout << "SensorDevices: SensorMote with ID = " << mote.smoteInfo.sensorMoteID << " already exists!\n";		
			return 0;		
		}
		else
		{
			mySensorMotes.push_back(sensorMote);
			cout << "SensorDevices: SensorMote with ID = " << sensorMote.smoteInfo.sensorMoteID << " added!\n";
			return 1;
		}
	}
	return 0;	
}

int SensorDevices::deleteSensorMote(string sensorMoteID)
{
	int index = findSensorMoteByID(sensorMoteID);
	if (index >= 0)
	{
		moteDetails mote = *(mySensorMotes.begin() + index);
		stopMeasuring(mote.smoteInfo.sensorMoteID);	
		mySensorMotes.erase(mySensorMotes.begin() + index);	
		cout << "SensorDevices: SensorMote with ID = " << sensorMoteID << " deleted!\n";		
		return 1;		
	}
	else
		cout << "SensorDevices: Deleting failed. No sensor mote with ID = " << sensorMoteID << "!\n";
	return 0;	
}
	
int SensorDevices::registerToDGW(IDeviceGateway *dgw)
{
	devGW = dgw;
	cout << "SensorDevices: Registering to DeviceGateway!\n";
	return 1;
}

int SensorDevices::deregisterFromDGW()
{
	devGW = NULL;
	cout << "SensorDevices: DeRegistering to DeviceGateway!\n";
	return 1;
}

int SensorDevices::pushMeasurements (string sensorMoteID, vector<measurement> measurements)
{
	int index = findSensorMoteByID(sensorMoteID);
	if (index >= 0)
	{
		moteDetails mote = *(mySensorMotes.begin() + index);
		cout << "SensorDevices: SensorMote with ID = " << mote.smoteInfo.sensorMoteID << " pushing measurements to DGw!\n";
		return devGW->pushMeasurements(mote.smoteInfo.sensorMoteID, measurements);		
	}
	cout << "SensorDevices: SensorMote with ID = " << sensorMoteID << " does not exist!\n";
	return 0;
}

vector<sensor> SensorDevices::getSensors (string sensorMoteID)
{
	vector<sensor> sensorsPerMote;
	int index = findSensorMoteByID(sensorMoteID);
	if (index >= 0)
	{
		moteDetails mote = *(mySensorMotes.begin() + index);
		sensorsPerMote = mote.sensorsInfo;
		cout << "SensorDevices: SensorMote with ID = " << mote.smoteInfo.sensorMoteID << " reporting sensors information to DGw!\n";
		return sensorsPerMote;	
	}
	cout << "SensorDevices: SensorMote with ID = " << sensorMoteID << " does not exist!\n";
	return sensorsPerMote;
}

moteInfo SensorDevices::getSensorMoteInfo (string sensorMoteID)
{
	moteInfo moteinformation;
	int index = findSensorMoteByID(sensorMoteID);
	if (index >= 0)
	{
		moteDetails mote = *(mySensorMotes.begin() + index);
		moteinformation = mote.smoteInfo;
		cout << "SensorDevices: SensorMote with ID = " << mote.smoteInfo.sensorMoteID << " reporting moteInfo to DGw!\n";
		return moteinformation;	
	}
	cout << "SensorDevices: SensorMote with ID = " << sensorMoteID << " does not exist!\n";
	return moteinformation;
}

float SensorDevices::getRate(string sensorMoteID)
{
	int index = findSensorMoteByID(sensorMoteID);
	if (index >= 0)
	{
		float repPeriod = *(reportingPeriods.begin() + index);
		return repPeriod;		
	}
	cout << "SensorDevices: SensorMote with ID = " << sensorMoteID << " does not exist!\n";
	return -1;
}

int SensorDevices::setRate(string sensorMoteID, float repPeriod)
{
	int index = findSensorMoteByID(sensorMoteID);
	if (index >= 0)
	{
		*(reportingPeriods.begin() + index) = repPeriod;
		return 1;		
	}
	return -1;
}

vector<moteDetails> SensorDevices::getAllSensorMotesDetails()
{
	return mySensorMotes;
}

int SensorDevices::findSensorMoteByID(string sensorMoteID)
{
	vector<moteDetails>::iterator it;
	for (it = mySensorMotes.begin(); it != mySensorMotes.end(); it++)
	{
		if ((it)->smoteInfo.sensorMoteID == sensorMoteID)
		{
			int index = (it - mySensorMotes.begin());
			return index;
		}
	}
	return -1;
}

moteDetails SensorDevices::getSensorMoteDetails(int index)
{
	moteDetails mote;
	if (index >= 0)
	{
		mote = *(mySensorMotes.begin() + index);
	}
	return mote;
}

int SensorDevices::setConfigurationFromFile(string fileName)
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
	vector<moteDetails> moteDevs = JsonToVecMoteDetails(jsonStr);
	
    	if (moteDevs.size() > 0)
    	{
		vector<moteDetails> details;
		details.insert(details.end(),moteDevs.begin(),moteDevs.end());
		initializeDevice(details);
		return 1;	
    	}
    }
    cout << "Failed to configure\n";

    return 0;  
}
