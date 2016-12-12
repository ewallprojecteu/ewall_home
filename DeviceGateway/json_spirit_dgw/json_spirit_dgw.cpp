/*=================================================================================
Basic explanation:

    Device Gateway cpp code

Authors: Daniel Denkovski (UKIM) (danield@feit.ukim.edu.mk)

Revision History:
    22.09.2014, ver 0.1
====================================================================================*/

#include "json_spirit_dgw.h"
#include "json_spirit_value.h"

using namespace std;
using namespace json_spirit;
//using namespace json_spirit_dgw;

string /*json_spirit_dgw::*/streamConfigToJSON(streamConfig conf)
{
    mObject objectToWrite;
    mArray resArray;
    resArray.push_back(conf.resolution.width);
    resArray.push_back(conf.resolution.height);
    objectToWrite["resolution"] = resArray;
    objectToWrite["nochs"] = conf.nochs;
    objectToWrite["rate"] = conf.rate;
    objectToWrite["format"] = conf.format;
    objectToWrite["stream"] = conf.stream;
    objectToWrite["bytesps"] = conf.bytesps;
    string confStr = write(objectToWrite, pretty_print);
    return confStr; 
}

string /*json_spirit_dgw::*/avDeviceInfoToJSON(avDeviceInfo devInfo)
{
    mObject objectToWrite;
    objectToWrite["deviceID"] = devInfo.deviceID;
    objectToWrite["serialNumber"] = devInfo.serialNumber;
    objectToWrite["deviceIF"] = devInfo.deviceIF;
    objectToWrite["connectionStr"] = devInfo.connectionStr;
    objectToWrite["room"] = devInfo.room;
    mArray locArr;
    locArr.push_back(devInfo.currentLoc.xPoint);
    locArr.push_back(devInfo.currentLoc.yPoint);
    locArr.push_back(devInfo.currentLoc.zPoint);
    objectToWrite["currentLoc"] = locArr;
    string devInfoStr = write(objectToWrite, pretty_print);
    return devInfoStr; 
}

string /*json_spirit_dgw::*/vecAVDeviceInfoToJSON(vector<avDeviceInfo> devsInfos)
{
    mArray devInfosArr;
    vector<avDeviceInfo>::iterator it;
    for (it = devsInfos.begin(); it != devsInfos.end(); it++)
    {
	    mObject objectToWrite;
	    objectToWrite["deviceID"] = (*it).deviceID;
	    objectToWrite["serialNumber"] = (*it).serialNumber;
	    objectToWrite["deviceIF"] = (*it).deviceIF;
	    objectToWrite["connectionStr"] = (*it).connectionStr;
	    objectToWrite["room"] = (*it).room;
	    mArray locArr;
	    locArr.push_back((*it).currentLoc.xPoint);
	    locArr.push_back((*it).currentLoc.yPoint);
	    locArr.push_back((*it).currentLoc.zPoint);
	    objectToWrite["currentLoc"] = locArr;
	    devInfosArr.push_back(objectToWrite);
    }
    string devInfosStr = write(devInfosArr, pretty_print);
    return devInfosStr; 
}

string /*json_spirit_dgw::*/measurementsToJSON(vector<measurement> meass)
{
    mArray measArr;
    vector<measurement>::iterator it;
    for (it = meass.begin(); it != meass.end(); it++)
    {
	    mObject objectToWrite;
	    objectToWrite["sensorMoteID"] = (*it).sensorMoteID;
	    objectToWrite["sensType"] = (*it).sensType;
	    objectToWrite["value"] = (*it).value;
	    objectToWrite["timestamp"] = (boost::int64_t)((*it).timestamp);
	    measArr.push_back(objectToWrite);
    }
    string measStr = write(measArr, pretty_print);
    return measStr; 
}

string /*json_spirit_dgw::*/sensorsToJSON(vector<sensor> senss)
{
    mArray sensArr;
    vector<sensor>::iterator it;
    for (it = senss.begin(); it != senss.end(); it++)
    {
	    mObject objectToWrite;
	    objectToWrite["type"] = (sensorType)(*it).type;
	    objectToWrite["parentMoteID"] = (*it).parentMoteID;
	    objectToWrite["siUnit"] = (*it).siUnit;
	    objectToWrite["minValue"] = (*it).minValue;
	    objectToWrite["maxValue"] = (*it).maxValue;
	    objectToWrite["accuracy"] = (*it).accuracy;
	    objectToWrite["offset"] = (*it).offset;
	    sensArr.push_back(objectToWrite);
    }
    string senStr = write(sensArr, pretty_print);
    return senStr;
}

string /*json_spirit_dgw::*/vecMoteDetailsToJSON(vector<moteDetails> moteDets)
{
    mArray moteArray;

    vector<moteDetails>::iterator it;
    for (it = moteDets.begin(); it != moteDets.end(); it++)
    {
	mObject objectToWrite;
	objectToWrite["sensorMoteID"] = (*it).smoteInfo.sensorMoteID;
	objectToWrite["serialNumber"] = (*it).smoteInfo.serialNumber;
	objectToWrite["deviceIF"] = (*it).smoteInfo.deviceIF;
	objectToWrite["connectionStr"] = (*it).smoteInfo.connectionStr;
	objectToWrite["macAddress"] = (*it).smoteInfo.macAddress;
	objectToWrite["room"] = (*it).smoteInfo.room;
	mArray locArr;
	locArr.push_back((*it).smoteInfo.currentLoc.xPoint);
	locArr.push_back((*it).smoteInfo.currentLoc.yPoint);
	locArr.push_back((*it).smoteInfo.currentLoc.zPoint);
	objectToWrite["currentLoc"] = locArr;

    	mArray sensorsToWrite;
	vector<sensor>::iterator its;
        for (its = (*it).sensorsInfo.begin(); its != (*it).sensorsInfo.end(); its++)
    	{
		mObject sensorObj1;
		sensorObj1["type"] = (*its).type;
		sensorObj1["parentMoteID"] = (*it).smoteInfo.sensorMoteID;
		sensorObj1["siUnit"] = (*its).siUnit;
		sensorObj1["minValue"] = (*its).minValue;
		sensorObj1["maxValue"] = (*its).maxValue;
		sensorObj1["accuracy"] = (*its).accuracy;
		sensorObj1["offset"] = (*its).offset;
		sensorsToWrite.push_back(sensorObj1);
    	}
    	objectToWrite["sensors"] = sensorsToWrite;
	moteArray.push_back(objectToWrite);
    }
    string moteArrayStr = write(moteArray, pretty_print);
    return moteArrayStr;
}

string /*json_spirit_dgw::*/moteInfoToJSON(moteInfo moteIn)
{
    mObject objectToWrite;
    objectToWrite["sensorMoteID"] = moteIn.sensorMoteID;
    objectToWrite["serialNumber"] = moteIn.serialNumber;
    objectToWrite["deviceIF"] = moteIn.deviceIF;
    objectToWrite["connectionStr"] = moteIn.connectionStr;
    objectToWrite["macAddress"] = moteIn.macAddress;
    objectToWrite["room"] = moteIn.room;
    objectToWrite["numSensors"] = moteIn.numSensors; 

    mArray locArr;
    locArr.push_back(moteIn.currentLoc.xPoint);
    locArr.push_back(moteIn.currentLoc.yPoint);
    locArr.push_back(moteIn.currentLoc.zPoint);
    objectToWrite["currentLoc"] = locArr;

    string moteInStr = write(objectToWrite, pretty_print);
    return moteInStr; 
}

streamConfig /*json_spirit_dgw::*/JsonToStreamConfig(string confStr)
{
    mValue value; 
    read(confStr,value);
    streamConfig conf;

    if (value.type() == obj_type) 
    {
	mObject &objRead = value.get_obj();
    	frameResolution res;
    	if (objRead.find("resolution")->second.type() == array_type) 
	{
		mArray &resArray = objRead.find("resolution")->second.get_array();
    		if (resArray[0].type() == int_type) 
			res.width = resArray[0].get_int();
    		if (resArray[1].type() == int_type) 
			res.height = resArray[1].get_int();
	}

    	conf.resolution = res;
    	if (objRead.find("nochs")->second.type() == int_type) 
		conf.nochs = (short)objRead.find("nochs")->second.get_int(); 
    	if (objRead.find("rate")->second.type() == int_type) 
		conf.rate = objRead.find("rate")->second.get_int();
    	if (objRead.find("format")->second.type() == int_type) 
		conf.format = (short)objRead.find("format")->second.get_int();
    	if (objRead.find("bytesps")->second.type() == int_type) 
		conf.bytesps = (short)objRead.find("bytesps")->second.get_int(); 
    	if (objRead.find("stream")->second.type() == int_type) 
		conf.stream = (short)objRead.find("stream")->second.get_int();
    }
    
    return conf;   
}

avDeviceInfo /*json_spirit_dgw::*/JsonToAVDeviceInfo(string devInfoStr)
{
    mValue value; 
    read(devInfoStr,value);
    avDeviceInfo devInfo;

    if (value.type() == obj_type) 
    {	
	mObject &objRead = value.get_obj();
    	location loc;
    	if (objRead.find("currentLoc")->second.type() == array_type) 
	{
		mArray &locArray = objRead.find("currentLoc")->second.get_array();
    		if ((locArray[0].type() == real_type) || (locArray[0].type() == int_type)) 
			loc.xPoint = locArray[0].get_real();
    		if ((locArray[1].type() == real_type) || (locArray[1].type() == int_type)) 
			loc.yPoint = locArray[1].get_real();
    		if ((locArray[2].type() == real_type) || (locArray[2].type() == int_type))
			loc.zPoint = locArray[2].get_real();
	}
    	if (objRead.find("deviceID")->second.type() == str_type) 
		devInfo.deviceID = objRead.find("deviceID")->second.get_str();
    	if (objRead.find("serialNumber")->second.type() == int_type) 
		devInfo.serialNumber = objRead.find("serialNumber")->second.get_int();
    	if (objRead.find("deviceIF")->second.type() == str_type) 
		devInfo.deviceIF = objRead.find("deviceIF")->second.get_str();
    	if (objRead.find("connectionStr")->second.type() == str_type) 
		devInfo.connectionStr = objRead.find("connectionStr")->second.get_str();
    	if (objRead.find("room")->second.type() == str_type) 
		devInfo.room = objRead.find("room")->second.get_str();
    	devInfo.currentLoc = loc;
    }

    return devInfo;   
}

vector<avDeviceInfo> /*json_spirit_dgw::*/JsonToVecAVDeviceInfo(string devsInfosStr)
{
    mValue value; 
    read(devsInfosStr,value);
    vector <avDeviceInfo> devInfos;

    if (value.type() == array_type)
    {
    	mArray &devsJson = value.get_array();
    	for (int i = 0; i < devsJson.size(); i++)
    	{
		avDeviceInfo devInfo;
        	if (devsJson[i].type() == obj_type) 
		{
			mObject &objRead = devsJson[i].get_obj();
    			location loc;
			if (objRead.find("currentLoc")->second.type() == array_type)
	    		{	
				mArray &locArray = objRead.find("currentLoc")->second.get_array();
	    			if ((locArray[0].type() == real_type) || (locArray[0].type() == int_type))
					loc.xPoint = locArray[0].get_real();
	    			if ((locArray[1].type() == real_type) || (locArray[1].type() == int_type))
					loc.yPoint = locArray[1].get_real();
	    			if ((locArray[2].type() == real_type) || (locArray[2].type() == int_type))
					loc.zPoint = locArray[2].get_real();
			}
	    		if (objRead.find("deviceID")->second.type() == str_type) 
				devInfo.deviceID = objRead.find("deviceID")->second.get_str();
	    		if (objRead.find("serialNumber")->second.type() == int_type) 
				devInfo.serialNumber = objRead.find("serialNumber")->second.get_int();
	    		if (objRead.find("deviceIF")->second.type() == str_type) 
				devInfo.deviceIF = objRead.find("deviceIF")->second.get_str();
	    		if (objRead.find("connectionStr")->second.type() == str_type) 
				devInfo.connectionStr = objRead.find("connectionStr")->second.get_str();
	    		if (objRead.find("room")->second.type() == str_type) 
				devInfo.room = objRead.find("room")->second.get_str();
	    		devInfo.currentLoc = loc;
			devInfos.push_back(devInfo);
		}
    	}
    }
 
    return devInfos;   
}
	
vector<measurement> /*json_spirit_dgw::*/JsonToMeasurements(string measStr)
{
    mValue value; 
    read(measStr,value);
    vector <measurement> meass;

    if (value.type() == array_type) 
    { 
	mArray &measJson = value.get_array();
    	for (int i = 0; i < measJson.size(); i++)
    	{
        	if (measJson[i].type() == obj_type) 
		{	
			mObject &objRead = measJson[i].get_obj();
    			measurement meas1;
    			if (objRead.find("sensorMoteID")->second.type() == str_type) 
				meas1.sensorMoteID = objRead.find("sensorMoteID")->second.get_str();
    			if (objRead.find("sensType")->second.type() == int_type) 
				meas1.sensType = (sensorType)objRead.find("sensType")->second.get_int();
    			if ((objRead.find("value")->second.type() == real_type) || (objRead.find("value")->second.type() == int_type))
				meas1.value = objRead.find("value")->second.get_real();
    			if (objRead.find("timestamp")->second.type() == int_type) 
				meas1.timestamp = (long long) objRead.find("timestamp")->second.get_int64();
			meass.push_back(meas1);
		}
	}
    }
    
    return meass;
}

vector <sensor> /*json_spirit_dgw::*/JsonToSensors(string senStr)
{
    mValue value; 
    read(senStr,value);
    vector <sensor> senss;
    
    if (value.type() == array_type)
    {
	mArray &sensJson = value.get_array();
	for (int i = 0; i < sensJson.size(); i++)
    	{
        	if (sensJson[i].type() == obj_type) 
		{
			mObject &objRead = sensJson[i].get_obj();
    			sensor sens1;

    			if (objRead.find("type")->second.type() == int_type) 
				sens1.type = (sensorType)objRead.find("type")->second.get_int();
    			if (objRead.find("parentMoteID")->second.type() == str_type) 
				sens1.parentMoteID = objRead.find("parentMoteID")->second.get_str();
    			if (objRead.find("siUnit")->second.type() == str_type) 
				sens1.siUnit = objRead.find("siUnit")->second.get_str();
    			if ((objRead.find("minValue")->second.type() == real_type) || (objRead.find("minValue")->second.type() == int_type))
				sens1.minValue = objRead.find("minValue")->second.get_real();
    			if ((objRead.find("maxValue")->second.type() == real_type) || (objRead.find("maxValue")->second.type() == int_type)) 
				sens1.maxValue = objRead.find("maxValue")->second.get_real();
    			if ((objRead.find("accuracy")->second.type() == real_type) || (objRead.find("accuracy")->second.type() == int_type))
				sens1.accuracy = (float)objRead.find("accuracy")->second.get_real();
			if ((objRead.find("offset")->second.type() == real_type) || (objRead.find("offset")->second.type() == int_type)) 
				sens1.offset = (float)objRead.find("offset")->second.get_real();
			senss.push_back(sens1);
		}
	}
    }
    
    return senss;
}
	
vector<moteDetails> JsonToVecMoteDetails(string moteDetsStr)
{
    mValue value; 
    read(moteDetsStr,value);
    vector <moteDetails> moteDets;

    if (value.type() == array_type)
    {
	mArray &moteDetsJson = value.get_array();
	for (int i = 0; i < moteDetsJson.size(); i++)
    	{
		if (moteDetsJson[i].type() == obj_type)
        	{
			mObject &objRead = moteDetsJson[i].get_obj();
    			moteDetails mote1;
    			if (objRead.find("sensorMoteID")->second.type() == str_type) 
				mote1.smoteInfo.sensorMoteID = objRead.find("sensorMoteID")->second.get_str();
    			if (objRead.find("serialNumber")->second.type() == int_type) 
				mote1.smoteInfo.serialNumber = objRead.find("serialNumber")->second.get_int();
    			if (objRead.find("deviceIF")->second.type() == str_type) 
				mote1.smoteInfo.deviceIF = objRead.find("deviceIF")->second.get_str();
    			if (objRead.find("connectionStr")->second.type() == str_type) 
				mote1.smoteInfo.connectionStr = objRead.find("connectionStr")->second.get_str();
    			if (objRead.find("macAddress")->second.type() == str_type) 
				mote1.smoteInfo.macAddress = objRead.find("macAddress")->second.get_str();
    			if (objRead.find("room")->second.type() == str_type) 
				mote1.smoteInfo.room = objRead.find("room")->second.get_str();
    			if (objRead.find("numSensors")->second.type() == int_type) 
				mote1.smoteInfo.numSensors = objRead.find("numSensors")->second.get_int();

    			location loc;
    			if (objRead.find("currentLoc")->second.type() == array_type)
			{
				mArray &locArray = objRead.find("currentLoc")->second.get_array();
    				if ((locArray[0].type() == real_type) || (locArray[0].type() == int_type))
					loc.xPoint = locArray[0].get_real();
    				if ((locArray[1].type() == real_type) || (locArray[1].type() == int_type))
					loc.yPoint = locArray[1].get_real();
    				if ((locArray[2].type() == real_type) || (locArray[2].type() == int_type)) 
					loc.zPoint = locArray[2].get_real();
			}
    			mote1.smoteInfo.currentLoc = loc;

			vector <sensor> sensVec;
			if (objRead.find("sensors")->second.type() == array_type)
			{
				mArray &sensArray = objRead.find("sensors")->second.get_array();
			    	for (int i = 0; i < sensArray.size(); i++)
			    	{
					if (sensArray[i].type() == obj_type) 
					{
						mObject &objReadSen = sensArray[i].get_obj();
						sensor sens1;
		
				    		if (objReadSen.find("type")->second.type() == int_type) 
							sens1.type = (sensorType)objReadSen.find("type")->second.get_int();
				    		if (objReadSen.find("parentMoteID")->second.type() == str_type) 
							sens1.parentMoteID = objReadSen.find("parentMoteID")->second.get_str();
				    		if (objReadSen.find("siUnit")->second.type() == str_type) 
							sens1.siUnit = objReadSen.find("siUnit")->second.get_str();
				    		if ((objReadSen.find("minValue")->second.type() == real_type) || (objReadSen.find("minValue")->second.type() == int_type)) 
							sens1.minValue = objReadSen.find("minValue")->second.get_real();
				    		if ((objReadSen.find("maxValue")->second.type() == real_type) || (objReadSen.find("maxValue")->second.type() == int_type)) 
							sens1.maxValue = objReadSen.find("maxValue")->second.get_real();
				    		if ((objReadSen.find("accuracy")->second.type() == real_type) || (objReadSen.find("accuracy")->second.type() == int_type)) 
							sens1.accuracy = (float)objReadSen.find("accuracy")->second.get_real();
						if ((objReadSen.find("offset")->second.type() == real_type) || (objReadSen.find("offset")->second.type() == int_type))
							sens1.offset = (float)objReadSen.find("offset")->second.get_real();
						sensVec.push_back(sens1);
					}
				}
			}
			mote1.sensorsInfo = sensVec;
			moteDets.push_back(mote1);
		}
    	}
    }
    
    return moteDets;
}

moteInfo JsonToMoteInfo(string moteInStr)
{
    mValue value; 
    read(moteInStr,value);    
    moteInfo mote1;

    if (value.type() == obj_type)
    {
	mObject &objRead = value.get_obj();
	if (objRead.find("sensorMoteID")->second.type() == str_type) 
		mote1.sensorMoteID = objRead.find("sensorMoteID")->second.get_str();
    	if (objRead.find("serialNumber")->second.type() == int_type) 
		mote1.serialNumber = objRead.find("serialNumber")->second.get_int();
    	if (objRead.find("deviceIF")->second.type() == str_type) 
		mote1.deviceIF = objRead.find("deviceIF")->second.get_str();
    	if (objRead.find("connectionStr")->second.type() == str_type) 
		mote1.connectionStr = objRead.find("connectionStr")->second.get_str();
    	if (objRead.find("macAddress")->second.type() == str_type) 
		mote1.macAddress = objRead.find("macAddress")->second.get_str();
    	if (objRead.find("room")->second.type() == str_type) 
		mote1.room = objRead.find("room")->second.get_str();
    	if (objRead.find("numSensors")->second.type() == int_type) 
		mote1.numSensors = objRead.find("numSensors")->second.get_int();

    	location loc;
    	if (objRead.find("currentLoc")->second.type() == array_type) 
	{
		mArray &locArray = objRead.find("currentLoc")->second.get_array();
    		if ((loc.xPoint = locArray[0].type() == real_type) || (loc.xPoint = locArray[0].type() == int_type))
			loc.xPoint = locArray[0].get_real();
    		if ((loc.xPoint = locArray[1].type() == real_type) || (loc.xPoint = locArray[1].type() == int_type))
			loc.yPoint = locArray[1].get_real();
    		if ((loc.xPoint = locArray[2].type() == real_type) || (loc.xPoint = locArray[2].type() == int_type))
			loc.zPoint = locArray[2].get_real();
	}
    	mote1.currentLoc = loc;
    }

    return mote1;
}
