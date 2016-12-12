#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "libfreenect.hpp"
#include <iostream>
#include <vector>
#include <cmath>
#include <pthread.h>

#include <libfreenect_sync.h>

//Define NO_DEVICEGATEWAY to read depth from stream without using Device Gateway 
//#define NO_DEVICEGATEWAY

#ifndef NO_DEVICEGATEWAY
#include "libDGwClientAV.h"
#include "DGWdefs.h"
#else //NO_DEVICEGATEWAY
#include <opencv2/highgui/highgui.hpp>
#endif //NO_DEVICEGATEWAY

// algorithm includes
#include "cTUSScrAct.hpp"

#include <X11/extensions/XTest.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/keysym.h>
#include <X11/keysymdef.h>

int xkey_init (char *display_name);
void xkey_send (long ks, int mod, int mod2);
void send_key(ScreenState_t curScreenState);

using namespace std;
using namespace cv;


uint16_t *pointerDepth = NULL;

class myMutex {
public:
	myMutex() {
		pthread_mutex_init(&m_mutex, NULL);
	}
	void lock() {
		pthread_mutex_lock(&m_mutex);
	}
	void unlock() {
		pthread_mutex_unlock(&m_mutex);
	}
private:
	pthread_mutex_t m_mutex;
};


class MyFreenectDevice : public Freenect::FreenectDevice {
public:
	MyFreenectDevice(freenect_context *_ctx, int _index)
		: Freenect::FreenectDevice(_ctx, _index), m_buffer_depth(FREENECT_DEPTH_11BIT),
		m_buffer_rgb(FREENECT_VIDEO_RGB), m_gamma(2048), m_new_rgb_frame(false),
		m_new_depth_frame(false), depthMat(Size(640, 480), CV_16UC1),
		rgbMat(Size(640, 480), CV_8UC3, Scalar(0)),
		ownMat(Size(640, 480), CV_8UC3, Scalar(0)) {

		for (unsigned int i = 0; i < 2048; i++) {
			float v = i / 2048.0;
			v = std::pow(v, 3) * 6;
			m_gamma[i] = v * 6 * 256;
		}
	}

	// Do not call directly even in child
	void VideoCallback(void* _rgb, uint32_t timestamp) {
		std::cout << "RGB callback" << std::endl;
		m_rgb_mutex.lock();
		uint8_t* rgb = static_cast<uint8_t*>(_rgb);
		rgbMat.data = rgb;
		m_new_rgb_frame = true;
		m_rgb_mutex.unlock();
	};

	// Do not call directly even in child
	void DepthCallback(void* _depth, uint32_t timestamp) {
		std::cout << "Depth callback" << std::endl;
		m_depth_mutex.lock();
		uint16_t* depth = static_cast<uint16_t*>(_depth);
		pointerDepth = depth;

		depthMat.data = (uchar*)depth;
		m_new_depth_frame = true;
		m_depth_mutex.unlock();
	}

	bool getVideo(Mat& output) {
		m_rgb_mutex.lock();
		if (m_new_rgb_frame) {
			cv::cvtColor(rgbMat, output, CV_RGB2BGR);
			m_new_rgb_frame = false;
			m_rgb_mutex.unlock();
			return true;
		}
		else {
			m_rgb_mutex.unlock();
			return false;
		}
	}

	bool getDepth(Mat& output) {
		m_depth_mutex.lock();
		if (m_new_depth_frame){
			depthMat.copyTo(output);
			m_new_depth_frame = false;
			m_depth_mutex.unlock();
			return true;
		}
		else {
			m_depth_mutex.unlock();
			return false;
		}
	}
private:
	std::vector<uint8_t> m_buffer_depth;
	std::vector<uint8_t> m_buffer_rgb;
	std::vector<uint16_t> m_gamma;
	Mat depthMat;
	Mat rgbMat;
	Mat ownMat;
	myMutex m_rgb_mutex;
	myMutex m_depth_mutex;
	bool m_new_rgb_frame;
	bool m_new_depth_frame;
};


/**
*
*
*
*/

IplImage *freenect_sync_get_depth_cv(int index)
{
	static IplImage *image = 0;
	static char *data = 0;
	if (!image) image = cvCreateImageHeader(cvSize(640, 480), 16, 1);
	unsigned int timestamp;
	if (freenect_sync_get_depth((void**)&data, &timestamp, index, FREENECT_DEPTH_MM))
		return NULL;
	cvSetData(image, data, 640 * 2);
	return image;
}

string SrcreenState[] = {"Passive","Active"};

Display *display = NULL;

int main(int argc, char *argv[]) 
{
	
	try{
	bool die(false);

	Mat depthMat;
	
	ScreenState_t screenStateOld = SCREEEN_PASSIVE;
	bool firsStateFound = false;

	// namedWindow("depth",CV_WINDOW_AUTOSIZE);

	// Init Algorithgm
	cTUSScrAct *Alg = new cTUSScrAct();
#ifndef NO_DEVICEGATEWAY
	DGwClientAV AVProcess;
	AVProcess.initializeDGwClientAV();
#endif //NO_DEVICEGATEWAY

	if (xkey_init(NULL)) { 
		fprintf(stderr,"Error: failed to connect to X server.\n");
	}

#ifndef NO_DEVICEGATEWAY
	while (AVProcess.readDepthFrame(depthMat))
#else
	while (1)
#endif
	{
#ifndef NO_DEVICEGATEWAY
		// convert Mat object to IplImage -> depthMapIPL
		IplImage copy = depthMat;
 		IplImage* depthMapIPL = &copy;
#else
		IplImage* depthMapIPL = freenect_sync_get_depth_cv(0);
#endif

		ScreenState_t curScreenState = Alg->Apply(depthMat);
		
		if (curScreenState != SCREEEN_NOT_READY) {
			if (firsStateFound) {
				if (curScreenState != screenStateOld) {
					send_key(curScreenState);
					screenStateOld = curScreenState;
				}
			} else {
				firsStateFound = true;
				send_key(curScreenState);
				screenStateOld = curScreenState;
			}

			//cvShowImage("depth", depthMapIPL);

			char k = cvWaitKey(25);
			if( k == 27 ){
				cvDestroyWindow("depth");
				break;
			}
		}
		
	}

	// Delete Algorithm
	delete Alg;

	return 0;

	}catch(const char *e){
		cout << "EXCEPTION " << e << endl;
	}catch(...){
		cout << "UNHANDELEED EXCEPTION " << endl;
	}

}

void send_key(ScreenState_t curScreenState) {
	
	// key symbols to be sent
	KeySym symbol;
	// can be prefixed by Ctrl+, Alt+, Shift, etc. (0 - for no prefix)
	int mod=2;
	int mod2=4;
	cout << SrcreenState[curScreenState] << "\n";
	if (curScreenState == SCREEEN_ACTIVE) {
		symbol = XStringToKeysym("a");
	} else {
		symbol = XStringToKeysym("k");
	}
	xkey_send(symbol,mod,mod2);
}


int xkey_init (char *display_name) {

	if (display_name == NULL) display_name = XDisplayName (NULL);
  	if ( (display=XOpenDisplay(display_name)) == NULL ) {
		fprintf( stdout, "Cannot connect to X server\n");
		return(-1);
	}
	return (0);
}

/*
*
* Sending keystok command
* ks = XStringToKeysym("w");
* mod = 0: no modifier - 1: Meta key pressed. - 2: CTRL key pressed
* - 3: ALT key pressed. 4: SHIFT key pressed
* mod2 can be used for second key (i.e. ctrl+shift+ ...)
*
*/

void xkey_send (long ks, int mod, int mod2) {
	int kc;

	if (mod) { 
		if (mod==1) kc = XKeysymToKeycode (display, XK_Meta_L);
		else if (mod==3) kc = XKeysymToKeycode (display, XK_Alt_L);
		else if (mod==4) kc = XKeysymToKeycode (display, XK_Shift_L);
		else kc = XKeysymToKeycode (display, XK_Control_L);
		XTestFakeKeyEvent(display, kc, True, 0);
		XFlush(display);
	}

	if (mod2) { 
		if (mod2==1) kc = XKeysymToKeycode (display, XK_Meta_L);
		else if (mod2==3) kc = XKeysymToKeycode (display, XK_Alt_L);
		else if (mod2==4) kc = XKeysymToKeycode (display, XK_Shift_L);
		else kc = XKeysymToKeycode (display, XK_Control_L);
		XTestFakeKeyEvent(display, kc, True, 0);
		XFlush(display);
	}

	kc = XKeysymToKeycode (display, ks);

	XTestFakeKeyEvent(display, kc, True, 0);
	XFlush(display);
	XTestFakeKeyEvent(display, kc, False, 0);
	XFlush(display);

	if (mod2) { 
		if (mod2==1) kc = XKeysymToKeycode (display, XK_Meta_L);
		else if (mod2==3) kc = XKeysymToKeycode (display, XK_Alt_L);
		else if (mod2==4) kc = XKeysymToKeycode (display, XK_Shift_L);
		else kc = XKeysymToKeycode (display, XK_Control_L);
		XTestFakeKeyEvent(display, kc, False, 0);
		XFlush(display);
	}

	if (mod) { 
		if (mod==1) kc = XKeysymToKeycode (display, XK_Meta_L);
		else if (mod==3) kc = XKeysymToKeycode (display, XK_Alt_L);
		else if (mod==4) kc = XKeysymToKeycode (display, XK_Shift_L);
		else kc = XKeysymToKeycode (display, XK_Control_L);
		XTestFakeKeyEvent(display, kc, False, 0);
		XFlush(display);
	}
}