/*=================================================================================
Basic explanation:

    Accelerometer processing cpp code

Authors: Aristodemos Pnevmatikakis (AIT) (apne@ait.edu.gr)
         Nikolaos Katsarakis (nkat@ait.edu.gr)
         Daniel Denkovski (UKIM) (danield@feit.ukim.edu.mk)

Revision History:
    30.09.2014, ver 0.1
====================================================================================*/

#ifndef LIBDGWACCELEROMETER_H
#define LIBDGWACCELEROMETER_H

#include "libDGwClient.h"

#include "cTUSAccActRec.hpp"

class DGwAccelerometer: public DGwClient
{
public:
	int initializeDGwClientAcc(int reportInt, const string& servName);

	DGwAccelerometer();
	
	~DGwAccelerometer();

	int pushMeasurements (string sensorMoteID, string measurements);

	int processMeasurement(const vector<measurement>& data);

private:
	bool postData; // Whether the data will be posted to couchdb server
	int reportingInterval;	// in ms
	string serverName;
    cTUSAccActRec *ActRec;

};


#endif
