/*=================================================================================
Description: Wrapper for TUS Expression Recognition Library

Authors: Nikolaos Katsarakis (AIT) (nkat@ait.edu.gr)

Revision History:
    2014-10-01, ver 0.1
====================================================================================*/

#ifndef EXPRECOGNISER_H
#define EXPRECOGNISER_H

#include <iostream>
#include <opencv2/highgui/highgui.hpp>
//#include <opencv2/imgproc/imgproc.hpp>
//#include <opencv2/video/video.hpp>
#include "tracker.h"

// Currently expression recognition library only available
// in Linux compiled form
#ifndef _WIN32
#include "cTUSExprRec.hpp"
#endif //

class ExpRecogniser
{
public:
    ExpRecogniser();
    void recogniseExpressions (const cv::Mat& grayFrame, std::vector<targetStruct> &targets, int dec);

private:
    // ER algorithm variables
    // Storage file names for ExpRec algorithm
    static const std::string KernelsData;
    static const std::string KSVMScaleData;
    static const std::string SVMModelData;

    // available expressions
    static const string expressArr[];

    cTUSExprRec *ExprRec;

    // define vector as sequences of faces IDs (i.e. Identficator number for each face found)
    std::vector<int> facesIDs;

    // define vector as sequences of rectangles (i.e. faces bounding boxes)
    std::vector<cv::Rect> facesRects;

    // define vector as a sequences of faces IDs and their corresponding expressions (to be returned from the ExpRec algorithm)
    std::vector<expression> expressions;
};

#endif //EXPRECOGNISER_H
