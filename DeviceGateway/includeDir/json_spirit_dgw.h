/*=================================================================================
Basic explanation:

Authors: Daniel Denkovski (UKIM) (danield@feit.ukim.edu.mk)

Revision History:
    22.09.2014, ver 0.1
====================================================================================*/

#ifndef JSON_SPIRIT_DGW
#define JSON_SPIRIT_DGW

#include <string>
#include <fstream>
#include <iostream>
#include <stdlib.h>
#include "json_spirit.h"
#include "DGWdefs.h"

/*namespace json_spirit_dgw
{*/
	string streamConfigToJSON(streamConfig conf);

	string avDeviceInfoToJSON(avDeviceInfo devInfo);

	string vecAVDeviceInfoToJSON(vector<avDeviceInfo> devsInfos);

	string measurementsToJSON(vector<measurement> meass);

	string sensorsToJSON(vector<sensor> senss);

	string vecMoteDetailsToJSON(vector<moteDetails> moteDets);

	string moteInfoToJSON(moteInfo moteIn);

	streamConfig JsonToStreamConfig(string confStr);

	avDeviceInfo JsonToAVDeviceInfo(string devInfoStr);

	vector<avDeviceInfo> JsonToVecAVDeviceInfo(string devsInfosStr);
	
	vector<measurement> JsonToMeasurements(string measStr);

	vector<sensor> JsonToSensors(string senStr);
	
	vector<moteDetails> JsonToVecMoteDetails(string moteDetsStr);

	moteInfo JsonToMoteInfo(string moteInStr);
//};

#endif
