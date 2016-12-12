#ifndef FREENECTWRP_HPP
#define FREENECTWRP_HPP

#include "libfreenect_ewall.hpp"
#include <iostream>
#include <vector>
#include <cmath>
#include <pthread.h>
#include <opencv/cv.h>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

#include "IAVDevices.h"

using namespace cv;
using namespace std;

#define DISPLAY_DEBUG 0

//typedef void (*pushFrame_t) (void *frame, streamConfig streamFormat, string timestamp);

/**
* @brief: Simple wrapper around pthread mutex
*
*
*/
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


/**
* @brief: Wrapper around freenect device
*
*
*/
class FreenectWRP : public Freenect::FreenectDevice {
	
public:
	     FreenectWRP(freenect_context *_ctx, int _index) : Freenect::FreenectDevice(_ctx, _index), 
			//m_buffer_depth(FREENECT_DEPTH_11BIT),
			m_buffer_depth(FREENECT_DEPTH_MM),//freenect_find_video_mode(FREENECT_RESOLUTION_MEDIUM, FREENECT_VIDEO_RGB).bytes),
			//m_buffer_rgb(FREENECT_VIDEO_RGB), 
			m_buffer_rgb(0),//freenect_find_video_mode(FREENECT_RESOLUTION_MEDIUM, FREENECT_VIDEO_RGB).bytes),
			//m_buffer_audio(FREENECT_VIDEO_RGB), 
			m_buffer_audio(0),
			//m_gamma(2048), 
			m_new_rgb_frame(false),
			m_new_depth_frame(false),
			m_new_audio_frame(false)/*,
			depthMat(Size(640,480),CV_16UC1),
			rgbMat(Size(640,480), CV_8UC3, Scalar(0)),
			ownMat(Size(640,480),CV_8UC3,Scalar(0)),
            		audioMat(Size(1024,4),CV_32SC1,Scalar(0))*/            
		{
			
		}
		
		/*void SetCallBacks(IAVDevice *ptr)
		{
			avdevptr = ptr;					
		}*/

		// Do not call directly even in child
		void VideoCallback(void* _rgb, uint32_t timestamp) {
#if DISPLAY_DEBUG
			std::cout << "RGB callback" << std::endl;
#endif
			m_rgb_mutex.lock();
			uint8_t* rgb = static_cast<uint8_t*>(_rgb);
			int dataSize = getVideoBufferSize();
			if (m_buffer_rgb.size() != dataSize) m_buffer_rgb.resize(dataSize,0);
			std::copy(rgb, rgb+getVideoBufferSize(), m_buffer_rgb.begin());
			m_new_rgb_frame = true;
			m_rgb_mutex.unlock();

			/*streamFormat.resolution.width = 640;
			streamFormat.resolution.height = 480;
			streamFormat.nochs = 3;
			streamFormat.rate = 25;
			streamFormat.format = 1;
			streamFormat.stream = 1;
			streamFormat.bytesps = 1;
			avdevptr->pushFrame (_rgb, streamFormat, string("0000000000"));*/

		};
		
		// Do not call directly even in child
		void DepthCallback(void* _depth, uint32_t timestamp) {
#if DISPLAY_DEBUG
			std::cout << "Depth callback" << std::endl;
#endif
			m_depth_mutex.lock();
			uint16_t* depth = static_cast<uint16_t*>(_depth);
			int dataSize = getDepthBufferSize();
			if (m_buffer_depth.size() != dataSize) m_buffer_depth.resize(dataSize,0);
			std::copy(depth, depth+getDepthBufferSize(), m_buffer_depth.begin());
			m_new_depth_frame = true;
			m_depth_mutex.unlock();

			//depthMat.data = (uchar*) depth;
			/*streamFormat.resolution.width = 640;
			streamFormat.resolution.height = 480;
			streamFormat.nochs = 1;
			streamFormat.rate = 25;
			streamFormat.format = 1;
			streamFormat.stream = 2;
			streamFormat.bytesps = 2;
			avdevptr->pushFrame (_depth, streamFormat, string("0000000000"));*/

		}
		
		void AudioCallback(int num_samples, int32_t* mic1, int32_t* mic2, int32_t* mic3, int32_t* mic4, uint32_t timestamp) {
#if DISPLAY_DEBUG
			std::cout << "Audio callback" << std::endl;
#endif
			m_audio_mutex.lock();
			int dataSize = num_samples*sizeof(int32_t)*4;
			if (m_buffer_audio.size() != dataSize) m_buffer_audio.resize(dataSize,0);
			int audioCount = 0;
			std::copy(mic1, mic1+num_samples*sizeof(int32_t), &m_buffer_audio[audioCount]);
			audioCount += num_samples*sizeof(int32_t);
			std::copy(mic2, mic2+num_samples*sizeof(int32_t), &m_buffer_audio[audioCount]);
			audioCount += num_samples*sizeof(int32_t);
			std::copy(mic3, mic3+num_samples*sizeof(int32_t), &m_buffer_audio[audioCount]);
			audioCount += num_samples*sizeof(int32_t);
			std::copy(mic4, mic4+num_samples*sizeof(int32_t), &m_buffer_audio[audioCount]);
			m_new_audio_frame = true;
			m_audio_mutex.unlock();

			/*streamFormat.resolution.width = num_samples;
			streamFormat.resolution.height = 1;
			streamFormat.nochs = 4;
			streamFormat.rate = 16000;
			streamFormat.format = 2;
			streamFormat.stream = 3;
			streamFormat.bytesps = 4;*/
			/*audioMat = Mat(Size(num_samples,4),CV_32SC1);
			uint8_t *p = audioMat.data;
			memcpy(p, mic1, num_samples*sizeof(int32_t)); p += num_samples*sizeof(int32_t);
			memcpy(p, mic2, num_samples*sizeof(int32_t)); p += num_samples*sizeof(int32_t);
			memcpy(p, mic3, num_samples*sizeof(int32_t)); p += num_samples*sizeof(int32_t);
			memcpy(p, mic4, num_samples*sizeof(int32_t));*/
			//avdevptr->pushFrame ((void*)audioMat.data, streamFormat, string("0000000000"));
		}
        
		/*bool getVideo(Mat& output) {
			m_rgb_mutex.lock();
			if(m_new_rgb_frame) {
				cv::cvtColor(rgbMat, output, CV_RGB2BGR);
				m_new_rgb_frame = false;
				m_rgb_mutex.unlock();
				return true;
			} else {
				m_rgb_mutex.unlock();
				return false;
			}
		}
		
		bool getDepth(Mat& output) {
				m_depth_mutex.lock();
				if(m_new_depth_frame) {
					depthMat.copyTo(output);
					m_new_depth_frame = false;
					m_depth_mutex.unlock();
					return true;
				} else {
					m_depth_mutex.unlock();
					return false;
				}
			}*/

		bool getRGBFrame(std::vector<uint8_t> &output/*void **output*/, streamConfig &streamFormat) {
			m_rgb_mutex.lock();	
			if(m_new_rgb_frame) {
				//cv::cvtColor(rgbMat, output, CV_RGB2BGR
    				if (output.size() != m_buffer_rgb.size()) output.resize(m_buffer_rgb.size(),0);
				output.swap(m_buffer_rgb);
				//*output = &(*m_buffer_rgb.begin());
				streamFormat.resolution.width = 640;
				streamFormat.resolution.height = 480;
				streamFormat.nochs = 3;
				streamFormat.rate = 25;
				streamFormat.format = 1;
				streamFormat.stream = 1;
				streamFormat.bytesps = 1;

				m_new_rgb_frame = false;
				m_rgb_mutex.unlock();
				return true;
			} else {
				m_rgb_mutex.unlock();
				return false;
			}		
		}

		bool getDepthFrame(std::vector<uint8_t> &output/*void **output*/, streamConfig &streamFormat) {
			m_depth_mutex.lock();
			if(m_new_depth_frame) {
				//depthMat.copyTo(output);
    				if (output.size() != m_buffer_depth.size()) output.resize(m_buffer_depth.size()*2,0);
				uint8_t* depth = (uint8_t*)&m_buffer_depth[0];
    				std::copy(depth, depth + output.size(), output.begin());
    				//output.swap(m_buffer_depth);
				//*output = (void*)&m_buffer_depth[0];//.begin();
				streamFormat.resolution.width = 640;
				streamFormat.resolution.height = 480;
				streamFormat.nochs = 1;
				streamFormat.rate = 25;
				streamFormat.format = 1;
				streamFormat.stream = 2;
				streamFormat.bytesps = 2;

				m_new_depth_frame = false;
				m_depth_mutex.unlock();
				return true;
			} else {
				m_depth_mutex.unlock();
				return false;
			}	
		}

		bool getAudioSamples(void **output, streamConfig &streamFormat) {
			m_audio_mutex.lock();
			if(m_new_audio_frame) {
				//depthMat.copyTo(output);
				*output = (void*)&m_buffer_audio[0];//.begin();
				streamFormat.bytesps = 4;
				streamFormat.nochs = 4;
				streamFormat.resolution.height = 1;
				streamFormat.resolution.width = m_buffer_audio.size()/streamFormat.bytesps/streamFormat.nochs;
				streamFormat.rate = 16000;
				streamFormat.format = 2;
				streamFormat.stream = 3;

				m_new_audio_frame = false;
				m_audio_mutex.unlock();
				return true;
			} else {
				m_audio_mutex.unlock();
				return false;
			}				
		}
	private:
		std::vector<uint16_t> m_buffer_depth;
		std::vector<uint8_t> m_buffer_rgb;
		std::vector<uint8_t> m_buffer_audio;
		/*std::vector<uint16_t> m_gamma;
		Mat depthMat;
		Mat rgbMat;
		Mat ownMat;
		Mat audioMat;*/
		myMutex m_rgb_mutex;
		myMutex m_depth_mutex;
		myMutex m_audio_mutex;
		bool m_new_rgb_frame;
		bool m_new_depth_frame;
		bool m_new_audio_frame;
		//streamConfig streamFormat;
		//IAVDevice *avdevptr;
};
#endif /*FREENECTWRP_HPP*/

