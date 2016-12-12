/*=================================================================================
Basic explanation:

Authors: Daniel Denkovski (UKIM) (danield@feit.ukim.edu.mk)

Revision History:
    25.08.2014, ver 0.1
====================================================================================*/

#pragma once

#include <stdint.h>
#include <string>
#include <vector>
#include <iostream>

using namespace std;

typedef struct frameResolution{
	int width; //in pixels
	int height;	//in pixels
	frameResolution(int w = 0, int h = 0) : width(w), height(h) {}
} frameResolution;

typedef struct streamConfig{
	frameResolution resolution; 	// image (width)X(height), audio (noSamples)X1
	short nochs;			// number of channels (3 for RGB), number of mics/channels (for audio)
	int rate; 			// in Hz
	short format; 			// 1 for rgb, 2 for audio wav
	short stream; 			// video 1, depth 2, audio 3
	short bytesps;			// number of bytes per sample (1 for short, 4 for int32_t)
	streamConfig(frameResolution res = frameResolution(0,0), short inochs = 0, int irate = 0, short form = 0, short str = 0, short bps = 0) : resolution(res), nochs(inochs), rate(irate), format(form), stream(str), bytesps(bps) {} 
} streamConfig;

typedef struct location{
	double xPoint;
	double yPoint;
	double zPoint;
	location(double x = 0, double y = 0, double z = 0) : xPoint(x), yPoint(y), zPoint(z) {}
} location;

typedef struct avDeviceInfo{
	string deviceID; 	//unique id of the device
	int serialNumber; 	//serial number of the device
	string deviceIF; 	//usb, ethernet...
	string connectionStr; //comPort, ip/mac address
	location currentLoc;
	string room;
	avDeviceInfo(string devid = "", int sNum = 0, string devIF = "", string conn = "", location l = location(0,0,0), string roomname = "") : deviceID(devid), serialNumber(sNum), deviceIF(devIF), connectionStr(conn), currentLoc(l), room(roomname) {}
} avDeviceInfo;

typedef enum {
	_NONE = 0,
	_TEMPERATURE = 1,
	_HUMIDITY = 2,
	_LUMINOCITY = 3,
	_ACTIVITY = 4,
	_ACCELEROMETER_X = 5,
	_ACCELEROMETER_Y = 6,
	_ACCELEROMETER_Z = 7,
	_WATERFLOW = 8,
	_POWERCONSUMPTION = 9,
	_GASES = 10,
	_MASS = 11,
	_PRESSURE = 12,	
	_IMA = 13,
	_BPM = 14,
	_SPO = 15,
	_LPG = 16,
	_NG = 17,
	_CO = 18,
	_DOOR = 19,
	_MDC_PULS_OXIM_SAT_O2 = 19384,
	_MDC_PULS_OXIM_PULS_RATE = 18458,
	_MDC_PULS_RATE_NON_INV = 18474,
	_MDC_PRESS_BLD_NONINV_SYS = 18949,
	_MDC_PRESS_BLD_NONINV_DIA = 18950,
	_MDC_PRESS_BLD_NONINV_MEAN = 18951,
	_MDC_CONC_GLU_CAPILLARY_WHOLEBLOOD = 29112,	
	_MDC_LEN_BODY_ACTUAL = 57668,
	_MDC_MASS_BODY_ACTUAL = 57664,
	_MDC_RATIO_MASS_BODY_LEN_SQ = 57680
} sensorType;

typedef struct sensor{
	sensorType type;
	string parentMoteID;
	string siUnit;
	double minValue;
	double maxValue;
	float accuracy;
	float offset;
	sensor(sensorType ty = _NONE, string pmote = "", string si = "", double min = 0, double max = 0, float acc = 0, float offs = 0) : type(ty), parentMoteID(pmote), siUnit(si), minValue(min), maxValue(max), accuracy(acc), offset(offs) {}
} sensor;

typedef struct measurement{
	string sensorMoteID;
	sensorType sensType;
	double value;
	long long timestamp;
	measurement(string moteID = "", sensorType type = _NONE, double val = 0, long long stime = 0): sensorMoteID(moteID), sensType(type), value(val), timestamp(stime) {}
} measurement;

typedef struct moteInfo{
	string sensorMoteID; //unique id of the device
	int serialNumber; //serial number of the device
	string deviceIF; //usb, ethernet...
	string connectionStr; //comPort 
	string macAddress; // mac address of the mote
	location currentLoc;
	string room;
	int numSensors;
	moteInfo(string moteid = "", int sNum = 0, string devIF = "", string conn = "", string mac = "", location l = location(0,0,0), string roomname = "", int numSen = 0) : sensorMoteID(moteid), serialNumber(sNum), deviceIF(devIF), connectionStr(conn), macAddress(mac), currentLoc(l), room(roomname), numSensors(numSen) {}
} moteInfo;

typedef struct moteDetails{
	moteInfo smoteInfo;
	vector<sensor> sensorsInfo;
} moteDetails;


//message types DeviceGateway <-> DGwClient
#define TypeUnknownReq 		100
#define TypeUnknownRsp 		200

#define TypeStartStreamReq 	101
#define TypeStartStreamRsp 	201

#define TypeReconfStreamReq	102
#define TypeReconfStreamRsp	202

#define TypeStopStreamReq	103
#define TypeStopStreamRsp	203

#define TypeRotateAVReq		104
#define TypeRotateAVRsp		204

#define TypeLastFrameReq	105
#define TypeLastFrameRsp	205

#define TypeAVConfReq		106
#define TypeAVConfRsp		206

#define TypeAVInfoReq		107
#define TypeAVInfoRsp		207

#define TypeAllAVInfoReq	108
#define TypeAllAVInfoRsp	208

#define TypeStartMeasReq	109
#define TypeStartMeasRsp	209

#define TypeReconfMeasReq	110
#define TypeReconfMeasRsp	210

#define TypeStopMeasReq		111
#define TypeStopMeasRsp		211

#define TypeMeasReq		112
#define TypeMeasRsp		212

#define TypeSensorsInfoReq	113
#define TypeSensorsInfoRsp	213

#define TypeAllSensInfoReq	114
#define TypeAllSensInfoRsp	214

#define TypeMoteInfoReq		115
#define TypeMoteInfoRsp		215

#define TypeMeasConfReq		116
#define TypeMeasConfRsp		216

#define TypePushFrameRsp	227
#define TypePushMeasRsp		228

//socket interface definitions

#define DevIDLength		20
#define DGwSocket		12345 	// DeviceGateway listen socket
#define DGwClientSocket		54321 	// DGwClient listen socket

#pragma pack (1)

typedef struct DGwSockMessage
{
	unsigned short MsgType;
	unsigned int MsgLen;
	char devID[DevIDLength];
} DGwSockMessage;

#pragma pack()

