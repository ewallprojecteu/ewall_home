/*=================================================================================
Basic explanation:

    Device Gateway cpp code

Authors: Daniel Denkovski (UKIM) (danield@feit.ukim.edu.mk)

Revision History:
    22.09.2014, ver 0.1
====================================================================================*/

#include "libDGwClient.h"
#include <string.h>
#include <stdio.h>

inline IDGwClient::~IDGwClient()
{
}

DGwClient::~DGwClient()
{
}

DGwClient::DGwClient()
{
}

int DGwClient::initializeDGwClient()
{
	sensorMotesJson.clear();
	avDevicesJson.clear();
	sockComm = getDGwSockets();
	sockComm->initializeSocketClient(this);
	cout << "DGwClient: DGwClient initialized!\n";
	return 1;
}
	
int DGwClient::startStream (string avDeviceID, streamConfig *conf)
{
	int msgLength = sizeof(DGwSockMessage);
	int resultsLength = sizeof(streamConfig);
	char *bufsnd = new char[msgLength + resultsLength];
	DGwSockMessage *sndmsg;
	sndmsg = (DGwSockMessage *)bufsnd;
	sndmsg->MsgType = TypeStartStreamReq;
	sndmsg->MsgLen = msgLength;
	avDeviceID.copy(sndmsg->devID, DevIDLength);

	char *dataout;
	int dataoutlen;
	
	memcpy((void*)(bufsnd + msgLength), (void*)conf, resultsLength);
	sockComm->queryDataFromDGw(bufsnd, msgLength + resultsLength, &dataout, dataoutlen);
	delete [] dataout;
	delete [] bufsnd;
	return 0;
}

int DGwClient::reconfigureStream (string avDeviceID, streamConfig *conf)
{
	int msgLength = sizeof(DGwSockMessage);
	int resultsLength = sizeof(streamConfig);
	char *bufsnd = new char[msgLength + resultsLength];
	DGwSockMessage *sndmsg;
	sndmsg = (DGwSockMessage *)bufsnd;
	sndmsg->MsgType = TypeReconfStreamReq;
	sndmsg->MsgLen = msgLength;
	avDeviceID.copy(sndmsg->devID, DevIDLength);

	char *dataout;
	int dataoutlen;

	memcpy((void*)(bufsnd + msgLength), (void*)conf, resultsLength);
	sockComm->queryDataFromDGw(bufsnd, msgLength + resultsLength, &dataout, dataoutlen);
	delete [] dataout;
	delete [] bufsnd;

	return 0;
}

int DGwClient::stopStream (string avDeviceID, short streamtype)
{
	int msgLength = sizeof(DGwSockMessage);
	int resultsLength = sizeof(short);
	char *bufsnd = new char[msgLength + resultsLength];
	DGwSockMessage *sndmsg;
	sndmsg = (DGwSockMessage *)bufsnd;
	sndmsg->MsgType = TypeStopStreamReq;
	sndmsg->MsgLen = msgLength;
	avDeviceID.copy(sndmsg->devID, DevIDLength);

	char *dataout;
	int dataoutlen;

	memcpy((void*)(bufsnd + msgLength), (void*)&streamtype, resultsLength);
	sockComm->queryDataFromDGw(bufsnd, msgLength, &dataout, dataoutlen);
	delete [] dataout;
	delete [] bufsnd;

	return 0;
}

int DGwClient::rotateAVDevice (string avDeviceID, double angle)
{
	int msgLength = sizeof(DGwSockMessage);
	int resultsLength = sizeof(double);
	char *bufsnd = new char[msgLength + resultsLength];
	DGwSockMessage *sndmsg;
	sndmsg = (DGwSockMessage *)bufsnd;
	sndmsg->MsgType = TypeRotateAVReq;
	sndmsg->MsgLen = msgLength;
	avDeviceID.copy(sndmsg->devID, DevIDLength);

	char *dataout;
	int dataoutlen;

	memcpy((void*)(bufsnd + msgLength), (void*)&angle, resultsLength);
	sockComm->queryDataFromDGw(bufsnd, msgLength + resultsLength, &dataout, dataoutlen);
	delete [] dataout;
	delete [] bufsnd;

	return 0;
}

/*int DGwClient::pushFrame(string avDeviceID, void *frameData, string frameConfig)
{
	cout << "DGwClient: DGwClient pushing frame from AVDevice with ID = " << avDeviceID << "to AV processing !\n";
	return 1;
}*/

string DGwClient::getConfiguration (string avDeviceID)
{
	int msgLength = sizeof(DGwSockMessage);
	char *bufsnd = new char[msgLength];
	DGwSockMessage *sndmsg;
	sndmsg = (DGwSockMessage *)bufsnd;
	sndmsg->MsgType = TypeAVConfReq;
	sndmsg->MsgLen = msgLength;
	avDeviceID.copy(sndmsg->devID, DevIDLength);

	int header = sizeof(DGwSockMessage);
	char *dataout;
	int dataoutlen;

	sockComm->queryDataFromDGw(bufsnd, msgLength, &dataout, dataoutlen);
	streamConfig conf;
	if (dataoutlen > header)
	{
		string jsonDataStr(dataout+header, dataoutlen-header);
		cout << "DGwClient: JSON data received:\n" << jsonDataStr;
		delete [] dataout;
		delete [] bufsnd;
		return jsonDataStr;
	}
	delete [] dataout;
	delete [] bufsnd;

	return "";
}

string DGwClient::getAVDeviceInfo (string avDeviceID)
{
	int msgLength = sizeof(DGwSockMessage);
	char *bufsnd = new char[msgLength];
	DGwSockMessage *sndmsg;
	sndmsg = (DGwSockMessage *)bufsnd;
	sndmsg->MsgType = TypeAVInfoReq;
	sndmsg->MsgLen = msgLength;
	avDeviceID.copy(sndmsg->devID, DevIDLength);

	int header = sizeof(DGwSockMessage);
	char *dataout;
	int dataoutlen;

	sockComm->queryDataFromDGw(bufsnd, msgLength, &dataout, dataoutlen);
	
	avDeviceInfo result;
	//char *jsonData = new char[dataoutlen-header];
	if (dataoutlen > header)
	{
		string jsonDataStr(dataout+header, dataoutlen-header);
		cout << "DGwClient: JSON data received:\n" << jsonDataStr;
		delete [] dataout;
		delete [] bufsnd;
		return jsonDataStr;
	}
	delete [] dataout;
	delete [] bufsnd;

	return "";
}

string DGwClient::getAllAVDevicesInfo()
{
	int msgLength = sizeof(DGwSockMessage);
	char *bufsnd = new char[msgLength];
	DGwSockMessage *sndmsg;
	sndmsg = (DGwSockMessage *)bufsnd;
	sndmsg->MsgType = TypeAllAVInfoReq;
	sndmsg->MsgLen = msgLength;

	int header = sizeof(DGwSockMessage);
	char *dataout;
	int dataoutlen;

	sockComm->queryDataFromDGw(bufsnd, msgLength, &dataout, dataoutlen);

	vector<avDeviceInfo> result;
	if (dataoutlen > header)
	{
		string jsonDataStr(dataout+header, dataoutlen-header);
		cout << "DGwClient: JSON data received:\n" << jsonDataStr;
		delete [] dataout;
		delete [] bufsnd;
		return jsonDataStr;
	}
	delete [] dataout;
	delete [] bufsnd;

	return "";
}
		
int DGwClient::startMeasuring (string sensorMoteID, float rate)
{
	int msgLength = sizeof(DGwSockMessage);
	int resultsLength = sizeof(float);
	char *bufsnd = new char[msgLength + resultsLength];
	DGwSockMessage *sndmsg;
	sndmsg = (DGwSockMessage *)bufsnd;
	sndmsg->MsgType = TypeStartMeasReq;
	sndmsg->MsgLen = msgLength;
	sensorMoteID.copy(sndmsg->devID, DevIDLength);

	char *dataout;
	int dataoutlen;

	memcpy((void*)(bufsnd + msgLength), (void*)&rate, resultsLength);
	sockComm->queryDataFromDGw(bufsnd, msgLength + resultsLength, &dataout, dataoutlen);
	delete [] dataout;
	delete [] bufsnd;

	return 0;
}

int DGwClient::reconfigure (string sensorMoteID, float rate)
{
	int msgLength = sizeof(DGwSockMessage);
	int resultsLength = sizeof(float);
	char *bufsnd = new char[msgLength + resultsLength];
	DGwSockMessage *sndmsg;
	sndmsg = (DGwSockMessage *)bufsnd;
	sndmsg->MsgType = TypeReconfMeasReq;
	sndmsg->MsgLen = msgLength;
	sensorMoteID.copy(sndmsg->devID, DevIDLength);

	char *dataout;
	int dataoutlen;

	memcpy((void*)(bufsnd + msgLength), (void*)&rate, resultsLength);			
	sockComm->queryDataFromDGw(bufsnd, msgLength + resultsLength, &dataout, dataoutlen);
	delete [] dataout;
	delete [] bufsnd;

	return 0;
}

int DGwClient::stopMeasuring (string sensorMoteID)
{
	int msgLength = sizeof(DGwSockMessage);
	char *bufsnd = new char[msgLength];
	DGwSockMessage *sndmsg;
	sndmsg = (DGwSockMessage *)bufsnd;
	sndmsg->MsgType = TypeStopMeasReq;
	sndmsg->MsgLen = msgLength;
	sensorMoteID.copy(sndmsg->devID, DevIDLength);

	char *dataout;
	int dataoutlen;

	sockComm->queryDataFromDGw(bufsnd, msgLength, &dataout, dataoutlen);
	delete [] dataout;
	delete [] bufsnd;

	return 0;
}

/*int DGwClient::pushMeasurements (string sensorMoteID, string measurements)
{
	cout << "DGwClient: SensorMote with ID = " << sensorMoteID << " pushes measurements to nonAV processing!\n" << measurements << "\n";
	return 1;
}*/

string DGwClient::getMeasurements (string sensorMoteID)
{
	int msgLength = sizeof(DGwSockMessage);
	char *bufsnd = new char[msgLength];
	DGwSockMessage *sndmsg;
	sndmsg = (DGwSockMessage *)bufsnd;
	sndmsg->MsgType = TypeMeasReq;
	sndmsg->MsgLen = msgLength;
	sensorMoteID.copy(sndmsg->devID, DevIDLength);

	int header = sizeof(DGwSockMessage);
	char *dataout;
	int dataoutlen;

	sockComm->queryDataFromDGw(bufsnd, msgLength, &dataout, dataoutlen);
	vector<measurement> result;
	if (dataoutlen > header)
	{
		string jsonDataStr(dataout+header, dataoutlen-header);
		cout << "DGwClient: JSON data received:\n" << jsonDataStr;
		delete [] dataout;
		delete [] bufsnd;
		return jsonDataStr;
	}
	delete [] dataout;
	delete [] bufsnd;

	return "";
}

string DGwClient::getSensors (string sensorMoteID)
{
	int msgLength = sizeof(DGwSockMessage);
	char *bufsnd = new char[msgLength];
	DGwSockMessage *sndmsg;
	sndmsg = (DGwSockMessage *)bufsnd;
	sndmsg->MsgType = TypeSensorsInfoReq;
	sndmsg->MsgLen = msgLength;
	sensorMoteID.copy(sndmsg->devID, DevIDLength);

	int header = sizeof(DGwSockMessage);
	char *dataout;
	int dataoutlen;

	sockComm->queryDataFromDGw(bufsnd, msgLength, &dataout, dataoutlen);
	vector<sensor> result;
	if (dataoutlen > header)
	{
		string jsonDataStr(dataout+header, dataoutlen-header);
		cout << "DGwClient: JSON data received:\n" << jsonDataStr;
		delete [] dataout;
		delete [] bufsnd;
		return jsonDataStr;
	}
	delete [] dataout;
	delete [] bufsnd;

	return "";
}

string DGwClient::getSensorMoteInfo (string sensorMoteID)
{
	int msgLength = sizeof(DGwSockMessage);
	char *bufsnd = new char[msgLength];
	DGwSockMessage *sndmsg;
	sndmsg = (DGwSockMessage *)bufsnd;
	sndmsg->MsgType = TypeMoteInfoReq;
	sndmsg->MsgLen = msgLength;
	sensorMoteID.copy(sndmsg->devID, DevIDLength);

	int header = sizeof(DGwSockMessage);
	char *dataout;
	int dataoutlen;

	sockComm->queryDataFromDGw(bufsnd, msgLength, &dataout, dataoutlen);
	moteInfo result;
	if (dataoutlen > header)
	{
		string jsonDataStr(dataout+header, dataoutlen-header);
		cout << "DGwClient: JSON data received:\n" << jsonDataStr;
		delete [] dataout;
		delete [] bufsnd;
		return jsonDataStr;
	}
	delete [] dataout;
	delete [] bufsnd;

	return "";
}

string DGwClient::getAllSensorDevicesInfo()
{
	int msgLength = sizeof(DGwSockMessage);
	char *bufsnd = new char[msgLength];
	DGwSockMessage *sndmsg;
	sndmsg = (DGwSockMessage *)bufsnd;
	sndmsg->MsgType = TypeAllSensInfoReq;
	sndmsg->MsgLen = msgLength;

	int header = sizeof(DGwSockMessage);
	char *dataout;
	int dataoutlen;

	sockComm->queryDataFromDGw(bufsnd, msgLength, &dataout, dataoutlen);

	vector<moteDetails> result;
	if (dataoutlen > header)
	{
		string jsonDataStr(dataout+header, dataoutlen-header);
		cout << "DGwClient: JSON data received:\n" << jsonDataStr;
		delete [] dataout;
		delete [] bufsnd;
		return jsonDataStr;
	}
	delete [] dataout;
	delete [] bufsnd;

	return "";
}

float DGwClient::getRate(string sensorMoteID)
{
	int msgLength = sizeof(DGwSockMessage);
	char *bufsnd = new char[msgLength];
	DGwSockMessage *sndmsg;
	sndmsg = (DGwSockMessage *)bufsnd;
	sndmsg->MsgType = TypeMeasConfReq;
	sndmsg->MsgLen = msgLength;
	sensorMoteID.copy(sndmsg->devID, DevIDLength);

	int header = sizeof(DGwSockMessage);
	char *dataout;
	int dataoutlen;

	sockComm->queryDataFromDGw(bufsnd, msgLength, &dataout, dataoutlen);
	float result = 0;
	if (dataoutlen > header)
	{
		result = *(float*)(dataout + header);
		cout << "DGwClient: Mote reporting period = " << result;
	}
	delete [] dataout;
	delete [] bufsnd;

	return result;
}