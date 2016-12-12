#include <iostream>
#include <fstream>
#include <sstream>
#include <climits>
#include <cstdlib>
#include "libDGwAccelerometer.h"
#include "DGWdefs.h"
#include "time_funcs.h"
#include "curl_helper.h"

#ifndef _WIN32
#include <unistd.h> // For usleep()
#endif /*_WIN32*/

using namespace std;

bool postData = false; // Whether the data will be posted to couchdb server
int reportingInterval=10000;	// in ms
string serverName = "localhost";


// This function loads an accelerometer data file and is designed to be compatible 
// with the corresponding function of the device gateway
// Each call reads the timestamp and 3 double-precision numbers from the given file
// Input:
//    file name to read, will be opened on first successful call and ignored it on subsequent calls
// Output:
//    vector of measurements, will be an empty vector if an error occured

vector<measurement> getMeasurements(const string& fileName=string())
{
	vector<measurement> measurements;
	static ifstream dataFile;
	if (!dataFile.is_open())
	{
		cout << "opening " << fileName << endl;
		dataFile.open(fileName.c_str());
		// Try to open it and return empty vector on failure
		if (!dataFile.is_open())
			return measurements;
	}
	long long timedata;
	double data[3];
	if (!(dataFile >> timedata >> data[0] >> data[1] >> data[2]))
	{
		dataFile.close();
		return measurements;
	}
	measurements.resize(3);
	measurements[0]=measurement("",	_ACCELEROMETER_X ,data[0], timedata);
	measurements[1]=measurement("",	_ACCELEROMETER_Y ,data[1], timedata);
	measurements[2]=measurement("",	_ACCELEROMETER_Z ,data[2], timedata);
	return measurements;
}


int main(int argc, char *argv[])
{
	// Parse command arguments
	if (argc <2 || argc > 3)
	{
		cout << "Tool for processing accelerometer data from Device Gateway and reporting metadata\n\n";
		cout << "Usage:\n \taccelerometer [reporting_interval] [couchdb_url]\n\n";
		cout << "Parameters:\n";
		cout << "\treporting_interval : reporting interval in milliseconds,\n";
		cout << "\t\t\tdefaults to 10000 if invalid input or <=0\n";
		cout << "\tcouchdb_url : URL to an existing CouchDB database to store metadata,\n";
		cout << "\t\t\tdisabled if not specified or set to 'localhost'\n\n";
		cout << "Example:\naccelerometer -1 localhost:5984/activity/ \n";
		return 0;
	}
	if (argc > 2)
		serverName = argv[2];
	int numread = atoi(argv[1]);
	if (numread > 0)
		reportingInterval = numread;

	DGwAccelerometer accelProcess;
	cout << "initializing DGwAccelerometer with reportingInterval= " << reportingInterval << ", serverName= " << serverName << endl;
	accelProcess.initializeDGwClientAcc(reportingInterval,serverName);

	//vector<measurement> data;
	//Main sensor reading loop
	while (1)
	{
#ifndef _WIN32
		usleep(50000);
#endif /*_WIN32*/
		/*data = getMeasurements(argv[1]);
		int res = accelProcess.processMeasurement(data);
		if (res == -1)
		{
			cout << "Please ensure that the input file " << argv[1] << " is large enough\n";
			return -1;
		}
		else if (res == 1)
			break;*/
	}
	return 0;
}

