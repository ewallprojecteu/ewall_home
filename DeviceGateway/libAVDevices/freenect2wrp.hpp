#ifndef FREENECT2WRP_HPP
#define FREENECT2WRP_HPP

#include <iostream>
#include <signal.h>

#include <vector>
#include <cmath>
#include <pthread.h>
#include <opencv/cv.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include <opencv2/opencv.hpp>

#include <libfreenect2/libfreenect2.hpp>
#include <libfreenect2/frame_listener_impl.h>
//#include <libfreenect2/threading.h>

using namespace std;
using namespace cv;

#include <pthread.h>

class myMutex {
	public:
		myMutex() {
			pthread_mutex_init( &m_mutex, NULL );
		}
		void lock() {
			pthread_mutex_lock( &m_mutex );
		}
		void unlock() {
			pthread_mutex_unlock( &m_mutex );
		}
	private:
		pthread_mutex_t m_mutex;
};

static libfreenect2::Freenect2Device *dev = 0;
static libfreenect2::SyncMultiFrameListener listener(libfreenect2::Frame::Color | libfreenect2::Frame::Ir | libfreenect2::Frame::Depth);

class FreenectWRP {

    public: FreenectWRP() {
    if(freenect2.enumerateDevices() == 0) {
            std::cout << "no device connected!" << std::endl;
    }
    std::string serial = freenect2.getDefaultDeviceSerialNumber();
    dev = freenect2.openDevice(serial);
    if(dev == 0) {
      std::cout << "failure opening device!" << std::endl;
    }
    //signal(SIGINT,sigint_handler);
    //protonect_shutdown = false;
    dev->setColorFrameListener(&listener);
    dev->setIrAndDepthFrameListener(&listener);
    RGBready = depthready = false;
}

public: 
void startVideo() 
{
    dev->start();
    std::cout << "device serial: " << dev->getSerialNumber() << std::endl;
    std::cout << "device firmware: " << dev->getFirmwareVersion() << std::endl;
}

void startDepth() 
{
    dev->start();
    std::cout << "device serial: " << dev->getSerialNumber() << std::endl;
    std::cout << "device firmware: " << dev->getFirmwareVersion() << std::endl;
}

void startAudio() 
{
    std::cout << "audio is not supported for libfreenect2: " << dev->getSerialNumber() << std::endl;
}

void stopVideo() 
{
    dev->stop();
    dev->close();
}

void stopDepth() 
{
    dev->stop();
    dev->close();
}

void stopAudio() 
{
    
}

bool getRGBFrame(std::vector<uint8_t> &output, streamConfig &streamFormat)
{
    m_frame_mutex.lock();
    if (!RGBready)
    {
    	listener.release(frames);
    	listener.waitForNewFrame(frames);
        depthready = true;
    }

    libfreenect2::Frame *rgb = frames[libfreenect2::Frame::Color];
    Mat tmp;
    cv::Mat tmp2;
    cv::flip(cv::Mat(rgb->height, rgb->width, CV_8UC4, rgb->data), tmp, 1);
    cv::cvtColor(tmp, tmp2, CV_BGRA2RGB);
    streamFormat.resolution.width = rgb->width;
    streamFormat.resolution.height = rgb->height;
    streamFormat.nochs = 3;
    streamFormat.rate = 25;
    streamFormat.format = 1;
    streamFormat.stream = 1;
    streamFormat.bytesps = 1;
    int dataSize = streamFormat.resolution.height * streamFormat.resolution.width * streamFormat.nochs;
    if (output.size() != dataSize) output.resize(dataSize,0);
    std::copy(tmp2.data, tmp2.data+dataSize, output.begin());
    RGBready = false; 
    m_frame_mutex.unlock();
    return true;
}

bool getDepthFrame(std::vector<uint8_t> &output, streamConfig &streamFormat)
{
    m_frame_mutex.lock();
    if (!depthready)
    {
    	listener.release(frames);
    	listener.waitForNewFrame(frames);
        RGBready = true;
    }

    //libfreenect2::Frame *depth = frames[libfreenect2::Frame::Depth];
    libfreenect2::Frame *depth = frames[libfreenect2::Frame::Ir];
    streamFormat.resolution.width = depth->width;
    streamFormat.resolution.height = depth->height;
    streamFormat.nochs = 1;
    streamFormat.rate = 25;
    streamFormat.format = 1;
    streamFormat.stream = 2;
    streamFormat.bytesps = 2;
    Mat depthMat = Mat(depth->height, depth->width, CV_32FC1, depth->data);//1/65535.0;;// / 4500.0f;
    Mat temp(Size(depth->width, depth->height),CV_16UC1);
    depthMat.convertTo(temp, CV_16UC1, 1); 
    //imshow("Test Window",temp); 
    int dataSize = depth->height * depth->width * streamFormat.bytesps;
    if (output.size() != dataSize) output.resize(dataSize,0);
    std::copy(temp.data, temp.data+dataSize, output.begin());
    depthready = false;
    m_frame_mutex.unlock();
    return true;
}

bool getAudioSamples(void **output, streamConfig &streamFormat)
{
    return false;
}

private:
    libfreenect2::Freenect2 freenect2;
    streamConfig streamFormatRGB;
    streamConfig streamFormatDepth;
    libfreenect2::FrameMap frames;
    bool RGBready, depthready;
    myMutex m_frame_mutex;
};// end class def


#endif /*FREENECT2WRP_HPP*/
