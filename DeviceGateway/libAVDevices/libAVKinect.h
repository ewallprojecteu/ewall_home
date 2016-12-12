/*=================================================================================
Basic explanation:

Authors: Daniel Denkovski (UKIM) (danield@feit.ukim.edu.mk)

Revision History:
    25.08.2014, ver 0.1
====================================================================================*/

#ifndef LIBAVKINECT_H
#define LIBAVKINECT_H

#include "libAVDevice.h"

class AVKinect: public AVDevice
{
public:
	int initializeDevice(avDeviceInfo devInfo);

	~AVKinect();
	
	AVKinect();
	
	int startStream (streamConfig *conf);

	int reconfigureStream (streamConfig *conf);

	int stopStream (short streamtype);

	int rotateAVDevice (double angle);

	int pushFrame (void *frame, streamConfig streamFormat, std::string timestamp);

	static void* threadRunRGB(void *object);

	static void* threadRunDepth(void *object);

	static void* threadRunAudio(void *object);

	void *frnctwrp;

	bool stream[3]; //0 for RGB, 1 for depth, 2 for audio

private:
	pthread_t hThStream[3]; //0 for RGB, 1 for depth, 2 for audio

};

#endif
