/*=================================================================================
Basic explanation:

    Accelerometer processing cpp code

Authors: Aristodemos Pnevmatikakis (AIT) (apne@ait.edu.gr)
         Nikolaos Katsarakis (nkat@ait.edu.gr)
         Daniel Denkovski (UKIM) (danield@feit.ukim.edu.mk)

Revision History:
    30.09.2014, ver 0.1
====================================================================================*/

#include "libDGwAccelerometer.h"
#include <string.h>
#include <stdio.h>
#include "curl_helper.h"
#include "time_funcs.h"
#include "json_spirit_dgw.h"
#include <cmath>


// Activity type recognition model file names
const string KSVMScaleData("../../models/KSVM_scale_range.yml");
const string SVMModelData("../../models/SVMModel.model");

// available activities
string activities[] = {"no data", "resting", "walking", "exercising", "running"};


long long steps = 0;

/* Quick-and-dirty JSON parsing of simple numerical values */
int getJSONdouble(const string& JSONstr, const string& attributeName, double *value)
{
	size_t pos = JSONstr.find("\""+attributeName+"\"");
	if (pos == string::npos)
		return -1;

	size_t startPos = JSONstr.find(":",pos+attributeName.size()+2);
	if (startPos == string::npos)
		return -1;
	startPos++;//Remove separating ':'

	double result;
	string dat=JSONstr.substr(startPos);
	stringstream convert(dat);
	if ( !(convert >> result) )
		return -1;
	*value=result;
	return 0;
}

int readDoubleFromDB(const std::string& attributeName, double *res)
{
	const string suffix = "/_all_docs?limit=1&descending=true&include_docs=true";
	string previousURL =  curl_getURL();
	string newURL = previousURL + suffix;
	curl_setURL(newURL.c_str());
	char recv[10000];
	curl_receive(recv, sizeof(recv)*sizeof(char));
	string received = string(recv);
	curl_setURL(previousURL.c_str());
	return getJSONdouble(received, attributeName, res);
}

DGwAccelerometer::~DGwAccelerometer()
{
    delete ActRec;
}

DGwAccelerometer::DGwAccelerometer()
{
}

int DGwAccelerometer::initializeDGwClientAcc(int reportInt=10000, const string& servName=string())
{
	this->initializeDGwClient();
	postData = false;

	//Initialise connection to couchdb server
	if (servName.compare("localhost")!=0)
	{
		cout << "Initialising server " << servName << endl;
		// If valid server, open a curl connection
		if (curl_init(servName.c_str())!=0)
		{
			cout << "Could not connect to couchdb server: " << servName << endl;
			return -1;
		}
		curl_set_debug_level(0); //Set debug level to 0 to avoid printing responses
		postData = true;
		double stepsD;
		if (readDoubleFromDB("steps",&stepsD)==0)
			steps=stepsD;
		else
			cout << "\n********\nWARNING:  Reading from DB failed, setting default steps = " << steps << "\n********\n";
	}
	serverName = servName;
	reportingInterval = reportInt;
	
    // Init Algorithm
    vector <string> modelFiles;
	modelFiles.push_back(KSVMScaleData);
	modelFiles.push_back(SVMModelData);
	ActRec = new cTUSAccActRec(modelFiles,500);

    
    cout << "DGwAccelerometer: DGwAccelerometer initialized!, steps = " << steps << endl;
	return 1;
}

int DGwAccelerometer::pushMeasurements (string sensorMoteID, string measurements)
{
	vector <measurement> data = JsonToMeasurements(measurements);
	if ((data.size() == 3) && (data[0].sensType == _ACCELEROMETER_X) && (data[1].sensType == _ACCELEROMETER_Y) && (data[2].sensType == _ACCELEROMETER_Z))
	{
		//cout << "DGwAccelerometer: SensorMote with ID = " << sensorMoteID << " pushes measurements to nonAV processing!\n" << measurements << "\n";
		processMeasurement(data);
	}
	return 1;
}

//Structure for peak detector
typedef struct extremum{
	double value;
	long long timestamp;
	int validity;
	extremum(double val = 0, long long ts = -INT_MAX, int valid = -1): value(val), timestamp(ts), validity(valid) {}
} extremum;

vector<double> loadVector(const string& fileName=string())
{
	vector<double> nums;
//	cout << "loading " << fileName << endl;
	ifstream dataFile(fileName.c_str());
	double data;
	// Try to open it and return empty vector on failure
	if (!dataFile.is_open())
		return nums;
	while (dataFile >> data)
		nums.push_back(data);
//	cout << "loaded " << nums.size() << " values" << endl;
	return nums;
}

double symmetricalFIR(const vector<double>& coeffs, const vector<double>& filtertap)
{
	int csize = coeffs.size(), fsize= filtertap.size();
	double y=0;
	if (csize==0 || fsize==0 || fsize != 2*(csize-1) +1)
	{
		cout << "error in FIR filter" << endl;
		return -1;
	}
	//Calculate the symmetric components
	for (int i=0;i<csize-1;i++)
		y+=(filtertap[i]+filtertap[fsize-1-i])*coeffs[i];
	//Add the central component
	y+=filtertap[csize-1]*coeffs[csize-1];
	return y;
}

int stepCounter(double a, double a0, long long timestamp)
{
	static extremum minimum(a,timestamp,-1), maximum(a,timestamp,-1);
	double verifyThr=0.5, extremThr=2;
	static double previousMax = -INT_MAX;

	if (a>maximum.value && a0 < a)
		maximum=extremum(a,timestamp,0);
	if (a<minimum.value && a0 > a)
		minimum=extremum(a,timestamp,0);
	if (maximum.validity == -1 && a<maximum.value)
		maximum=extremum(a,timestamp,-1);
	if (minimum.validity == -1 && a>minimum.value)
		minimum=extremum(a,timestamp,-1);
	if (minimum.validity == 0 && a>minimum.value+verifyThr)
		minimum.validity=1;
	if (maximum.validity == 0 && a<maximum.value-verifyThr)
		maximum.validity=1;
	if (minimum.validity == 1 && maximum.validity==1 && (maximum.value-minimum.value>extremThr || previousMax-minimum.value>extremThr))
	{
		previousMax=maximum.value;
		minimum=maximum=extremum(a,timestamp,-1);
		steps++;
	}
	return steps;
}

// Returns -1 on error, 0 on ok, 1 on finish
int DGwAccelerometer::processMeasurement(const vector<measurement>& data)
{
	static long long timestamp0, reportedTime=-1;
	static int counter=0;
	static double accel0[3], accelM[3]={0,0,0}, accelM2[3]={0,0,0}, accelV[3], g[3]={-INT_MAX,-INT_MAX,-INT_MAX}, minVar=.5, IMA=0, ISA=0, alpha=0.95;
	static double accelFiltered[2];
	static int tapLength;
	static vector<double> coeffs;
	static vector<double> filterTap;
	double accel;

	cout << "timestamp: " << data[0].timestamp << ", accel= [" << data[0].value <<", " << data[1].value << "," << data[2].value << "]\n";
	
	if (counter++==0)
	{
		coeffs = loadVector("filterCoeffs.txt");
		if (coeffs.size()==0)
		{
			cout << "filterCoeffs.txt could not be loaded, please make sure it is in the same folder as the executable\n";
			return -1;
		}
		tapLength = (coeffs.size() - 1) * 2 + 1;
		filterTap.resize(tapLength);
		// Initial acceleration and time values
		accel0[0]=data[0].value;
		accel0[1]=data[1].value;
		accel0[2]=data[2].value;
		timestamp0 = reportedTime = data[0].timestamp;
	}
	//Fill in the filter taps
	filterTap.erase(filterTap.end()-1);
	if (g[0]==-INT_MAX)
		accel = sqrt(data[0].value*data[0].value + data[1].value*data[1].value + data[2].value*data[2].value);
	else
		accel = sqrt((data[0].value-g[0])*(data[0].value-g[0]) + (data[1].value-g[1])*(data[1].value-g[1]) + (data[2].value-g[2])*(data[2].value-g[2]));
	filterTap.insert(filterTap.begin(), accel);

	// Estimate the initial mean and variance, after waiting for the accelerometer to be idle (low variance)
	for (int i=0; i<3; i++)
	{
		accelM[i]=alpha*accelM[i] + (1-alpha)*data[i].value;
		accelM2[i]=alpha*accelM2[i] + (1-alpha)*data[i].value*data[i].value;
		accelV[i]=accelM2[i]-accelM[i]*accelM[i];
	}

	// Keep reading values as long as we keep getting triples
	if (data.size()!=3)
	{
		if (counter<100)
		{
			cout << "Could not get enough data for initialization after reading " << counter << " measurements\n";
			return -1;
		}
		else
		{
			cout << "Read " << counter << " measurements\n";
			return 1;
		}
	}
	// Skip further processing until the accelerometer is idle (low variance) or timeout has elapsed
	if ((counter<=5 || accelV[0]>minVar || accelV[1]>minVar || accelV[2]>minVar) && counter < 100)
	{
		cout << "Waiting for idle time, counter = " << counter << ", accelV[" << accelV[0] << "," << accelV[1] << "," << accelV[2] << "]\n";
		return 0;
	}
	if (g[0]==-INT_MAX)
	{
		// Initial gravity estimation is the acceleration mean after stabilization
		for (int i=0; i<3; i++)
			g[i] = accelM[i];
	}
	// Estimate gravity as the slowly vaying component of the acceleration
	for (int i=0; i<3; i++)
	{
		if (accelV[i]<minVar)
			g[i] = alpha*g[i] + (1-alpha)*accel0[i];
		accel0[i] -= g[i];
	}

	//Filter the acceleration
	accelFiltered[counter%2]=symmetricalFIR(coeffs,filterTap);
	stepCounter(accelFiltered[counter%2],accelFiltered[(counter-1)%2], data[0].timestamp);

    //Recognize user activity
    ActRec->Apply((double*)&data[0].value,(double*)&data[1].value,(double*)&data[2].value, data.size()/3);
    
	//If not enough time passed from previous report, update counters
	if (timestamp0-reportedTime<reportingInterval)
	{
		IMA += (fabs(accel0[0]) + fabs(accel0[1]) + fabs(accel0[2])) * (data[0].timestamp - timestamp0);
		ISA += sqrt(accel0[0]*accel0[0] + accel0[1]*accel0[1] + accel0[2]*accel0[2]) * (data[0].timestamp - timestamp0);
	}
	else
	{
		//Calculate the average by dividing with the elapsed time
		IMA /= (timestamp0 - reportedTime);
		ISA /= (timestamp0 - reportedTime);

        Activity_t activity = ActRec->GetActivity();
        
		char datetime[30];
		millis2string(reportedTime, datetime, sizeof(datetime)); 
		cout << "time = " << datetime << ", IMA = " << IMA << ", ISA = " << ISA << ", steps = " << steps << ", posting=" << postData <<endl;
		if (postData)
		{
			//Prepare JSON message
			stringstream msg;
			msg << "{\n";
			msg << "\t\"_id\" : \"" << datetime << "\",\n";
			msg << "\t\"timestamp\" : \"" << datetime << "\",\n";
			msg << "\t\"activity\" : {\n";
			msg << "\t\t\"IMA\" : "  << IMA << ",\n";
			msg << "\t\t\"ISA\" : "  << ISA << ",\n";
			msg << "\t\t\"steps\" : "  << steps << "\n";
			msg << "\t\t\"physicalActivity\" : "  << activities[(int)ActRec->GetActivity()] << "\n";
			msg << "\t}\n";
			msg << "}";
			cout << msg.str() << endl;
			curl_send(msg.str().c_str(), msg.str().size());
		}

		//Reset the counters after reporting
		reportedTime = timestamp0;
		IMA = (fabs(accel0[0]) + fabs(accel0[1]) + fabs(accel0[2])) * (data[0].timestamp - timestamp0);
		ISA = sqrt(accel0[0]*accel0[0] + accel0[1]*accel0[1] + accel0[2]*accel0[2]) * (data[0].timestamp - timestamp0);
	}

	// The next sensor values become the current ones
	timestamp0 = data[0].timestamp;
	accel0[0]=data[0].value;
	accel0[1]=data[1].value;
	accel0[2]=data[2].value;
	return 0;
}

