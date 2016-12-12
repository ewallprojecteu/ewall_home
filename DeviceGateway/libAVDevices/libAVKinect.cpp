/*=================================================================================
Basic explanation:

    AV Kinect cpp code

Authors: Daniel Denkovski (UKIM) (danield@feit.ukim.edu.mk)
	 Krasimir Tonechv (TUS)  (k_tonchev@tu-sofia.bg)
Revision History:
    25.08.2014, ver 0.1 - Initial
    29.09.2014, ver 0.2 - Adapted for kinect
====================================================================================*/

// Define Kinect V2 (XBOX ONE) to be used (or if commented) - Kinect V1 ( i.e. XBOX 360) will be used
// #define KINECT_V2

#include "libAVKinect.h"

#ifndef KINECT_V2 // Kinect v1
#include "freenectwrp.hpp"
static Freenect::Freenect freenect;
static FreenectWRP* frnctwrp2 = NULL;
#else // Kinect v2
#include "freenect2wrp.hpp"
static FreenectWRP* frnctwrp = NULL;
#endif
#include <unistd.h>

// begin code for writting to audio file
static FILE *audio_file;
static int samples = 0;
static char wavheader[] = {
	   0x52, 0x49, 0x46, 0x46, // ChunkID = "RIFF"
	   0x00, 0x00, 0x00, 0x00, // Chunksize (will be overwritten later)
	   0x57, 0x41, 0x56, 0x45, // Format = "WAVE"
	   0x66, 0x6d, 0x74, 0x20, // Subchunk1ID = "fmt "
	   0x10, 0x00, 0x00, 0x00, // Subchunk1Size = 16
	   0x01, 0x00, 0x01, 0x00, // AudioFormat = 1 (linear quantization) | NumChannels = 1
	   0x80, 0x3e, 0x00, 0x00, // SampleRate = 16000 Hz
	   0x00, 0xfa, 0x00, 0x00, // ByteRate = SampleRate * NumChannels * BitsPerSample/8 = 64000
	   0x04, 0x00, 0x20, 0x00, // BlockAlign = NumChannels * BitsPerSample/8 = 4 | BitsPerSample = 32
	   0x64, 0x61, 0x74, 0x61, // Subchunk2ID = "data"
	   0x00, 0x00, 0x00, 0x00, // Subchunk2Size = NumSamples * NumChannels * BitsPerSample / 8 (will be overwritten later)
         };
// end of code for writting to audio file

static myMutex m_frame_mutex;

AVKinect::~AVKinect()
{
	stream[0] = stream[1] = stream[2] = false;
        pthread_join(hThStream[0],NULL);
	pthread_join(hThStream[1],NULL);
	pthread_join(hThStream[2],NULL);
}

AVKinect::AVKinect()
{
	stream[0] = stream[1] = stream[2] = false;
}

//eWall: This is where the constructor is called
int AVKinect::initializeDevice(avDeviceInfo devInfo)
{
	initialize(devInfo);
#ifndef KINECT_V2	
	// call constructors
	frnctwrp2 = &(freenect.createDevice<FreenectWRP>(0));
	frnctwrp = (void*)frnctwrp2;
#else
	frnctwrp = (void*)new FreenectWRP();
#endif
	//((FreenectWRP *)frnctwrp)->SetCallBacks(this);
	cout << "AVDevices: AVKinect after setting callbacks"<< endl;
	stream[0] = stream[1] = stream[2] = false;

	cout << "AVDevices: AVKinect with ID = " << deviceInfo.deviceID << " initialized!\n";
	return 1;
}

/**
* @brief: Here we will start the stream based on parameters streamConfig 
*/
int AVKinect::startStream (streamConfig *conf)
{
	//TODO: reconfigure Kinect based on conf
	
	//start stream based on conf
	short streamtype  = conf->stream;	
	if(streamtype == 1 ){ // start video stream 
		((FreenectWRP *)frnctwrp)->startVideo();
		stream[0] = true;
		pthread_create(&hThStream[0], NULL, &threadRunRGB, (void*)this);
	   	cout << "AVDevices: AVKinect starting video streaming"<< endl;			
	}
	else if(streamtype == 2 ){ // start depth stream
		((FreenectWRP *)frnctwrp)->startDepth();
		stream[1] = true;
		pthread_create(&hThStream[1], NULL, &threadRunDepth, (void*)this);
	   	cout << "AVDevices: AVKinect starting depth streaming"<< endl;			
	}
	else if(streamtype == 3 ){ // start audio stream
           	// begin code for writting to audio file
           	/*audio_file = fopen("audio_stream.wav", "wb");
           	fwrite(wavheader, 1, 44, audio_file);*/
           	// end of code for writting to audio file
	   	((FreenectWRP *)frnctwrp)->startAudio();
		stream[2] = true;
		pthread_create(&hThStream[2], NULL, &threadRunAudio, (void*)this);
	   	cout << "AVDevices: AVKinect starting audio streaming"<< endl;			
	}
	else
	   	cout << "AVDevices: Unknown stream type"<< endl;

	cout << "AVDevices: AVKinect with ID = " << deviceInfo.deviceID << " started streaming with rate = " << conf->rate << "Hz and resolution of " << conf->resolution.width << "x" << conf->resolution.height << "!\n";
	return 1;
}

int AVKinect::reconfigureStream (streamConfig *conf)
{
	short streamtype  = conf->stream;
	cout << "AVDevices: AVKinect with ID = " << deviceInfo.deviceID << " reconfigured streaming to rate = " << conf->rate << "Hz and resolution of " << conf->resolution.width << "x" << conf->resolution.height << "!\n";
	return 1;
}

/**
* @brief: Here we stop the streams
*/
int AVKinect::stopStream (short streamtype)
{
	if(streamtype == 1 ) // stop video stream 
	{
	    ((FreenectWRP *)frnctwrp)->stopVideo();
	    stream[0] = false;
	    pthread_join(hThStream[0],NULL);
	}
	else if(streamtype == 2 ) // stop depth stream
	{
	    ((FreenectWRP *)frnctwrp)->stopDepth();
	    stream[1] = false;
	    pthread_join(hThStream[1],NULL);
	}
	else if(streamtype == 3 ) // stop audio stream
	{
// begin code for writting to audio file
// Make the WAV header valid
	/*char buf[4];
	fseek(audio_file, 4, SEEK_SET);*/
	// Write ChunkSize = 36 + subchunk2size
	/*int chunksize = samples * 4 + 36;
	buf[0] = (chunksize & 0x000000ff);
	buf[1] = (chunksize & 0x0000ff00) >> 8;
	buf[2] = (chunksize & 0x00ff0000) >> 16;
	buf[3] = (chunksize & 0xff000000) >> 24;
	fwrite(buf, 1, 4,audio_file);
	fseek(audio_file, 40, SEEK_SET);*/
	// Write Subchunk2Size = NumSamples * NumChannels (1) * BitsPerSample/8 (4)
	/*int subchunk2size = samples * 4;
	buf[0] = (subchunk2size & 0x000000ff);
	buf[1] = (subchunk2size & 0x0000ff00) >> 8;
	buf[2] = (subchunk2size & 0x00ff0000) >> 16;
	buf[3] = (subchunk2size & 0xff000000) >> 24;
	fwrite(buf, 1, 4, audio_file);
        fclose(audio_file);*/
// end of code for writting to audio file

	    ((FreenectWRP *)frnctwrp)->stopAudio();
	    stream[2] = false;
	    pthread_join(hThStream[2],NULL);
	}
	cout << "AVDevices: AVKinect with ID = " << deviceInfo.deviceID << " stopped streaming!\n";
	return 1;
}

int AVKinect::rotateAVDevice (double angle)
{
	cout << "AVDevices: AVKinect with ID = " << deviceInfo.deviceID << " rotated for " << angle << " degrees!\n";
	return 1;
}

/**
* @brief: This have to be called from callback
*/
int AVKinect::pushFrame (void *frame, streamConfig streamFormat, string timestamp)
{
	//cout << "AVDevices: AVKinect with ID = " << deviceInfo.deviceID << " pushed frame to DeviceGateway!\n";
	//cv::Mat rgbMat(cv::Size(256,4),CV_8UC3,cv::Scalar(0));
	//rgbMat.data = (unsigned char *) frame;
	//cv::cvtColor(rgbMat, rgbMat, CV_RGB2BGR);
	//cv::namedWindow("rgb",CV_WINDOW_AUTOSIZE);
	//cv::imshow("rgb", rgbMat);
        //cv::waitKey(30);

// begin code for writting to audio file
        /*cv::Mat audioMat(Size(256,1),CV_32SC1,Scalar(0));
        uchar *p = audioMat.data;
        memcpy(p, frame, 256*sizeof(int32_t));

        fwrite(p, 1, 256*sizeof(int32_t), audio_file);
        samples += 256;*/
// end of code for writting to audio file

     //   cout << "Calling pushFrame\n";
    //cv::imshow("rgb", cv::Mat(streamFormat.resolution.height, streamFormat.resolution.width, CV_8UC4, frame)); // kinect v2
    //char k = cvWaitKey(5);

    return devGW->pushFrame(deviceInfo.deviceID, frame, streamFormat, timestamp); // call pushFrame in LibDeviceGateway
}

void* AVKinect::threadRunRGB(void *object)
{
	void *frame;
	std::vector<uint8_t> buffer;
	streamConfig streamFormat;
	AVKinect* myKinect = static_cast<AVKinect*>(object); 

	cout << "Starting RGB thread!" << endl;
	while (myKinect->stream[0]) {
		usleep(80000);
    		m_frame_mutex.lock();
        	if (((FreenectWRP *)(myKinect->frnctwrp))->getRGBFrame(buffer,streamFormat)) {
			frame = (void*)(&buffer[0]);
                	myKinect->pushFrame(frame, streamFormat, string("0000000000"));
		}
    		m_frame_mutex.unlock();

    }
	cout << "Exiting RGB thread!" << endl;
	pthread_exit(NULL);
}

void* AVKinect::threadRunDepth(void *object)
{
	void *frame;
	std::vector<uint8_t> buffer;
	streamConfig streamFormat;
	AVKinect* myKinect = static_cast<AVKinect*>(object);

	cout << "Starting depth thread!" << endl;

	while (myKinect->stream[1]) {
		usleep(80000);
    		m_frame_mutex.lock();
		if (((FreenectWRP *)(myKinect->frnctwrp))->getDepthFrame(buffer,streamFormat)) {
			frame = (void*)(&buffer[0]);
			Mat depthMat = Mat(streamFormat.resolution.height, streamFormat.resolution.width, CV_16UC1, frame);//depth->data) / 4500.0f;
			myKinect->pushFrame(frame, streamFormat, string("0000000000"));
        	}
    		m_frame_mutex.unlock();
	}
	cout << "Exiting depth thread!" << endl;
	pthread_exit(NULL);//
}

void* AVKinect::threadRunAudio(void *object)
{
	void *frame;
	streamConfig streamFormat;
	AVKinect* myKinect = static_cast<AVKinect*>(object);
	cout << "Starting audio thread!" << endl;

	while (myKinect->stream[2]) {
		usleep(1000);
		if (((FreenectWRP *)(myKinect->frnctwrp))->getAudioSamples(&frame,streamFormat))
			myKinect->pushFrame(frame, streamFormat, string("0000000000"));
	}
	cout << "Exiting audio thread!" << endl;
	pthread_exit(NULL);
}
