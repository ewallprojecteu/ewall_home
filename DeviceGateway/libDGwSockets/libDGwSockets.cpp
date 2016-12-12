/*=================================================================================
Basic explanation:

    Device Gateway cpp code

Authors: Daniel Denkovski (UKIM) (danield@feit.ukim.edu.mk)

Revision History:
    22.09.2014, ver 0.1
====================================================================================*/

#include "libDGwSockets.h"
#include "json_spirit_dgw.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h> 

static IDGwSockets *g_dgwsock = NULL;
static int g_dgwsockCount = 0;
pthread_t hthreadSocket;
char *sendMsgBuff;

struct send_struct {
	int socket;
	int length;
	char *data;
};

void *threadRunSend(void *object)
{
	struct send_struct *args = (struct send_struct *) object;
	if (args->socket > 0)
	    send(args->socket, args->data, args->length, 0);
	pthread_exit(NULL);
}

void *threadRunServ(void *object)
{
	DGwSockets* mydgwserv = (DGwSockets*)object;
    	fd_set readfds;
	int max_sd, sd, activity, addrlen, valread;
    	struct sockaddr_in address; 
    	address.sin_family = AF_INET;
    	address.sin_addr.s_addr = INADDR_ANY;
    	address.sin_port = htons(DGwSocket);
	int new_socket;
	char buffer[MAX_DATA];
    	addrlen = sizeof(address);
    	int opt = true;
    
   	if ((mydgwserv->server_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
    	{
        	perror("socket failed");
        	exit(EXIT_FAILURE);
    	}
 
    	if (setsockopt(mydgwserv->server_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0)
    	{
        	perror("setsockopt");
        	exit(EXIT_FAILURE);
    	}
     
    	if (bind(mydgwserv->server_socket, (struct sockaddr *)&address, sizeof(address)) < 0) 
    	{
        	perror("bind failed");
        	exit(EXIT_FAILURE);
    	}
	printf("Listener on port %d \n", DGwSocket);
	
    	if (listen(mydgwserv->server_socket, 3) < 0)
    	{
        	perror("listen");
        	exit(EXIT_FAILURE);
    	}
	cout << "DGwSockets: Thread run socket server started!!\n";
	
	while (mydgwserv->runComm)
	{
		FD_ZERO(&readfds);
		FD_SET(mydgwserv->server_socket, &readfds);
		max_sd = mydgwserv->server_socket;
		int header = sizeof(DGwSockMessage);

		vector<int>::iterator it;
		for (it = mydgwserv->client_sockets.begin(); it != mydgwserv->client_sockets.end(); it++)
		{
		    sd = (*it);
		    if(sd > 0)
			FD_SET( sd , &readfds);
            	    if(sd > max_sd)
			max_sd = sd;
		}
	 
		activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
		if ((activity < 0) && (errno!=EINTR)) 
		{
		    printf("select error");
		}
		 
		if (FD_ISSET(mydgwserv->server_socket, &readfds)) 
		{
		    if ((new_socket = accept(mydgwserv->server_socket, (struct sockaddr *)&address, (socklen_t*)&addrlen)) < 0)
		    {
		        perror("accept");
		        exit(EXIT_FAILURE);
		    }
	       
		    if (mydgwserv->client_sockets.size() < MAX_CLIENTS - 1)
			mydgwserv->client_sockets.push_back(new_socket);

		    if (new_socket > max_sd)
			max_sd = new_socket;
		}

		for (it = mydgwserv->client_sockets.begin(); it != mydgwserv->client_sockets.end(); it++)
		{
		    sd = (*it);
		    if (FD_ISSET(sd, &readfds)) 
		    {
		        if ((valread = read(sd, buffer, header)) == 0)
		        {
		            getpeername(sd , (struct sockaddr*)&address , (socklen_t*)&addrlen);
		            close(sd);
			    mydgwserv->client_sockets.erase(it);
		            it--;
		        }
		        else
		        {
			    DGwSockMessage msg = *(DGwSockMessage*)&buffer;
			    	if (msg.MsgType == TypeStartStreamReq)
				    {
					cout << "DGwSockets: TypeStartStreamReq received!\n";
					streamConfig conf = *(streamConfig*)(&buffer + sizeof(DGwSockMessage));
					string avDeviceID(msg.devID);
					mydgwserv->dgwserv->startStream(avDeviceID, &conf);

					int msgLength = sizeof(DGwSockMessage);
					char *bufsnd = new char[msgLength];
 					DGwSockMessage *sndmsg;
					sndmsg = (DGwSockMessage *)bufsnd;
					sndmsg->MsgType = TypeStartStreamRsp;
					sndmsg->MsgLen = msgLength;

					send(sd, bufsnd, msgLength, 0);
					delete [] bufsnd;
				    }
			    	else if (msg.MsgType == TypeReconfStreamReq)
				    {
					cout << "DGwSockets: TypeReconfStreamReq received!\n";
					streamConfig conf = *(streamConfig*)(&buffer + sizeof(DGwSockMessage));
					string avDeviceID(msg.devID);
					mydgwserv->dgwserv->reconfigureStream(avDeviceID, &conf);

					int msgLength = sizeof(DGwSockMessage);
					char *bufsnd = new char[msgLength];
 					DGwSockMessage *sndmsg;
					sndmsg = (DGwSockMessage *)bufsnd;
					sndmsg->MsgType = TypeReconfStreamRsp;
					sndmsg->MsgLen = msgLength;

					send(sd, bufsnd, msgLength, 0);
					delete [] bufsnd;
				    }
			    	else if (msg.MsgType == TypeStopStreamReq)
				    {
					cout << "DGwSockets: TypeStopStreamReq received!\n";
					string avDeviceID(msg.devID);
					short streamtype = *(short*)(buffer + sizeof(DGwSockMessage));
					mydgwserv->dgwserv->stopStream(avDeviceID, streamtype);

					int msgLength = sizeof(DGwSockMessage);
					char *bufsnd = new char[msgLength];
 					DGwSockMessage *sndmsg;
					sndmsg = (DGwSockMessage *)bufsnd;
					sndmsg->MsgType = TypeStopStreamRsp;
					sndmsg->MsgLen = msgLength;

					send(sd, bufsnd, msgLength, 0);
					delete [] bufsnd;
				    }
				else if (msg.MsgType == TypeRotateAVReq)
				    {
					cout << "DGwSockets: TypeRotateAVReq received!\n";
					string avDeviceID(msg.devID);
					double angle = *(double*)(buffer + sizeof(DGwSockMessage));
					mydgwserv->dgwserv->rotateAVDevice(avDeviceID, angle);

					int msgLength = sizeof(DGwSockMessage);
					char *bufsnd = new char[msgLength];
 					DGwSockMessage *sndmsg;
					sndmsg = (DGwSockMessage *)bufsnd;
					sndmsg->MsgType = TypeRotateAVRsp;
					sndmsg->MsgLen = msgLength;

					send(sd, bufsnd, msgLength, 0);
					delete [] bufsnd;
				    }
				else if (msg.MsgType == TypeAVConfReq)
				    {
					cout << "DGwSockets: TypeAVConfReq received!\n";
					string avDeviceID(msg.devID);
					streamConfig conf = mydgwserv->dgwserv->getConfiguration(avDeviceID);
					string results;
					if ((conf.rate != 0) && (conf.format != 0) && (conf.stream != 0)) 
						results = streamConfigToJSON(conf);
					else 
						results ="[\n]";

					int msgLength = sizeof(DGwSockMessage);
					int resultsLength = results.size();
					char *bufsnd = new char[msgLength + resultsLength];
 					DGwSockMessage *sndmsg;
					sndmsg = (DGwSockMessage *)bufsnd;
					sndmsg->MsgType = TypeAVConfRsp;
					sndmsg->MsgLen = msgLength + resultsLength;

					results.copy(bufsnd + msgLength, resultsLength);
					send(sd, bufsnd, msgLength + resultsLength, 0);
					delete [] bufsnd;
				    }
				else if (msg.MsgType == TypeAVInfoReq)
				    {
					cout << "DGwSockets: TypeAVInfoReq received!\n";
					string avDeviceID(msg.devID);
					avDeviceInfo avinfo = mydgwserv->dgwserv->getAVDeviceInfo(avDeviceID);
					string results;
					if (avinfo.deviceID != "")
						results = avDeviceInfoToJSON(avinfo);
					else 
						results ="[\n]";					

					int msgLength = sizeof(DGwSockMessage);
					int resultsLength = results.size();
					char *bufsnd = new char[msgLength + resultsLength];
 					DGwSockMessage *sndmsg;
					sndmsg = (DGwSockMessage *)bufsnd;
					sndmsg->MsgType = TypeAVInfoRsp;
					sndmsg->MsgLen = msgLength + resultsLength;

					results.copy(bufsnd + msgLength, resultsLength);
					send(sd, bufsnd, msgLength + resultsLength, 0);
					delete [] bufsnd;
				    }
				else if (msg.MsgType == TypeAllAVInfoReq)
				    {
					cout << "DGwSockets: TypeAllAVInfoReq received!\n";
					string avDeviceID(msg.devID);
					vector<avDeviceInfo> avinfos = mydgwserv->dgwserv->getAllAVDevicesInfo();
					string results = vecAVDeviceInfoToJSON(avinfos);

					int msgLength = sizeof(DGwSockMessage);
					int resultsLength = results.size();
					char *bufsnd = new char[msgLength + resultsLength];
 					DGwSockMessage *sndmsg;
					sndmsg = (DGwSockMessage *)bufsnd;
					sndmsg->MsgType = TypeAllAVInfoRsp;
					sndmsg->MsgLen = msgLength + resultsLength;

					results.copy(bufsnd + msgLength, resultsLength);
					send(sd, bufsnd, msgLength + resultsLength, 0);
					delete [] bufsnd;
				    }
				else if (msg.MsgType == TypeStartMeasReq)
				    {
					cout << "DGwSockets: TypeStartMeasReq received!\n";
					string sensDeviceID(msg.devID);
					float period = *(float*)(buffer + sizeof(DGwSockMessage));
					mydgwserv->dgwserv->startMeasuring(sensDeviceID,period);

					int msgLength = sizeof(DGwSockMessage);
					char *bufsnd = new char[msgLength];
 					DGwSockMessage *sndmsg;
					sndmsg = (DGwSockMessage *)bufsnd;
					sndmsg->MsgType = TypeStartMeasRsp;
					sndmsg->MsgLen = msgLength;

					send(sd, bufsnd, msgLength, 0);
					delete [] bufsnd;
				    }
				else if (msg.MsgType == TypeReconfMeasReq)
				    {
					cout << "DGwSockets: TypeReconfMeasReq received!\n";
					string sensDeviceID(msg.devID);
					float period = *(float*)(buffer + sizeof(DGwSockMessage));				
					mydgwserv->dgwserv->reconfigure(sensDeviceID,period);

					int msgLength = sizeof(DGwSockMessage);
					char *bufsnd = new char[msgLength];
 					DGwSockMessage *sndmsg;
					sndmsg = (DGwSockMessage *)bufsnd;
					sndmsg->MsgType = TypeReconfMeasRsp;
					sndmsg->MsgLen = msgLength;

					send(sd, bufsnd, msgLength, 0);
					delete [] bufsnd;
				    }
				else if (msg.MsgType == TypeStopMeasReq)
				    {
					cout << "DGwSockets: TypeStopMeasReq received!\n";
					string sensDeviceID(msg.devID);
					mydgwserv->dgwserv->stopMeasuring(sensDeviceID);

					int msgLength = sizeof(DGwSockMessage);
					char *bufsnd = new char[msgLength];
 					DGwSockMessage *sndmsg;
					sndmsg = (DGwSockMessage *)bufsnd;
					sndmsg->MsgType = TypeStopMeasRsp;
					sndmsg->MsgLen = msgLength;

					send(sd, bufsnd, msgLength, 0);
					delete [] bufsnd;
				    }
				else if (msg.MsgType == TypeMeasReq)
				    {
					cout << "DGwSockets: TypeMeasReq received!\n";
					string sensDeviceID(msg.devID);
					int res = mydgwserv->dgwserv->getMeasurements(sensDeviceID);
				    }
				else if (msg.MsgType == TypeSensorsInfoReq)
				    {
					cout << "DGwSockets: TypeSensorsInfoReq received!\n";
					string sensDeviceID(msg.devID);
					vector<sensor> senss = mydgwserv->dgwserv->getSensors(sensDeviceID);
					string results = sensorsToJSON(senss);

					int msgLength = sizeof(DGwSockMessage);
					int resultsLength = results.size();
					char *bufsnd = new char[msgLength + resultsLength];
 					DGwSockMessage *sndmsg;
					sndmsg = (DGwSockMessage *)bufsnd;
					sndmsg->MsgType = TypeSensorsInfoRsp;
					sndmsg->MsgLen = msgLength + resultsLength;

					results.copy(bufsnd + msgLength, resultsLength);
					send(sd, bufsnd, msgLength + resultsLength, 0);
					delete [] bufsnd;
				    }
				else if (msg.MsgType == TypeAllSensInfoReq)
				    {
					cout << "DGwSockets: TypeAllSensInfoReq received!\n";
					string sensDeviceID(msg.devID);
					vector<moteDetails> motedets = mydgwserv->dgwserv->getAllSensorDevicesInfo();
					string results = vecMoteDetailsToJSON(motedets);

					int msgLength = sizeof(DGwSockMessage);
					int resultsLength = results.size();
					char *bufsnd = new char[msgLength + resultsLength];
 					DGwSockMessage *sndmsg;
					sndmsg = (DGwSockMessage *)bufsnd;
					sndmsg->MsgType = TypeAllSensInfoRsp;
					sndmsg->MsgLen = msgLength + resultsLength;

					results.copy(bufsnd + msgLength, resultsLength);
					send(sd, bufsnd, msgLength + resultsLength, 0);
					delete [] bufsnd;
				    }
				else if (msg.MsgType == TypeMoteInfoReq)
				    {
					cout << "DGwSockets: TypeMoteInfoReq received!\n";
					string sensDeviceID(msg.devID);
					moteInfo moteinf = mydgwserv->dgwserv->getSensorMoteInfo(sensDeviceID);
					string results;
					if (moteinf.sensorMoteID != "")
						results = moteInfoToJSON(moteinf);
					else 
						results ="[\n]";

					int msgLength = sizeof(DGwSockMessage);
					int resultsLength = results.size();
					char *bufsnd = new char[msgLength + resultsLength];
 					DGwSockMessage *sndmsg;
					sndmsg = (DGwSockMessage *)bufsnd;
					sndmsg->MsgType = TypeMoteInfoRsp;
					sndmsg->MsgLen = msgLength + resultsLength;

					results.copy(bufsnd + msgLength, resultsLength);
					send(sd, bufsnd, msgLength + resultsLength, 0);
					delete [] bufsnd;
				    }
				else if (msg.MsgType == TypeMeasConfReq)
				    {
					cout << "DGwSockets: TypeMeasConfReq received!\n";
					string sensDeviceID(msg.devID);
					float results = mydgwserv->dgwserv->getRate(sensDeviceID);

					int msgLength = sizeof(DGwSockMessage);
					int resultsLength = sizeof(moteInfo);
					char *bufsnd = new char[msgLength + resultsLength];
					DGwSockMessage *sndmsg;
					sndmsg = (DGwSockMessage *)bufsnd;
					sndmsg->MsgType = TypeMeasConfRsp;
					sndmsg->MsgLen = msgLength + resultsLength;

					memcpy((void*)(bufsnd + msgLength), (void*)&results, resultsLength);
					send(sd, bufsnd, msgLength + resultsLength, 0);	
					delete [] bufsnd;
			    }
			    	    }
				memset((void*)&buffer, 0, MAX_DATA);
		    	}
		}
	 }
}

void *threadRunClient(void *object)
{
	DGwSockets* mydgwclient = (DGwSockets*)object;
    	fd_set readfds;
	int sd, addrlen, valread;
    	struct sockaddr_in address; 
    	address.sin_family = AF_INET;
    	address.sin_addr.s_addr = inet_addr("127.0.0.1");
    	address.sin_port = htons(DGwSocket);
    	addrlen = sizeof(address);
    	int opt = true;
	int header = sizeof(DGwSockMessage);
    
   	if ((mydgwclient->client_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0) 
    	{
        	perror("socket failed");
        	exit(EXIT_FAILURE);
    	}
 
    	if (setsockopt(mydgwclient->client_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0)
    	{
        	perror("setsockopt");
        	exit(EXIT_FAILURE);
    	}
	
    	if (connect(mydgwclient->client_socket, (struct sockaddr *)&address, sizeof(address)) < 0)
    	{
        	perror("connect");
        	exit(EXIT_FAILURE);
    	}
	cout << "DGwSockets: Thread run socket client started!\n";
	
	while (mydgwclient->runComm)
	{
		FD_ZERO(&readfds);
		FD_SET(mydgwclient->client_socket, &readfds);
		sd = mydgwclient->client_socket;
		if (FD_ISSET(sd, &readfds)) 
		{
		    if ((valread = read(sd, mydgwclient->bufferedData, header)) == 0)
		    {
		            getpeername(sd , (struct sockaddr*)&address , (socklen_t*)&addrlen);
		            close(sd);
			    mydgwclient->client_socket = 0;
			    break;
		    }
		    else
		    {
			DGwSockMessage msg = *(DGwSockMessage*)&mydgwclient->bufferedData;
			while ((valread += read(sd, mydgwclient->bufferedData + valread, msg.MsgLen - header)) < msg.MsgLen){}

			if (msg.MsgType == TypePushFrameRsp)
			{
				//cout << "DGwSockets: TypePushFrameRsp received!\n";
				string avDevID(msg.devID);
				int configlen = msg.MsgLen - header;
				if (configlen > 0)
				{	
					streamConfig streamFormat;
					memcpy((void*)&streamFormat, (void*)(mydgwclient->bufferedData+header), configlen);
					int resultsLength = 0;
					string jsonDataStr = streamConfigToJSON(streamFormat);
	
					if ((streamFormat.stream == 1) || (streamFormat.stream == 2) || (streamFormat.stream == 3))
					{
						resultsLength = (streamFormat.resolution.width*streamFormat.resolution.height)*streamFormat.nochs*streamFormat.bytesps;
					}

					if (resultsLength > 0)
					{
						while ((valread += read(sd, mydgwclient->bufferedData + valread, resultsLength - valread + msg.MsgLen)) < msg.MsgLen + resultsLength){}

						unsigned char *frameData;
						frameData = (unsigned char *)(mydgwclient->bufferedData+header+configlen);
						mydgwclient->dgwclient->pushFrame(avDevID,frameData,jsonDataStr);
					}
				}

			}
			else if (msg.MsgType == TypePushMeasRsp)
			{
				//cout << "DGwSockets: TypePushMeasRsp received!\n";
				int dataoutlen = msg.MsgLen;
				vector<measurement> result;
				if (dataoutlen > header)
				{
					string jsonDataStr(mydgwclient->bufferedData+header, dataoutlen-header);
					string senMoteID(msg.devID);
					mydgwclient->dgwclient->pushMeasurements(senMoteID,jsonDataStr);
				}
			}
			else
			{
				mydgwclient->dataAvailable = true;
			}
		    }

		}
	}
}

IDGwSockets *getDGwSockets()
{
	if (!g_dgwsock)
		g_dgwsock = new DGwSockets();
	if (g_dgwsock)
		g_dgwsockCount++;
	return g_dgwsock;
}

void releaseDGwSockets(const IDGwSockets *pDGwSockets)
{
	if (g_dgwsock && (pDGwSockets == g_dgwsock))
	{
		g_dgwsockCount--;
		if(g_dgwsockCount == 0){
			delete g_dgwsock;
			g_dgwsock = NULL;
		}
	}
}

IDGwSockets::~IDGwSockets()
{
}

DGwSockets::~DGwSockets()
{
	delete [] sendMsgBuff;
	releaseDGwSockets(this);
}

DGwSockets::DGwSockets()
{
}

int DGwSockets::initializeDGwSockets()
{
	server_socket = 0;
	client_sockets.clear();
	client_socket = 0;
	runComm = false;
	dataAvailable = false;
	cout << "DGwSockets: DGwSockets initialized!\n";
	return 1;
}

int DGwSockets::initializeSocketServer(IDeviceGateway *dgw)
{
	dgwserv = dgw;
	initializeDGwSockets();
	runComm = true;
	pthread_create(&hthreadSocket, NULL, &threadRunServ, this);

	cout << "DGwSockets: DGwSockets initialized SocketClient!\n";
	return 1;
}

int DGwSockets::initializeSocketClient(IDGwClient *dgwc)
{
	dgwclient = dgwc;
	initializeDGwSockets();
	runComm = true;
	pthread_create(&hthreadSocket, NULL, &threadRunClient, this);

	cout << "DGwSockets: DGwSockets initialized SocketServer!\n";
	return 1;
}

int DGwSockets::sendDataToDGwClients(char *data, int length)
{
	int noUsers = client_sockets.size();
	send_struct users[noUsers];

	pthread_t hThSnd[noUsers];
	pthread_attr_t states[noUsers];
	vector<int>::iterator it; int iter = 0;

	for (it = client_sockets.begin(); it != client_sockets.end(); it++)
	{
	    users[iter].length = length;
	    users[iter].socket = (*it);
	    users[iter].data = data;
	    pthread_attr_init(&states[iter]);
	    pthread_attr_setdetachstate(&states[iter],PTHREAD_CREATE_JOINABLE);
	    pthread_create(&hThSnd[iter], &states[iter], threadRunSend, (void*)&users[iter]);
	    iter++;
	    if (iter > client_sockets.size())
		break;
	}

	for (int iUs = 0; iUs < noUsers; iUs++)
	    pthread_join(hThSnd[iUs],NULL);

	//cout << "DGwSockets: DGwSockets sends data to clients!\n";
	return 1;
}

int DGwSockets::queryDataFromDGw(char *data, int length, char **dataOut, int &dataOutLen)
{
	memset((void*)&bufferedData, 0, MAX_DATA);
	dataAvailable = false;
	cout << "DGwSockets: Client sends " << length << " bytes to DGw!\n";
	int datasnd = send(client_socket, data, length, 0);
	while (!dataAvailable) {cout << ".";}
	dataAvailable = false;
	DGwSockMessage msg = *(DGwSockMessage*)&bufferedData;
	dataOutLen = (int)(msg.MsgLen);
	*dataOut = new char[dataOutLen];
	memcpy(*dataOut, &bufferedData, dataOutLen);
	cout << "DGwSockets: Client receives " << dataOutLen << " bytes from DGw!\n";
	return 1;
}

int DGwSockets::closeSocketClient()
{
	int sd;
	runComm = false;
	vector<int>::iterator it;
	close(server_socket);
	server_socket = 0;
	for (it = client_sockets.begin(); it != client_sockets.end(); it++)
	{
	    sd = (*it);
	    close(sd);	
	}
	client_sockets.clear();
	
	cout << "DGwSockets: DGwSockets closes socket clients!\n";
	return 1;
}

int DGwSockets::closeSocketServer()
{
	close(client_socket);
	client_socket = 0;		
	runComm = false;
	cout << "DGwSockets: DGwSockets closes socket server!\n";
	return 1;
}
