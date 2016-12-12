/*=================================================================================
Description: Performs simple visual processing

Authors: Nikolaos Katsarakis (AIT) (nkat@ait.edu.gr)

Revision History:
    13.11.2014, ver 0.1
====================================================================================*/

#include <iostream>
#include <opencv2/highgui/highgui.hpp>

class SimpleProcessing
{
public:
	SimpleProcessing();
	~SimpleProcessing();

	int processFrame(const cv::Mat& camFrame, bool *active, double *luminance);
	int showFrame(const cv::Mat& camFrame, const std::string& dispStr);

private:
};

