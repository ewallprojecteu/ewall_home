/*=================================================================================
Basic explanation:

Authors: Daniel Denkovski (UKIM) (danield@feit.ukim.edu.mk)

Revision History:
    22.09.2014, ver 0.1
====================================================================================*/

#ifndef LIBDGwSockets_H
#define LIBDGwSockets_H

#include "IDGwSockets.h"

class DGwSockets: public IDGwSockets
{
public:
	int initializeDGwSockets();

	DGwSockets();
	
	~DGwSockets();

	int initializeSocketServer(IDeviceGateway *dgw);

	int initializeSocketClient(IDGwClient *dgwc);

	int sendDataToDGwClients(char *data, int length);

	int queryDataFromDGw(char *data, int length, char **dataOut, int &dataOutLen);

	int closeSocketClient();

	int closeSocketServer();

	IDeviceGateway *dgwserv;

	IDGwClient *dgwclient;

	bool runComm;

	int server_socket;

	vector <int> client_sockets;

	bool dataAvailable;

	int client_socket;

	char bufferedData[MAX_DATA];
};
#endif
