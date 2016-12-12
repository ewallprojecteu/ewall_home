/*=================================================================================
Basic explanation:

Authors: Daniel Denkovski (UKIM) (danield@feit.ukim.edu.mk)

Revision History:
    22.09.2014, ver 0.1
====================================================================================*/

#ifndef LIBDGWCLIENTSENSORS_H
#define LIBDGWCLIENTSENSORS_H

#include "libDGwClient.h"

class DGwClientSensors: public DGwClient
{
public:

	int initializeDGwClientSensors(const string& servBaseName);

	DGwClientSensors();
	
	~DGwClientSensors();

	int pushMeasurements (string sensorMoteID, string measurements);

	int saveMeasurements(string sensorMoteID, const vector<measurement>& data);


private:
	bool postData; // Whether the data will be posted to couchdb server
	string serverBaseName;
};


#endif
