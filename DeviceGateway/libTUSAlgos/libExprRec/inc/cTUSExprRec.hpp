#ifndef __cTUSExprRec__
#define __cTUSExprRec__

#include <cstdlib>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <vector>
#include <opencv2/opencv.hpp>

using namespace cv;
using namespace std;

// expressions
typedef enum{
	NEUTRAL = 0,
	ANGER,
	DISGUST, 
	FEAR, 
	HAPPY, 
	SADNESS, 
	SURPRISE
}Expression_t;

typedef struct{
    int expFacesID;
    int expr;
}expression;


class cTUSExprRec
{

public:
	cTUSExprRec(vector<string> modelFiles, int maxFaceNum, int expBuffLen, float confRatio);
	~cTUSExprRec();
	Expression_t estExpr(Mat imageFace);
    /**
        Apply expression recognition algorithm over a sequence of frames.
        Output parameter:
            expressions - vector of structs that contain
                          faces IDs along with their corresponding expressions.
                          Only faces IDs for which expresion recognition can be performed are returned
        Input parameters:
            facesIDs - vector of faces IDs (assigned by the face detector/tracker).
            facesRects - vector of faces rectangles (assigned by the face detector/tracker)
                         Each rectangle corresponds to the face ID speciffied in facesIDs.
            grScFrame - input video frame
        Return:
            This function returns true if there is a change between the last and the current results from the algorithm
            (i.e. from the last and the current frame) and an expression for at least one face can be recognized.
    */
    bool Apply(vector<expression>& expressions, vector<int> facesIDs, vector<Rect> facesRects, const Mat& grScFrame);

private:

	Mat numOfKernels;
	Mat kernels;
	Mat indexes;

	Mat KSVMScaleRange;
	Mat resizedImage;

	void *pctx;

	Mat scaleFeature(Mat reducedFeature, Mat KSVMScaleRange);
	Mat singleScRet(Mat inputImage);
    Mat imageAdjust(Mat src, float low_in, float high_in, float low_out, float high_out);
	float interp1(Mat X, Mat Y, float targetX);
	void createLightFilt(Mat* filt, int rows, int cols, int hsiz);
	void createPercArr(Mat* perc, int totSize);
    int getReprExpr(int* exprBuffer, int expBuffLen, int exprSize, float confRatio);
    bool compareStates(vector<expression> expressions, vector<expression> expressionsOld);
    vector<expression> expressionsOld;
};

#endif
