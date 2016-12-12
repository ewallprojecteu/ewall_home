#include "cTUSExprRec.hpp"
#include "svm.h"

using namespace std;
using namespace cv;

#define Malloc(type,n) (type *)malloc((n)*sizeof(type))

typedef struct{
	int numOfKernelsInt;
	int kernelWidth;
	int kernelHeight;
	int maxFaceNum;
	int localFrameID;
	int expBuffLen;
	float confRatio;
    int exprSize;

	svm_node* x;    
	svm_model* SVMModel;
	Mat* filt;
	Mat* perc;
    int **exprBuffer;
    int *histoOcc;
    int *idLookUp;
}algctx_t;

/**
*/
cTUSExprRec::cTUSExprRec(vector<string> modelFiles, int maxFaceNum, int expBuffLen, float confRatio)
{

	// allocate algorithm context
	pctx = (void *) new char[sizeof(algctx_t)];

	// get the filenames
	string KernelsData = modelFiles.at(0);
	string KSVMScaleData = modelFiles.at(1);
	string SVMModelData = modelFiles.at(2);

	//Load Kernels and features data
	FileStorage KernelsDataFile(KernelsData, FileStorage::READ);
	if (!KernelsDataFile.isOpened())    throw "[LGADASVM] ERROR: Unable to open Kernels data file";
	KernelsDataFile["numOfKernels"] >> numOfKernels;
	KernelsDataFile["kernels"] >> kernels;

	// get number of kernels as integer type
	((algctx_t*)pctx)->numOfKernelsInt = (int)numOfKernels.at<float>(0,0);

	// get kernel sizes (will be the same as image sizes)
	((algctx_t*)pctx)->kernelWidth = kernels.cols;
	((algctx_t*)pctx)->kernelHeight = kernels.rows/((algctx_t*)pctx)->numOfKernelsInt;
	KernelsDataFile["indexes"] >> indexes;

	KernelsDataFile.release();

	// Load Scale Range File
	FileStorage KSVMScaleDataFile(KSVMScaleData, FileStorage::READ);
	if (!KSVMScaleDataFile.isOpened())  throw "[LGADASVM] ERROR: Unable to open Scale data file";
	KSVMScaleDataFile["KSVMScaleRange"] >> KSVMScaleRange;
	KSVMScaleDataFile.release();

	// Load SVM model
	((algctx_t*)pctx)->SVMModel = svm_load_model(SVMModelData.c_str());

	// Allocate memory for x - structure so called svm_node of size featureSize+1. Adding -1 is intended to mark the end of the feature vector.
	((algctx_t*)pctx)->x = Malloc(svm_node, indexes.rows+1);

	int rows = ((algctx_t*)pctx)->kernelHeight;
	int cols = ((algctx_t*)pctx)->kernelWidth;

	// allocate memory for light filter
	((algctx_t*)pctx)->filt = new Mat(cv::Mat::zeros(rows, cols, CV_32F));

	// create light filter
	createLightFilt(((algctx_t*)pctx)->filt, rows, cols, 15);

	// allocate array of indices of pixel value order as a percentage from 0 - 100 (to be used for histogram truncation)
	((algctx_t*)pctx)->perc = new Mat(cv::Mat::zeros(1, rows*cols+2, CV_32F));

	// create array of indices
	createPercArr(((algctx_t*)pctx)->perc, rows*cols+2);

	// set the maximum number of faces that algorithm allows
	((algctx_t*)pctx)->maxFaceNum = maxFaceNum;

	// set the buffer length (i.e. the number of frames required to estimate expression in the time domain for a given face)
	((algctx_t*)pctx)->expBuffLen = expBuffLen;

	// set the confidential ratio that will be used to get representative expression
	((algctx_t*)pctx)->confRatio = confRatio;

	// initialize local frame identifier (i.e the counter of frames in the expression buffer)
	((algctx_t*)pctx)->localFrameID = 0;

    ((algctx_t*)pctx)->exprSize = 7;
	
	// alocate dynamic expresions buffer
    ((algctx_t*)pctx)->exprBuffer = new int*[((algctx_t*)pctx)->maxFaceNum];

	// initialize expressions buffer with values of -1. This means that there is no expression.
	for (int i = 0; i < ((algctx_t*)pctx)->maxFaceNum; i++) {  // for each row (i.e. faceID)
        ((algctx_t*)pctx)->exprBuffer[i] = new int[((algctx_t*)pctx)->expBuffLen];
		for (int j = 0; j < ((algctx_t*)pctx)->expBuffLen; j++) { // for each column (i.e. frameID)
            ((algctx_t*)pctx)->exprBuffer[i][j] = -1;
		}
	}

    // allocate a histogram of occurences
    ((algctx_t*)pctx)->histoOcc = new int[((algctx_t*)pctx)->exprSize];

    ((algctx_t*)pctx)->idLookUp = new int[((algctx_t*)pctx)->maxFaceNum];

    // initialize idLookUp
    for (int i = 0; i < ((algctx_t*)pctx)->maxFaceNum; i++) {
        ((algctx_t*)pctx)->idLookUp[i] = -1;
    }
}

cTUSExprRec::~cTUSExprRec()
{
    // Free all alocated mem
    delete [] ((algctx_t*)pctx)->idLookUp;

    delete [] ((algctx_t*)pctx)->histoOcc;

    for (int i = 0; i < ((algctx_t*)pctx)->maxFaceNum; i++) {  // for each row (i.e. faceID)
        delete [] (((algctx_t*)pctx)->exprBuffer[i]);
    }

    delete [] (((algctx_t*)pctx)->exprBuffer);
    delete (((algctx_t*)pctx)->perc);
    delete (((algctx_t*)pctx)->filt);
    free(((algctx_t*)pctx)->x);
	// clear svm model
    svm_free_and_destroy_model(&((algctx_t*)pctx)->SVMModel);
	// delete algorithm context
    delete (char*)pctx;
}

void cTUSExprRec::createLightFilt(Mat* filt, int rows, int cols, int hsiz) {
    int cent = ceil(rows/2.0) - 1;
    float summ = 0;
	int i, j;
	float* p;
	for ( i = 0; i < rows; i++) {
		p = filt->ptr<float>(i); 
		for ( j = 0; j < cols; j++) {
			float radius = ((cent-i)*(cent-i)+(cent-j)*(cent-j));
			float entr = exp(-(radius/(hsiz*hsiz)));
			p[j] = exp(-(radius/(hsiz*hsiz)));
			summ = summ + filt->at<float>(i,j);
		}
	}
    *filt = *filt/summ;
}

void cTUSExprRec::createPercArr(Mat* perc, int totSize) {
	// get indices of pixel value order as a percentage from 0 - 100
	int i;
	for (i = 0; i < totSize-2; i++) {
	    perc->at<float>(0, i+1) = 100*(i + 0.5)/(totSize-2);
	}
	perc->at<float>(0, totSize - 1) = 100.0;
}

/**
*/
cv::Mat cTUSExprRec::scaleFeature(Mat reducedFeature, Mat KSVMScaleRange) 
{
	// define boundaries of range
	float y_lower = -1;
	float y_upper = 1;

	int featureSize = reducedFeature.cols;
	int scaleSize = KSVMScaleRange.rows;
	if (featureSize != scaleSize) 
		throw  "[PCASMV] ERROR: Feature size and the number of scale range indexes do not mutch!";

	Mat scaledFeature = Mat(1, featureSize, CV_32FC1);

	int i;
	float y_min, y_max, value;

	// for each element of the feature
	for (i = 0; i < featureSize; i++) {
		y_min = KSVMScaleRange.at<float>(i,0);
		y_max = KSVMScaleRange.at<float>(i,1);
		value = reducedFeature.at<float>(0,i);
		if(value == y_min)
			value = y_lower;
		else if(value == y_max)
			value = y_upper;
		else 
			value = y_lower + (y_upper-y_lower) *(value - y_min)/(y_max-y_min);
		scaledFeature.at<float>(0,i) = value;
	}
	return scaledFeature;
}

/**
*/
bool cTUSExprRec::Apply(vector<expression>& expressions, vector<int> facesIDs, vector<Rect> facesRects, const Mat& grScFrame) {

	// check for consistency
	if (facesIDs.empty() || facesIDs.size() != facesRects.size() || grScFrame.empty()) {
	    return false;
	}

	// get the maximum number of faces that algorithm allows
	int maxFaceNum = ((algctx_t*)pctx)->maxFaceNum;

	if (facesRects.size() > maxFaceNum) {
		cout << "Too many faces found: " << facesRects.size() << "! The maximum number of faces that algorithm allows is: " << maxFaceNum << endl; 
	    return false;
	}

	// get the number of frames required to estimate expression in the time domain for a given face
	int expBuffLen = ((algctx_t*)pctx)->expBuffLen;

	// reset localFrameID if it reaches expBuffLen
	if (((algctx_t*)pctx)->localFrameID == expBuffLen) {
	    ((algctx_t*)pctx)->localFrameID = 0;
	}

    // scan the last idLookUp record and check for macthing with facesIDs given
    for (int localFaceId = 0; localFaceId < maxFaceNum; localFaceId++) {

        // if idLookUp does not contain neither of facesIDs given, then the record idLookUp[locaFaceId] should be set to -1
        if(!(find(facesIDs.begin(), facesIDs.end(), ((algctx_t*)pctx)->idLookUp[localFaceId]) != facesIDs.end())) {
            ((algctx_t*)pctx)->idLookUp[localFaceId] = -1;
            for (int i = 0; i < expBuffLen; i++)
                ((algctx_t*)pctx)->exprBuffer[localFaceId][i] = -1;
        }
    }

    int idx = 0;

    // put faces IDs in idLookUp at the free positions (i.e. at -1s)
    for (int facesIDsIdx = 0; facesIDsIdx < facesIDs.size(); facesIDsIdx++) {
        bool match = false;

        while (((algctx_t*)pctx)->idLookUp[idx] != -1) {
            idx++;
        }

        // for each record in the idLookUp
        for (int localFaceId = 0; localFaceId < maxFaceNum; localFaceId++) {
            if (facesIDs.at(facesIDsIdx) == ((algctx_t*)pctx)->idLookUp[localFaceId]) {
                match = true;
                break;
            }
         }

         // if not match put facesIDs.at(facesIDsIdx) at the first met -1 in idLookUp
         if (!match) {
            ((algctx_t*)pctx)->idLookUp[idx] = facesIDs.at(facesIDsIdx);
         }

    }

	
    // for each possible localFaceId (0 to maxFaceNum-1)
    for (int localFaceId = 0; localFaceId < maxFaceNum; localFaceId++) {

        if (((algctx_t*)pctx)->idLookUp[localFaceId] != -1) {

            int pos = find(facesIDs.begin(), facesIDs.end(), ((algctx_t*)pctx)->idLookUp[localFaceId]) - facesIDs.begin();

		    // get curent face rectangle
			Rect curFaceRect = facesRects.at(pos);

			// increase height a little
			int incr = (int) (0.15*curFaceRect.height + 0.5);
			curFaceRect.height = curFaceRect.height + incr;

			// if out of bounds, decrease height
			if (curFaceRect.y + curFaceRect.height >= grScFrame.rows) {
				    curFaceRect.height = grScFrame.rows - curFaceRect.y;
			}

            Expression_t expression = estExpr(grScFrame(curFaceRect));

			// fill the buffer array with the expression estimated by the algorithm
            ((algctx_t*)pctx)->exprBuffer[localFaceId][((algctx_t*)pctx)->localFrameID] = expression;
		
		} else {
            // fill the buffer array with -1 (i.e. no expression esitmation for this localFaceId)
			for (int i = 0; i < expBuffLen; i++)
            ((algctx_t*)pctx)->exprBuffer[localFaceId][i] = -1;
		}

	} // end for
	
    bool stateChanged = false;
    expressions.clear();
    vector<expression> expressionsTemp;

	// analyse exprBuffer and set the return parameters accordingly
    for (int localFaceId = 0; localFaceId < maxFaceNum; localFaceId++) {
        // return representative expression index or -1 if its confidence is too small
        int exprID = getReprExpr(((algctx_t*)pctx)->exprBuffer[localFaceId], expBuffLen, ((algctx_t*)pctx)->exprSize, ((algctx_t*)pctx)->confRatio);
        if (exprID != -1) {
            expression currExp;
            currExp.expFacesID = ((algctx_t*)pctx)->idLookUp[localFaceId];
            currExp.expr = exprID;
            expressionsTemp.push_back(currExp);
		}
	}


	// check conditions for state change
    if ((expressionsTemp.size() > 0) && (!compareStates(expressionsTemp, expressionsOld))) {
        stateChanged = true;
    }

	// make a backup of the current state
    if (expressionsTemp.size() > 0) {
        expressionsOld = expressionsTemp;
    }

    // for each faceID in facesIDs
    for (int facesIDsIdx = 0; facesIDsIdx < facesIDs.size(); facesIDsIdx++) {
        expression currExp;
        currExp.expFacesID = facesIDs.at(facesIDsIdx);
        currExp.expr = -1;

        // for each faceID in expressionsTemp
        for (int idx = 0; idx < expressionsTemp.size(); idx++) {
            if (facesIDs.at(facesIDsIdx) == expressionsTemp.at(idx).expFacesID) {
                currExp.expr = expressionsTemp.at(idx).expr;
                break;
            }
        }

        expressions.push_back(currExp);
    }

	// increment localFrameID
	((algctx_t*)pctx)->localFrameID++;
	return stateChanged;
}

bool cTUSExprRec::compareStates(vector<expression> expressions, vector<expression> expressionsOld) {
    if (expressions.size() == 0) {
            return false;
    }

    if (expressionsOld.size() == 0) {
            return false;
    }

    if (expressions.size() != expressionsOld.size()) {
            return false;
    }

    for (int i = 0; i < expressions.size(); i++) {
        if (expressions.at(i).expFacesID != expressionsOld.at(i).expFacesID) {
            return false;
        }

        if (expressions.at(i).expr != expressionsOld.at(i).expr) {
            return false;
        }

    }

   return true;

}

/**
*/
Expression_t cTUSExprRec::estExpr(Mat imageFace)
{
	// resize facial image to a fixed size
	resize(imageFace, resizedImage, Size2i(((algctx_t*)pctx)->kernelWidth, ((algctx_t*)pctx)->kernelHeight), 0, 0, INTER_CUBIC);
	
	//normalize(resizedImage, resizedImage, 0, 255, CV_MINMAX);

	//namedWindow( "Display window No Norm", WINDOW_AUTOSIZE ); // Create a window for display.
	Mat imi;
	resizedImage.convertTo(imi, CV_8UC1);
    //imshow( "Display window No Norm", imi );                // Show our image inside it.
    //waitKey(10); // Wait for a keystroke in the window

	// light compensation

	Ptr<CLAHE> clahe = createCLAHE();
	clahe->setClipLimit(1);
	clahe->setTilesGridSize(Size(4,4));
	clahe->apply(resizedImage,resizedImage);
    resizedImage = singleScRet(resizedImage);

	//namedWindow( "Display window", WINDOW_AUTOSIZE ); // Create a window for display.
	resizedImage.convertTo(imi, CV_8UC1);
    //imshow( "Display window", imi );                // Show our image inside it.
    //waitKey(10); // Wait for a keystroke in the window

	// convert image data type
	resizedImage.convertTo(resizedImage, CV_32FC1);

	Mat reducedFeature = Mat(1, indexes.rows, CV_32FC1);

	Mat fftImage;

	Mat planes[] = {Mat_<float>(resizedImage), Mat::zeros(resizedImage.size(), CV_32F)};
    merge(planes, 2, fftImage);
	dft(fftImage, fftImage, DFT_COMPLEX_OUTPUT);

	//dft(resizedImage, fftImage, DFT_COMPLEX_OUTPUT);
	
	Mat fftImageFiltered = Mat(((algctx_t*)pctx)->kernelHeight, ((algctx_t*)pctx)->kernelWidth, CV_32FC2);

	int id = 0;

	// for each kernel
	for (int kernelNum = 0; kernelNum < ((algctx_t*)pctx)->numOfKernelsInt; kernelNum++) {

		// get filter kernel
		Mat curKernelp = Mat(kernels, Rect(0,kernelNum*((algctx_t*)pctx)->kernelHeight, ((algctx_t*)pctx)->kernelWidth, ((algctx_t*)pctx)->kernelHeight));
		Mat curKernel = curKernelp.clone();
		Mat planes[] = {Mat_<float>(curKernel), Mat::zeros(curKernel.size(), CV_32F)};
		Mat complexKernel;
		merge(planes, 2, complexKernel); 

		// elementwise multiplication fftImage by complexKernel -> fftImageFiltered
		mulSpectrums(fftImage, complexKernel, fftImageFiltered, DFT_COMPLEX_OUTPUT);

		// perform inverse fft and get magnitude -> ifftMagnitude
		dft(fftImageFiltered, fftImageFiltered, DFT_INVERSE|DFT_COMPLEX_OUTPUT|DFT_SCALE);
        split(fftImageFiltered, planes);
		magnitude(planes[0], planes[1], planes[0]);
		Mat ifftMagnitude = planes[0];

		while(kernelNum + 1 == (int)indexes.at<float>(id,0)) {
			reducedFeature.at<float>(0,id) = ifftMagnitude.at<float>((int)indexes.at<float>(id,1), (int)indexes.at<float>(id,2));
			id += 1;
			if (id == indexes.rows) {
				break;
			}
		}
	}
	// scale reduced future component in the range [-1 1]
	Mat scaledFeature = scaleFeature(reducedFeature, KSVMScaleRange);

	// inside a loop copy the components of the scaledFeature to x.
	for(int k = 0; k < reducedFeature.cols; k++){
		((algctx_t*)pctx)->x[k].index = k+1;
		((algctx_t*)pctx)->x[k].value = scaledFeature.at<float>(0,k);
	}
	((algctx_t*)pctx)->x[reducedFeature.cols].index = -1;

	// predict labels using SVMModel and x - the SVM node
	int expression = (int)svm_predict(((algctx_t*)pctx)->SVMModel, ((algctx_t*)pctx)->x);

	return (Expression_t) expression;
}

cv::Mat cTUSExprRec::singleScRet(Mat inputImage) {

	Mat normalizedImage;

	// normalize in range 0 - 255
	normalize(inputImage, normalizedImage, 0, 255, CV_MINMAX);

	// convert normalized image to float
	normalizedImage.convertTo(normalizedImage, CV_32FC1);

	// add 0.01 for the log operation
	normalizedImage += Scalar(0.01);

	// take image dimension
	int rows = inputImage.rows;
	int cols = inputImage.cols;

	// local variables for the loop operations
	int i, j;

	// local pointer to each row of pixels for the loop operations
	float* p;

	// Filter image and adjust for log operation

	// allocate memory for filtered image (the same syze and type as normalized iamge) 
	Mat filtImage = Mat(normalizedImage.size(), normalizedImage.type());

	// filter image using prepared filter
	filter2D(normalizedImage, filtImage, CV_32F, *((algctx_t*)pctx)->filt, Point(-1,-1), 0.0, BORDER_REPLICATE);

	// check if there is zerro intensity value and add 0.01 for the log operation. Ceil pixel values as well.
	for ( i = 0; i < rows; i++) {
		p = filtImage.ptr<float>(i); 
		for ( j = 0; j < cols; j++) {
			if (p[j] == 0.0) {
				p[j] = 0.01;
		    } else {
				p[j] = ceil(p[j]);
		    }
		}
	}

	// Natural logarithm operations and subtraction

	// take logarithm of normalizedImage image
	log(normalizedImage, normalizedImage);

	// take logarithm of filtered image
	log(filtImage, filtImage);

	// subtract logarithm versions of normalizedImage and filtImage to get luminance invariant image -> normalizedImage
	subtract(normalizedImage, filtImage, normalizedImage);

	// Histogram truncation

	// normalize in range 0 - 1
	normalize(normalizedImage, normalizedImage, 0, 1, CV_MINMAX);

	// get image as one row array
	Mat reshapedImage = normalizedImage.reshape(1, 1);

	// allocate and initialize array that will store sorted image values
	Mat sortim = Mat::zeros(1, reshapedImage.cols, CV_32F);

	// sort image vaules array
	cv::sort(reshapedImage, sortim, CV_SORT_ASCENDING);

	// create expanded sorted array (begin with zero and terminate with 1)
	Mat sortv = Mat::zeros(1, 1, CV_32F);
	cv::hconcat(sortv, sortim, sortv);
	cv::hconcat(sortv, Mat::ones(1, 1, CV_32F), sortv);

	// interpolate to find grey values at desired percentage levels
	float lowThresh = interp1(*((algctx_t*)pctx)->perc, sortv, 0.2);
	float highThresh = interp1(*((algctx_t*)pctx)->perc, sortv, 99.8); 

	// adjust image acording to a given intesity range
	normalizedImage = imageAdjust(normalizedImage, lowThresh, highThresh, 0.0, 1.0);

	// normalization in range 0 - 255
	normalize(normalizedImage, normalizedImage, 0, 255, CV_MINMAX);

	return normalizedImage;
}

cv::Mat cTUSExprRec::imageAdjust(Mat src, float low_in, float high_in, float low_out, float high_out) {

	    Mat dst(src.rows, src.cols, CV_32FC1);
		float err_in = high_in - low_in;
		float err_out = high_out - low_out;
		int j, i;
		float val;
		float* p;

		for ( i = 0; i < src.rows; i++) {
			p = src.ptr<float>(i); 
			for ( j = 0; j < src.cols; j++) {
				val = ((p[j] - low_in)/err_in)*err_out + low_out;
				if (val > 1 ) {
					val = 1;
				}
				if (val < 0 ) {
					val = 0;
				}
				dst.at<float>(i,j) = (float) val;
			}
		}

		return dst;
}


float cTUSExprRec::interp1(Mat X, Mat Y, float targetX)
{
  Mat dist = abs(X-targetX);
  double minVal, maxVal;
  Point minLoc1, minLoc2, maxLoc;

  // find the nearest neighbour
  Mat mask = Mat::ones(X.rows, X.cols, CV_8UC1);
  minMaxLoc(dist,&minVal, &maxVal, &minLoc1, &maxLoc, mask);

  // mask out the nearest neighbour and search for the second nearest neighbour
  mask.at<uchar>(minLoc1) = 0;
  minMaxLoc(dist,&minVal, &maxVal, &minLoc2, &maxLoc, mask);

  // use the two nearest neighbours to interpolate the target value
  float diffX = X.at<float>(minLoc2) - X.at<float>(minLoc1);
  float diffY = Y.at<float>(minLoc2) - Y.at<float>(minLoc1);
  float diffTarget = targetX - X.at<float>(minLoc1);
  return (Y.at<float>(minLoc1) + (diffTarget * diffY) / diffX);
}

/**
Return representative expression index (0 to exprSize). If the number of most repeated expression devided by expBuffLen is less than
than the confRatio, a value of -1 is returned.
*/
int cTUSExprRec::getReprExpr(int* exprBuffer, int expBuffLen, int exprSize, float confRatio)
{
    int i;

    // initialize occurrences array
    for (i = 0; i < exprSize; i++) {
        ((algctx_t*)pctx)->histoOcc[i] = 0;
    }

    // find the number of occurences for each expression
    for (i = 0; i < expBuffLen; i++) {
        if (exprBuffer[i] != -1) {
            ((algctx_t*)pctx)->histoOcc[exprBuffer[i]] += 1;
        }
    }

    // find the most occurring expression
    int maxOcc = ((algctx_t*)pctx)->histoOcc[0], mostOccExpr = 0;

    for (i = 1; i < exprSize; i++)
    {
        if (((algctx_t*)pctx)->histoOcc[i] > maxOcc)
        {
            maxOcc = ((algctx_t*)pctx)->histoOcc[i];
            mostOccExpr = i;
        }
    }

    if ((float)maxOcc/expBuffLen < confRatio) {
        mostOccExpr = -1;
    }

    return mostOccExpr;
}
