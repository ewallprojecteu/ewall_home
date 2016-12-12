#include <iostream>
#include "../FaceProcessing/src/Communication.h"
#include "SimpleProcessing.h"
#include "time_funcs.h"

// ER algorithm includes
#include "cTUSExprRec.hpp"

//Define NO_DEVICEGATEWAY to read video from stream without using Device Gateway 
//#define NO_DEVICEGATEWAY

#ifndef NO_DEVICEGATEWAY
#include "libDGwClientAV.h"
#include "DGWdefs.h"
#else //NO_DEVICEGATEWAY
#include <opencv2/highgui/highgui.hpp>
#endif //NO_DEVICEGATEWAY

#ifdef _WIN32
#include <Windows.h> //For IsDebuggerPresent()
#endif //

using namespace std;

// ER algorithm variables
// Storage file names for ExpRec algorithm
const string KernelsData("./models/LGADASVM/kernels_features.yml");
const string KSVMScaleData("./models/LGADASVM/KSVM_scale_range.yml");
const string SVMModelData("./models/LGADASVM/SVMModel.model");

// Storage file name for object detector
const string faceDetectionModel("./models/haarcascade_frontalface_alt2.xml");

// available expressions
string expressArr[] = {"neutral", "anger", "disgust", "fear", "happy", "sad", "surprise"};

////////////////////////


int main(int argc, char *argv[])
{
	bool      active;
	double    luminance;
    cv::Mat   rgbFrame;
    cv::Mat   grScFrame;
	long long frameCounter = -1;

	string serverName = "http://localhost:5984/visual_environment/";

	vector <string> modelFiles;

	// Init LGADASVM algorithm
	modelFiles.push_back(KernelsData);
	modelFiles.push_back(KSVMScaleData);
	modelFiles.push_back(SVMModelData);
    cTUSExprRec *ExprRec = new cTUSExprRec(modelFiles, 10, 15, 0.6);

	// load OpenCV face detector model
	CascadeClassifier faceCascade;
	if(!faceCascade.load(faceDetectionModel)){ 
		cerr << "Error loading face detection model." << endl;
		exit(EXIT_FAILURE); 
	}
	//////////////////////

 // target structure of objects to be written in the Database
 vector<targetStruct> targets;

//Initialize based on input type
#ifndef NO_DEVICEGATEWAY
cout<<"DEVICEGATEWAY" << endl;
	if (argc>2)
	{
		cout<<"Usage: " << argv[0] << " \"server name\"" << endl;
		return -1;
	}
	if (argc==2)
		serverName = argv[1];
	DGwClientAV AVProcess;
	AVProcess.initializeDGwClientAV();
#else //NO_DEVICEGATEWAY
//	if (argc<2)
//	{
//		cout<<"Usage: " << argv[0] << " \"video name\"" << " (\"server name\")" << endl;
//		return -1;
//	}
//	cv::VideoCapture cap(argv[1]);
	cv::VideoCapture cap(0);
cout<<"NO_DEVICEGATEWAY" << endl;
	if (!cap.isOpened())
	{
		cout<<"Video " << argv[1] << " could not be opened." << endl;
		return -1;
	}
#endif //NO_DEVICEGATEWAY

	cout << "Initializing couchdb server: " << serverName << endl;
	Communication couchdb(serverName);
	SimpleProcessing proc;

#ifndef NO_DEVICEGATEWAY
    while (AVProcess.readFrame(rgbFrame))
#else
    while (cap.read(rgbFrame))
#endif
	{
		frameCounter++;

        // get the the rgb frame (i.e. the rgbFrame)
        proc.processFrame(rgbFrame, &active, &luminance);

        // convert the current frame (rgbFrame) from RGB to Grayscale (grScFrame)
        cvtColor(rgbFrame, grScFrame, COLOR_BGR2GRAY);

        // define vector as sequences of faces IDs (i.e. Identficator number for each face found)
        vector<int> facesIDs;

        // define vector as sequences of rectangles (i.e. faces bounding boxes)
        vector<Rect> facesRects;

        // define vector as a sequences of faces IDs and their corresponding expreesions (to be returned from the ExpRec algorithm)
        vector<expression> expressions;

        /** Implement detector of multiple faces. Assign IDs and apply facial expression algorithm
        TODO: To be replaced with the Facial tracker implementation */

        // face detector setup parameters must be the same as specified here!
        faceCascade.detectMultiScale(grScFrame, facesRects, 1.2, 7, 1, Size(60, 60));

        // if face found (i.e. facesRects is not empty)
        if (!facesRects.empty()) {
            // assign an ID for each face found -> to be replaced with tracker ID
            for (int idx = 0; idx < facesRects.size(); idx++) {
                facesIDs.push_back(idx+1);
                // draw face rectangle (to be removed later...)
                rectangle(rgbFrame, facesRects.at(idx), Scalar(0, 0, 255), 2);
            }

        }

        // apply expression recognition alogirthm. Return true if there is a change between
        // the last and the current results from ExpRec algorithm and
        // an expression for at least one face can be estimated
        bool stateChanged = ExprRec->Apply(expressions, facesIDs, facesRects,  grScFrame);

        if (stateChanged) {
            cout <<"State changed... " <<endl;
            targetStruct target;
            for (int idx = 0; idx < expressions.size(); idx++) {
                int expId = expressions.at(idx).expr;
                if (expId != -1) {
                    cout <<"Face ID: " << expressions.at(idx).expFacesID << " Expression: " << expressArr[expId]<<endl;
                    target.emotion = expressArr[expId];
                } else {
                    cout <<"Face ID: " << expressions.at(idx).expFacesID << " Expression: N/A" << endl;
                    target.emotion = "N/A";
                }
                target.target_id = expressions.at(idx).expFacesID;
                Rect curFaceRec = facesRects.at(expressions.at(idx).expFacesID-1);
                target.face = RotatedRect(Point2f(curFaceRec.x + curFaceRec.width / 2, curFaceRec.y + curFaceRec.height / 2), Size(curFaceRec.width,curFaceRec.height), 0);
                target.spread = 0;
                target.emotionConf = 0.6;
                target.gender = 0.8;
                target.ageEst = 30;
                target.ageEstConf = 0.8;
                targets.push_back(target);
            }

            couchdb.sendMessage(getMillis(), targets);
            targets.clear();
        }

        for (int idx = 0; idx < expressions.size(); idx++) {
            stringstream label; //Local so that it gets cleared in each loop
            int expId = expressions.at(idx).expr;
            if (expId != -1) {
                label << "ID: " << expressions.at(idx).expFacesID << ": " << expressArr[expId];
            } else {
                label << "ID: " << expressions.at(idx).expFacesID << ": N/A";
            }
            cv::putText(rgbFrame, label.str(), Point(facesRects.at(expressions.at(idx).expFacesID-1).x+5, facesRects.at(expressions.at(idx).expFacesID-1).y+15), FONT_HERSHEY_PLAIN, 1, Scalar(0, 255, 0, 0), 1, CV_AA);
        }

        facesIDs.clear();
        facesRects.clear();

        stringstream frameSstrean; //Local so that it gets cleared in each loop
                frameSstrean << "Frame: " << frameCounter << ", active:" << (active?"true":"false") << ", luminance:" << luminance;

        // show the frame
        char ret=(char) proc.showFrame(rgbFrame,frameSstrean.str());
        if (ret == 'q' || ret == 'Q' || ret == 27 /*ESC*/) {
            break; //Stop the program
        }
			
	}



// Delete Algorithm
		delete ExprRec;

#ifdef _WIN32
	//Pause if run within Visual Studio
	if (IsDebuggerPresent())
	{
		cv::destroyAllWindows();
		cout << "Press Enter to exit ";
		getchar();
	}
#endif //WIN32
	cout << "Exiting..." << endl;
	return 0;
}


