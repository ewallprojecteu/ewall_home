/*=================================================================================
Description: Performs face processing

Authors: Nikolaos Katsarakis (AIT) (nkat@ait.edu.gr)

Revision History:
    17.02.2015, ver 0.1
====================================================================================*/

#include "ExpRecogniser.h"

// ER algorithm variables
// Storage file names for ExprRec algorithm
const string ExpRecogniser::KernelsData = "./models/kernels_features.yml";
const string ExpRecogniser::KSVMScaleData = "./models/KSVM_scale_range.yml";
const string ExpRecogniser::SVMModelData = "./models/SVMModel.model";

// available expressions
const string ExpRecogniser::expressArr[] = {"NEUTRAL", "ANGER", "DISGUST", "FEAR", "HAPPINESS", "SADNESS", "SURPRISE"};

ExpRecogniser::ExpRecogniser()
{
    vector <string> modelFiles;

    // Init LGADASVM algorithm
    modelFiles.push_back(KernelsData);
    modelFiles.push_back(KSVMScaleData);
    modelFiles.push_back(SVMModelData);
    ExprRec = new cTUSExprRec(modelFiles, 10, 15, 0.6);
}

void ExpRecogniser::recogniseExpressions (const cv::Mat& grayFrame, std::vector<targetStruct> &targets, int dec)
{
    int numTargets = targets.size();
    if (numTargets == 0)
        return;

    facesRects.resize(numTargets);
    facesIDs.resize(numTargets);
    expressions.resize(numTargets);
    Mat drawFrame;
    Size graySize = grayFrame.size();
    Rect imgRect = Rect(Point(0,0),graySize);
//    cv::namedWindow("test",CV_WINDOW_AUTOSIZE);

    // Fill in required facesRects, facesIDs
    for (int i=0; i<numTargets; i++)
    {
        drawFrame = grayFrame.clone();
        targetStruct curTarget=targets[i];
        RotatedRect curFace=curTarget.face;
        curFace.center*=dec;
        curFace.size.width*=dec;
        curFace.size.height*=dec;
        facesRects[i] = curFace.boundingRect() & imgRect;
        facesIDs[i]= (int) curTarget.target_id;
//        cv::rectangle(drawFrame,facesRects[i], Scalar(127));
//        cv::imshow("test", drawFrame);

    }
    bool stateChanged = ExprRec->Apply(expressions, facesIDs, facesRects,  drawFrame);
    if (stateChanged)
        cout << "*********************  stateChanged ******************" << endl;
    for (int i=0; i<numTargets; i++)
    {
        int expId = expressions[i].expr;
        if (expId != -1) {
            cout << "i: " << i << " Face ID (from facesIDs): " << facesIDs[i]
                    << " Face ID (from expressions): " << expressions[i].expFacesID
                    << " Expression: " << expressArr[expId] << " (" << expId << ")" <<endl;
            targets[i].emotion = expressArr[expId];
        } else {
            cout << "i: " << i << " Face ID (from facesIDs): " << facesIDs[i]
                    << " Face ID (from expressions): " << expressions[i].expFacesID
                    << " Expression: UNKNOWN (" << expId << ")" <<endl;
            targets[i].emotion = "UNKNOWN";
        }

        targets[i].emotionConf = 0.6;
    }
}

