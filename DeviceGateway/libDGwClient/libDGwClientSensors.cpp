/*=================================================================================
Basic explanation:

    Device Gateway cpp code

Authors: Daniel Denkovski (UKIM) (danield@feit.ukim.edu.mk)

Revision History:
    22.09.2014, ver 0.1
====================================================================================*/

#include "libDGwClientSensors.h"
#include "CAdataAdaptor.h"
#include <string.h>
#include <stdio.h>
#include "curl_helper.h"
#include "time_funcs.h"
#include "json_spirit_dgw.h"
#include <cmath>

DGwClientSensors::~DGwClientSensors()
{
}

DGwClientSensors::DGwClientSensors()
{
}

int DGwClientSensors::initializeDGwClientSensors(const string& servBaseName=string())
{
	this->initializeDGwClient();
	postData = false;

	//Initialise connection to couchdb server
	if (servBaseName.compare("localhost")!=0)
	{
		cout << "Initialising server " << servBaseName << endl;
		// If valid server, open a curl connection
		if (curl_init(servBaseName.c_str())!=0)
		{
			cout << "Could not connect to couchdb server: " << servBaseName << endl;
			return -1;
		}
		curl_set_debug_level(0); //Set debug level to 0 to avoid printing responses
		postData = true;
	}
	serverBaseName = servBaseName;
	cout << "DGwClientSensors: DGwClientSensors initialized!\n";
	return 1;
}

int DGwClientSensors::pushMeasurements (string sensorMoteID, string measurements)
{
	vector <measurement> data = JsonToMeasurements(measurements);
	if (data.size()>0)
	{
		saveMeasurements(sensorMoteID, data);
	}
	//cout << "DGwClientSensors: SensorMote with ID = " << sensorMoteID << " pushes measurements to nonAV processing!\n" << measurements << "\n";
	return 1;
}

int DGwClientSensors::saveMeasurements(string sensorMoteID, const vector<measurement>& data)
{
	string moteID = sensorMoteID;
	cout << "saveMeasurements " << moteID << " , data length = "<< data.size() << endl;
	if (data.size()<0)
		return -1;
	int datasize=data.size();
	char datetime[30];
	millis2string(data[0].timestamp, datetime, sizeof(datetime));
	cout << "time = " << datetime << ", data length = " << datasize << endl;
	if (postData)
	{	
		//Prepare JSON message
		stringstream msg;
		msg << "{\n";
		msg << "\t\"_id\" : \"" << datetime << "\",\n";
		msg << "\t\"timestamp\" : \"" << datetime << "\",\n";
		for (int i=0;i<datasize;i++)
		{
			string name, unit, mote;
			int cadev = getSensorNameUnitMote(data[i].sensType, name, unit, mote);
			string outval;
			getSensorValueType(data[i].sensType, data[i].value, outval);
			if (cadev >= 0)
				msg << "\t\"" << name << "\" : " << outval;
			else
				cerr << "ignoring invalid sensor type " << data[i].sensType << endl;

			if (cadev == 1)
			{
				moteID = mote;//moteIDstr;
			}

			if (i<datasize-1)
				msg << ",\n";
			else
				msg << "\n";
		}
		msg << "}";
		cout << msg.str() << endl;
		string server = serverBaseName + moteID;
		cout << "sending to " << server << endl;
		curl_setURL(server.c_str());
		curl_send(msg.str().c_str(), msg.str().size());
	}
}
