#include <iostream>
#include "libDGwClientSensors.h"
#include "DGWdefs.h"
#include "time_funcs.h"
#include "curl_helper.h"

#ifndef _WIN32
#include <unistd.h> // For usleep()
#endif /*_WIN32*/

using namespace std;



int main(int argc, char *argv[])
{
	string serverName = "localhost";
	// Parse command arguments
	if (argc !=2)
	{
		cout << "Tool for retrieving sensor data from Device Gateway and sending to couchdb \n\n";
		cout << "Usage:\n \tsensorsToDB couchdb_url\n\n";
		cout << "Parameters:\n";
		cout << "\tcouchdb_url : Base URL to an existing CouchDB server to store metadata,\n";
		cout << "\t\t\tdisabled if set to 'localhost'\n\n";
		cout << "\t\t\tthe databases should be already existing before\n\n";
		cout << "Example:\nsensorsToDB http://localhost:5984/' \n";
		return 0;
	}
	serverName = argv[1];

	DGwClientSensors sensorProcess;
	sensorProcess.initializeDGwClientSensors(serverName);

	//Main sensor reading loop
	//vector<measurement> data;
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



