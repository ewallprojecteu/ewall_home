/*=================================================================================
Basic explanation:

Authors: Daniel Denkovski (UKIM) (danield@feit.ukim.edu.mk)

Revision History:
    22.09.2014, ver 0.1
====================================================================================*/

#pragma once

#ifndef IDGWSOCKETS_H
#define IDGWSOCKETS_H

#include "DGWdefs.h"
#include "IDeviceGateway.h"
#include "IDGwClient.h"

#define MAX_CLIENTS 10
//This number is too small for high resolutions #define MAX_DATA 4194304//524288//4194304
#define MAX_DATA (1920*1080*4) // This supports RGBA image with size 1920x1080 pixels

class IDeviceGateway;
class IDGwClient;

class IDGwSockets
{
public:
	virtual int initializeDGwSockets() = 0;

	virtual int initializeSocketServer(IDeviceGateway *dgw) = 0;

	virtual int initializeSocketClient(IDGwClient *dgwc) = 0;

	virtual int sendDataToDGwClients(char *data, int length) = 0;

	virtual int queryDataFromDGw(char *data, int length, char **dataOut, int &dataOutLen) = 0;

	virtual int closeSocketClient() = 0;

	virtual int closeSocketServer() = 0;
	
	virtual ~IDGwSockets() = 0;
};

#ifdef __cplusplus
extern "C" {
#endif
//get handler to DeviceGateway
IDGwSockets *getDGwSockets();
//release DeviceGateway
void releaseDGwSocket(const IDGwSockets *pDGwSocket);
#ifdef __cplusplus
}
#endif

#endif
