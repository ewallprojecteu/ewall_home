/*=================================================================================
Description: Sends messages to couchdb

Authors: Nikolaos Katsarakis (AIT) (nkat@ait.edu.gr)

Revision History:
    13.11.2014, ver 0.1
====================================================================================*/

#include "Communication.h"
#include <sstream>
#include <cmath>
#include "time_funcs.h"
#include "curl_helper.h"

using namespace std;

Communication::Communication(string url)
{
	curl_init(url.c_str());
	curl_set_debug_level(0);
	_url = url;
	sentActive=false;
	sentLuminance=-1;
}

Communication::~Communication()
{
	curl_cleanup();
}


string createMessage(long long timestamp, bool active, double luminance)
{
	char timeString[50];
	millis2string(timestamp, timeString, sizeof(timeString)/sizeof(char));
	stringstream msgStream;
	msgStream << "{\n";
	msgStream << "\"_id\":" << "\"" << timeString << "\",\n" ;
	msgStream << "\"timestamp\":" << "\"" << timeString << "\",\n" ;
	msgStream << "\"movement\":" << (active?"1":"0") << ",\n" ;
	msgStream << "\"luminance\":" << luminance << "\n" ;
	msgStream << "}\n";
	return msgStream.str() ;
}

int Communication::sendMessage(long long timestamp, bool active, double luminance)
{
	if (sentLuminance<0 || active!=sentActive || fabs(luminance-sentLuminance)>5)
	{
		string msg = createMessage(timestamp,active,luminance);
		cout << endl << "--- sending ---\n" << msg << endl;
		sentActive=active;
		sentLuminance=luminance;
		curl_send(msg.c_str(), msg.size());
	}
	return 0;
}
