#include <opencv2/highgui/highgui.hpp>
#include <string>
#include "tracker.h"

#ifndef DISPLAY_H
#define DISPLAY_H

#define CV_RED   cv::Scalar(0,0,255)
#define CV_PINK  cv::Scalar(63,0,255)
#define CV_GREEN cv::Scalar(0,255,0)
#define CV_BLUE  cv::Scalar(255,0,0)
#define CV_CYAN  cv::Scalar(255,63,0)
#define CV_ORANGE cv::Scalar(0,63,255)

cv::Size writeText(cv::Mat& image, const std::string& text, cv::Point position=cv::Point(), 
	double fontScale=0.7, cv::Scalar color=cv::Scalar(0,255,0));
void drawTarget(cv::Mat& Irgb, std::vector<targetStruct>& targets, unsigned int targetIdx, 
	int64 framenum /*for drawing target distance*/, int dec=1 /*decimation of the targets*/);
void setDisplayParams(double _capture_fps, int _faceWidth);
cv::Size drawPlot(cv::Mat &Irgb, cv::Rect drawRegion, std::vector<int> data, const char* dataFormat, 
	std::string title, std::vector<std::string> labels, 
	int plotType, cv::Scalar barColor, cv::Scalar textColor);

#endif // DISPLAY_H