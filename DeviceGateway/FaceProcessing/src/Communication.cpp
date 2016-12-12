/*=================================================================================
Description: Sends messages to couchdb

Authors: Nikolaos Katsarakis (AIT) (nkat@ait.edu.gr)

Revision History:
    13.11.2014, ver 0.1
====================================================================================*/

#include "Communication.h"
#include <sstream>
#include <cmath>
#include <climits>
#include "time_funcs.h"
#include "curl_helper.h"

using namespace std;

Communication::Communication(string url)
{
	_url = url;
	if (url.compare("localhost")!=0)
	{
		curl_init(_url.c_str());
		curl_set_debug_level(0);
		sending = true;
	}
	else
		sending = false;
	lastSentTargets.reserve(20);
	sentTime = LLONG_MIN;
}

Communication::~Communication()
{
	if (sending)
		curl_cleanup();
}


string createMessage(int64 frameTime, const vector<targetStruct>& targets)
{
	char timeString[50];
	millis2string(frameTime, timeString, sizeof(timeString)/sizeof(char));
	stringstream msgStream;
	msgStream << "{\n";
	msgStream << "\t\"_id\":" << "\"" << timeString << "\",\n" ;
	msgStream << "\t\"timestamp\":" << "\"" << timeString << "\",\n" ;
	msgStream << "\t\"people\":[\n" ;
	for (unsigned int i=0;i<targets.size();i++)
	{
		msgStream << "\t\t{\n" ;
		msgStream << "\t\t\t\"id\" : " << targets[i].target_id << ",\n";
		msgStream << "\t\t\t\"x\" : " << targets[i].face.center.x << ",\n";
		msgStream << "\t\t\t\"y\" : " << targets[i].face.center.y << ",\n";
		msgStream << "\t\t\t\"width\" : " << targets[i].face.size.width << ",\n";
		msgStream << "\t\t\t\"height\" : " << targets[i].face.size.height << ",\n";
		msgStream << "\t\t\t\"positionconf\" : " << 1-targets[i].spread << ",\n";
		string gender = targets[i].gender < 0.5 ? "MALE":"FEMALE";
		double genderConf = 2*abs(targets[i].gender-0.5);
		msgStream << "\t\t\t\"gender\" : \"" << gender << "\",\n";
		msgStream << "\t\t\t\"genderconf\" : " << genderConf << ",\n";
		msgStream << "\t\t\t\"age\" : " << targets[i].ageEst << ",\n";
		msgStream << "\t\t\t\"ageconf\" : " << targets[i].ageEstConf << ",\n";
		msgStream << "\t\t\t\"emotion\" : \"" << targets[i].emotion << "\",\n";
		msgStream << "\t\t\t\"emotionconf\" : " << targets[i].emotionConf << "\n";
		msgStream << "\t\t}" ;
		if (i<targets.size()-1)
			msgStream << ",";
		msgStream << "\n" ;
	}
	msgStream << "\t]\n";
	msgStream << "}\n";
	return msgStream.str() ;
}

bool changedTargets(const vector<targetStruct>& sentTargets,  const vector<targetStruct>& newTargets)
{
	const double minSizeRatio = 0.8, maxSizeRatio = 1.2, moveRatio = 0.8;

	/*Detect significant change in targets*/
	if  (sentTargets.size() != newTargets.size())
		return true;

	for (unsigned int i=0;i<sentTargets.size();i++)
	{
		if (sentTargets[i].target_id != newTargets[i].target_id)
			return true;
		cv::Size2f meanSize = sentTargets[i].face.size + newTargets[i].face.size;
		meanSize.width/=2; meanSize.height/=2;

		float widthRatio = newTargets[i].face.size.width/meanSize.width;
		if (widthRatio < minSizeRatio || widthRatio > maxSizeRatio)
			return true;

		float heightRatio = newTargets[i].face.size.width/meanSize.height;
		if (heightRatio < minSizeRatio || heightRatio > maxSizeRatio)
			return true;

		// Check if target has moved significantly, compared to its size
		double dist = norm(newTargets[i].face.center - sentTargets[i].face.center);
		if (dist > moveRatio*(meanSize.width + meanSize.height)/2)
			return true;
		
		// Check if face analytics have changed
		//if (strcmp(newTargets[i].emotion.c_str(), sentTargets[i].emotion.c_str()) != 0)
		if (newTargets[i].emotion.compare(sentTargets[i].emotion) != 0)
			return true;
		if (fabs(newTargets[i].emotionConf - sentTargets[i].emotionConf) / newTargets[i].emotionConf >= 0.1)
			return true;

		// Check if face size has changed (maybe already covered by lines 92-98, but does not hurt to repeat)
		if (fabs(newTargets[i].face.size.width - sentTargets[i].face.size.width) / newTargets[i].face.size.width >= 0.1)
			return true;

	}

	return false;
}

int Communication::sendMessage(int64 frameTime, const vector<targetStruct>& targets)
{
	if (changedTargets(lastSentTargets, targets) || sentTime == LLONG_MIN /*Send first message*/)
	{
		string msg = createMessage(frameTime, targets);
		cout << endl << "--- sending ---\n" << msg << endl;
		if (sending)
			curl_send(msg.c_str(), msg.size());
		/* Store last sent state */
		lastSentTargets = targets;
		sentTime = frameTime;
	}
	return 0;
}
