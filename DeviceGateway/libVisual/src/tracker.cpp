#include "tracker.h"
#include "colorhist.hpp"
#include "VJ.h"
#include "PF.h"

#include <opencv2/video/tracking.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <math.h>
#include "assignmentoptimal.hpp"
#include "display.h"
#include "genderReco.hpp"
#include "time_funcs.h"
#include "curl_helper.h"

using namespace cv;
using namespace std;

	//stats structure
	typedef struct
	{
		double	facesPerFrame;
		int64	faceCounter;
		vector<int64> pastTargetsTimes; //Timestamps of most recent target deaths
		int64	genderCounter[2]; //we need both face and gender counters, as some persons may not be recognised
		vector<int> durationHist; //Histogram of track durations
		vector<int>	durationHistLimits; //Upper limit of each bin in durationHist
	}tgtStats;

tgtStats statistics;

#define MAXDIST2MERGE	1.7	// Maximum distance that two rects are allowed to be merged

//Limits for target kill
float minAspect=.3f, maxAspect=1.9f; //Aspect ratio
float minAreaRatio=0.1f,maxAreaRatio=50.0f; //Area ratio compared to most recent face detection

double LARGE_LIKELIHOOD_DROP=2e2;  //2e2 // Likelihood drop above which some particles are forced back to the state, if likelihood remains low, target is killed
double MIN_LIKELIHOOD_RATIO=1.75;  //Kill target if the likelihood drops below MIN_LIKELIHOOD_RATIO*(minimum possible likelihood)

extern double LARGE_SPREAD;  // Particle spread above which reinitialisation attempts begin (.5)


bool use1DHist=false; //Whether to use 1D or 3D histograms
bool postMessages=false; //Whether to send data about targets to a server

//This image is needed for 1D histogram backprojection and color likelihood calculation
Mat histimg;

//Tracker globals
Mat	IyCrCb_tracker; //Input image in YCrb colorspace
Mat	frg_tracker; //Full size foreground mask from main, required for backproj of camshift tracker

extern	double histUpdateRate; //face histogram learning rate from colorhist
extern int PAR_FOR_STRIPES; //parallel for stripes from PF
extern double SIGMA2_COLOR;

int TRACKER_MODALITIES = 0;

int faceWidths[2]={20,50}; //Face width limits for far/medium/near distance full hd
//int faceWidths[2]={28,45}; //Face width limits for far/medium/near distance 720p


void face2rects(Rect faceRect, Rect limit, Rect out[FACE_RECTS_SIZE], double expansion, double xConstrain, double yExpand)
{
	out[0] = Rect(cvRound(faceRect.x+xConstrain*faceRect.width), cvRound(faceRect.y-yExpand*faceRect.height), cvRound((1-2*xConstrain)*faceRect.width), cvRound((1+yExpand)*faceRect.height));
	out[0] &= limit; //Constrain it inside the limit
	int x=faceRect.x, y=out[0].y, w=cvRound(expansion*out[0].width), h=cvRound(expansion*out[0].height);
	out[1] = Rect(faceRect.x, y-h, faceRect.width, h) & limit;
	out[2] = Rect(x-w, y-h, w, out[0].height+h) & limit;
	out[3] = Rect(x+faceRect.width, y-h, w, out[0].height+h) & limit;
	out[4] = Rect(x-w, y-h, faceRect.width+2*w, out[0].height+h) & limit;
}

void face2rectsMotion(Rect faceRect, Rect limit, Rect out[FACE_RECTS_SIZE])
{
	double xConstrainMotion=0; //How much to constrain the face on each side of the x axis, default 0.05
	double yExpandMotion=0.35;	//How much to expand the face above the top on y axis, default 0.35
	double expansionMotion=0.5; //How much to expand the resulting rectangle of the face for background color estimation, default 0.5
	face2rects(faceRect,limit,out,expansionMotion,xConstrainMotion,yExpandMotion);
}

int sendMessage(const targetStruct &target, int64 frameTime)
{
	static int64 firstTimestamp=-1;
	//If running on a live camera, frameTime would be large
	//else add the current time to all timestamps
	if (firstTimestamp==-1)
		firstTimestamp=frameTime<1000000000000?getMillis():0;
	const char target_message[]=
		"{\n"
		"\t\"startDateTime\" : \"%s\",\n"
		"\t\"duration\" : %lld,\n"
		"\t\"gender\" : %d,\n"
		"\t\"genderConf\" : %d,\n"
		"\t\"age\" : %d,\n"
		"\t\"ageConf\" : %d%s"
		"}\n";
	string glancemessage;
	int numGlances = 0;//target.glance_times.size();
	if (numGlances==0)
		glancemessage = "\n";
	else
	{
		glancemessage = ",\n"
			"\t\"glances\" : [";
		for (int i=0;i<numGlances-1;i+=2)
		{

		}
		glancemessage += "\t]\n";
	}
	char buff[30];
	millis2string(firstTimestamp+target.created_time,buff,30);
	int gender,percent;
	decisions2gender(target.gender,gender,percent);
	string message= format(target_message,buff, frameTime-target.created_time, gender, percent, 
		target.ageEst, cvRound(100*target.ageEstConf), glancemessage.c_str());
	cout << message;
	if (postMessages)
		curl_send(message.c_str(),message.length());
	return 0;
}

void killTargets(vector<targetStruct> &targets, int64 frameTime)
{
	static double MIN_COLOR_LIKELIHOOD=exp(-0.5/SIGMA2_COLOR)*MIN_LIKELIHOOD_RATIO; // Color likelihood below which target is killed (6e-4)
	//

	for (int i=(int)targets.size()-1;i>=0;i--)
	{
		////If there was no face detected, try to perform a detailed face detection near the target window
		//if (!targets[i].detectedFace)
		//{
		//	Rect searchRect = targets[i].face.boundingRect();
		//	searchRect.x-=cvRound(searchRect.width*.1);
		//	searchRect.y-=cvRound(searchRect.height*.1);
		//faceRect=(faces+i)->rect;
		////Enlarge found rectangle by 10%
		//faceRect.x-=cvRound(faceRect.width*.1);
		//if(faceRect.x<0) faceRect.x=0;
		//faceRect.y-=cvRound(faceRect.height*.1);
		//if(faceRect.y<0) faceRect.y=0;
		//faceRect.width=cvRound(faceRect.width*1.2);
		//if(faceRect.x+faceRect.width>Igray.cols) faceRect.width=Igray.cols-faceRect.x;
		//faceRect.height=cvRound(faceRect.height*1.2);
		//if(faceRect.y+faceRect.height>Igray.rows) faceRect.height=Igray.rows-faceRect.y;

		//It=Igray(cv::Rect(faceRect)).clone();
		//cv::equalizeHist(It,It);

		//fine_cascade.detectMultiScale(It, detections, 1.05, 1, CASCADE_FIND_BIGGEST_OBJECT );

//		}
		int area=targets[i].trackWindow.area();
		float aspect = targets[i].trackWindow.width/(float)targets[i].trackWindow.height;
		float areaRatio = targets[i].trackWindow.area()/(float)targets[i].latestDetection.area();
		int noGlanceLimit=20000;//std::min(300,25*((int)targets[i].genderDecisions.size()+2));

		//Kill targets
		bool targetKill=false;
		string killReason;
		//Kill missed targets
		if(area <= 1 )
		{
			targetKill=true;
			killReason="Area = " + format("%d [2,inf)",area);
		}
		//Or if a target is too wide or too narrow
		else if (aspect >maxAspect || aspect < minAspect)
		{
			targetKill=true;
			killReason="Aspect = " + format("%.2f [%.2f,%.2f]",aspect,minAspect,maxAspect);
		}
//		//Or if much different area than latest face detection
		else if (areaRatio > maxAreaRatio || areaRatio < minAreaRatio)
		{
			targetKill=true;
			killReason="Area Ratio = " + format("%.2f [%.2f,%.2f]",areaRatio,minAreaRatio,maxAreaRatio);
		}
#ifdef USE_PF_TRACKER
		else if (targets[i].Linit/targets[i].Lmax>LARGE_LIKELIHOOD_DROP)
		{
			targetKill=true;
			killReason="LikelihoodDrop " + format("%.1f [0,%g]",  targets[i].Linit/targets[i].Lmax, LARGE_LIKELIHOOD_DROP);
		}
		else if ((TRACKER_MODALITIES & CUE_COLOR)!=0 && targets[i].LcompMax[1]<MIN_COLOR_LIKELIHOOD)
		{
			targetKill=true;
			killReason="Color Likelihood " + format("%g < %g",  targets[i].LcompMax[1], MIN_COLOR_LIKELIHOOD);
		}
		else if (targets[i].spread>LARGE_SPREAD)
		{
			targetKill=true;
			killReason="Spread " + format("%.2f [0,%.2f]",  targets[i].spread, LARGE_SPREAD);
		}
#endif /*USE_PF_TRACKER*/
		//Or if no glance for a long time
		else if (frameTime-targets[i].created_time > 3000)
		{
			int glanceIdx=targets[i].glance_times.size()-1;
			int64 glanceTime;
			if (glanceIdx<0)
				glanceTime=targets[i].created_time;
			else
				glanceTime=targets[i].facing_time;

			if (!targets[i].isGlancing && frameTime-glanceTime>noGlanceLimit)
			{
				targetKill=true;
				killReason="No glance found for " + format("%5.1f sec [0,%5.1f]",(frameTime-glanceTime)/1000.0, noGlanceLimit/1000.0);
			}
		}

		if (targetKill)
		{
			int64 durationHist=frameTime-targets[i].created_time;
			cout << "At "<< frameTime <<" ms: killing target " << targets[i].target_id << ", duration: " << durationHist << ", reason: " << killReason << endl;
#ifndef USE_PF_TRACKER
			cv::destroyWindow(format("backproj %3d",targets[i].target_id));
#endif /*USE_PF_TRACKER*/

			//Update target killed statistics
			int j;
			for (j=0; j<(int)statistics.durationHistLimits.size(); j++)
			{
				if (durationHist<statistics.durationHistLimits[j])
					break;
			}
//			if (j==statistics.durationHistLimits.size())
//				j--;
			statistics.durationHist[j]++;
			statistics.pastTargetsTimes.push_back(frameTime);
			if (statistics.pastTargetsTimes.size()>9*statistics.pastTargetsTimes.capacity()/10)
			{
				statistics.pastTargetsTimes.erase(statistics.pastTargetsTimes.begin(),
					statistics.pastTargetsTimes.begin()+statistics.pastTargetsTimes.capacity()/2);
			}


			if (sum(targets[i].genderCounts)[0]>6)
				targets[i].gender<=0.5? statistics.genderCounter[0]++:statistics.genderCounter[1]++;
			sendMessage(targets[i], frameTime);
			targets.erase(targets.begin()+i);
		}
	}
}

double distOfRects(Rect r1, Rect r2)
{
	double d, c, w1, w2, h1, h2;

	w1=(double)r1.width;
	w2=(double)r2.width;
	h1=(double)r1.height;
	h2=(double)r2.height;
	c=sqrt(w1*w2*h1*h2);

	d=(pow((double)(r1.x-r2.x),2.0)+pow((double)(r1.y-r2.y),2.0)+pow((w1-w2),2.0)+pow((h1-h2),2.0))/c;

	return d;
}

void enlargeRect(Rect& rect, float factor)
{
	rect.x-=cvRound(rect.width*factor);
	rect.y-=cvRound(rect.height*factor);
	rect.width=cvRound(rect.width*(2*factor+1));
	rect.height=cvRound(rect.height*(2*factor+1));
}

//Returns sqrt(x^2+y^2)
inline double norm2(double x, double y)
{
	return sqrt(pow(x,2)+pow(y,2));
}

int updateCamShiftTracker(targetStruct& target)
{
	Mat backproj;
	static Rect imgRect=Rect(0,0,IyCrCb_tracker.cols,IyCrCb_tracker.rows);
	Rect targetRegion=target.trackWindow;
	enlargeRect(targetRegion,0.8f);
	targetRegion &= imgRect;

//	targetRect=tgt.boundingRect() & imgRect;

	if (targetRegion.width==0 || targetRegion.height==0)
		return -1;

	Rect trackWindow=Rect(target.trackWindow.x-targetRegion.x,target.trackWindow.y-targetRegion.y,
		target.trackWindow.width,target.trackWindow.height);

	if(use1DHist)
		backProject1D(histimg(targetRegion), target.colorModel, backproj);
	else
		backProject(IyCrCb_tracker(targetRegion),target.colorModel,backproj);
	bitwise_and(backproj,frg_tracker(targetRegion),backproj);
	imshow(format("backproj %3d",target.target_id),backproj);

	target.face = CamShift(backproj, trackWindow, TermCriteria(  CV_TERMCRIT_ITER | CV_TERMCRIT_EPS, 3, 2 ));
	target.face.center.x+=targetRegion.x;
	target.face.center.y+=targetRegion.y;
	target.trackWindow=Rect(trackWindow.x+targetRegion.x,trackWindow.y+targetRegion.y,trackWindow.width,trackWindow.height);
	return 0;
}

bool isTargetClose(const std::vector<targetStruct>& targets, int indx1, int indx2)
{
	const double overlap = 0.7;
	//const double sizeDifference = 0.1;

	//static double minSizeRatio = 1-sizeDifference;
	//static double maxSizeRatio = 1+sizeDifference;
	//
	Rect target1=targets[indx1].face.boundingRect(), target2=targets[indx2].face.boundingRect();

	int area1=target1.area(), area2=target2.area();
	assert(area1 > 0 && area1 >0);

	//if (target1.width/target2.width < minSizeRatio || target1.width/target2.width > maxSizeRatio)
	//	return false;
	//if (target1.height/target2.height < minSizeRatio || target1.height/target2.height > maxSizeRatio)
	//	return false;

	Rect intersect=target1&target2;

	if ((double)intersect.area()/std::min(area1,area2) > overlap)
		return true;
	else
		return false;
}


//Search the vector for the specified value, starting from end
//and moving towards the first element
//returns found index or -1
int pairFind(const vector<Point>& vec, Point val)
{
	int i=vec.size();
	if (i<=0)
		return -1;
	int found = -1;
	while (i--)
	{
		if (vec[i]==val)
		{
			found = i;
			break;
		}
	}
	return found;
}

int findTarget(const std::vector<targetStruct>& targets, long targetID)
{
	int i=targets.size();
	if (i<=0)
		return -1;
	int found=-1;
	while (i--)
	{
		if (targets[i].target_id== targetID)
		{
			found = i;
			break;
		}
	}
	return found;
}

void mergeTargetPair(std::vector<targetStruct>& targets, int indx1, int indx2)
{
	//Find which one to erase
	//For now delete the newest
	//int indxDelete;
	//if (targets[indx1].created_time>targets[indx2].created_time)
	//	indxDelete=indx1;
	//else
	//	indxDelete=indx2;

	//Delete the one with best likelihood match
	int indxDelete;
	if (targets[indx1].Lmax<targets[indx2].Lmax)
		indxDelete=indx1;
	else
		indxDelete=indx2;
	//reduce total number of targets. no need to reduce gender counter, as target's gender is only registered when target is killed
	statistics.faceCounter--;
	targets.erase(targets.begin()+indxDelete);
}

void mergeTargets(std::vector<targetStruct>& targets)
{
	static vector<Point> mergeCandidates;
	static vector<int> mergeCounter;

	const int frames2merge=15; //Merge after XX frames being close

	int merge1, merge2;


	//Check existing candidate pairs, remove pairs with deleted targets or not close any longer
	merge1=mergeCandidates.size()-1;
	while (merge1>=0)
	{
		int tgtindx1 = findTarget(targets, mergeCandidates[merge1].x);
		int tgtindx2 = findTarget(targets, mergeCandidates[merge1].y);
		//Remove pair in the following cases:
		if (tgtindx1<0 || tgtindx2<0 /*deleted targets*/
			|| !isTargetClose(targets,tgtindx1, tgtindx2) /*or not close anymore*/)
		{
			mergeCandidates.erase(mergeCandidates.begin()+merge1);
			mergeCounter.erase(mergeCounter.begin()+merge1);
		}
		else
		{
			// Both targets exist and are still close, increase mergeCounter, join if required
			if (++mergeCounter[merge1]>=frames2merge)
			{
				cout << "Merging targets " << targets[tgtindx1].target_id << " and " << targets[tgtindx2].target_id << endl;
				mergeTargetPair(targets,tgtindx1,tgtindx2);
				//Todo nkat: close window in Camshift tracker
				mergeCandidates.erase(mergeCandidates.begin()+merge1);
				mergeCounter.erase(mergeCounter.begin()+merge1);
			}
		}
		merge1--;
	}

	merge1=targets.size()-1;
	while(merge1>=1)
	{
		merge2=merge1-1;
		while(merge2>=0)
		{
			if(isTargetClose(targets, merge1, merge2))
			{
				Point pair;
				// If both targets are in an existing pair, continue
				
				// Pair always store the largest id to x
				if (targets[merge1].target_id>targets[merge2].target_id)
					pair= Point(targets[merge1].target_id,targets[merge2].target_id);
				else
					pair= Point(targets[merge2].target_id,targets[merge1].target_id);
				int idx = pairFind(mergeCandidates, pair);
				// If already existing, it has been updated, handle non existing			
				if (idx<0)
				{
					// Create new pair
					mergeCandidates.push_back(pair);
					mergeCounter.push_back(1);
				}
			}
			merge2--;
		}
		merge1--;
	}

}

//Update glancing time and facing duration for targets
void updateGlances(std::vector<targetStruct>& targets, int64 frameTime)
{
	for (int i=targets.size()-1;i>=0;i--)
	{
		targetStruct& curTarget=targets[i];
		int glanceLast=curTarget.glance_times.size()-1; //Index of last glance
		//Immediately start glance if target is frontal
		if (curTarget.isFrontal)
		{
			curTarget.facing_time=frameTime;
			//Start glance
			if (!curTarget.isGlancing)
			{
				curTarget.isGlancing=true;
				curTarget.glance_times.push_back(frameTime);
				curTarget.glance_times.push_back(frameTime);
				glanceLast+=2;
			}
		}
		//End glance if target was not frontal in the last 0.5 second
		else if (curTarget.facing_time>=0 && frameTime-curTarget.facing_time>500)
			curTarget.isGlancing=false;

		//if target is still glancing
		if (curTarget.isGlancing)
		{
			//update glancing time
			curTarget.glance_times[glanceLast]=frameTime;
			if (curTarget.trackWindow.width<faceWidths[0])
				curTarget.facing_dist=0;
			else if (curTarget.trackWindow.width>faceWidths[1])
				curTarget.facing_dist=2;
			else
				curTarget.facing_dist=1;
		}

		//Recalculate facing duration
		curTarget.facing_duration=0;
		for (int j=glanceLast;j>=1;j-=2)
			curTarget.facing_duration+=curTarget.glance_times[j]-curTarget.glance_times[j-1];
	}
}

//#define DEBUG_ASSIGNMENTS

int trackFaces(const cv::Mat& _YCrCb, const cv::Mat& _frg, const std::vector<cv::Rect>& newFaces, int64 frameTime, std::vector<targetStruct>& targets)
{
	const double statLearningRate=0.02;

	if(newFaces.size()==0 && targets.size()==0)
	{
		//update statistics
		statistics.facesPerFrame*=(1-statLearningRate);
		return 0;
	}

	//Additional variables for 1D histograms
	static Mat Y,Cr,Cb;
	static int ch[] = {0,0, 1,1, 2,2};

	if(IyCrCb_tracker.size!=_YCrCb.size)
	{
		cout << "size changed, resetting targets" << endl;
		targets.clear();
		//clearStatistics();
	}

	IyCrCb_tracker = _YCrCb;
	Size imgSize = IyCrCb_tracker.size();

	if (use1DHist && Y.size()!=imgSize)
	{
		Y.create(imgSize, CV_8UC1);
		Cr.create(imgSize, CV_8UC1);
		Cb.create(imgSize, CV_8UC1);
	}
	static cv::Mat out[]={Y, Cr, Cb};
	
	int facenum=newFaces.size();
//	Rect imgrect = Rect(0,0,IyCrCb_tracker.cols,IyCrCb_tracker.rows);
	frg_tracker = _frg;
//	Mat roi, dispframe;
	MatND face_hist, bkg_hist;
	int i,j;
	double targetSize;
	
	//Prepare image for 1D histogram
	if (use1DHist)
	{
		cv::mixChannels(&IyCrCb_tracker, 1, out, 3, ch, 3);
		imageForHist1D(Y,Cr,Cb,histimg);
	}

	static long targetcount=0; //ID of new targets

	int targetnum=(int)targets.size();

	double dist; //Distance between target and face detection

	//Assignments of current targets to face detections
	static vector<int> targetAssigns;
	//The reverse assignments of face detections to targets
	static vector<int> faceAssigns;

	//Reset target and face assignments
//	targetAssigns.assign(targetnum,-1); nkat: to verify before replacing
	targetAssigns.resize(targetnum);
	for (i=0;i<targetnum;i++)
		targetAssigns[i]=-1;
//	faceAssigns.assign(facenum,-1);  nkat: to verify before replacing
	faceAssigns.resize(facenum);
	for (i=0;i<facenum;i++)
		faceAssigns[i]=-1;

	//If facenum==0(no face detections), the tracker will just follow the previous targets
	//If targetnum==0(no targets), new targets will be created based on the faces

//	cout<<"target assign"<<endl;
	//In the special case of one target and one face detection, associate it only
	//if small distance, no need for assignment optimal
	if (facenum==1 && targetnum==1)
	{
		targetSize=norm2(targets[0].face.size.width,targets[0].face.size.height);
		dist=distOfRects(newFaces[0],targets[0].face.boundingRect());
		if (dist<MAXDIST2MERGE)
		{
			targetAssigns[0]=0;
			faceAssigns[0]=0;
		}
	}
	//Else assignment optimal faces with targets
	else if (facenum>0 && targetnum>0)
	{
#ifdef DEBUG_ASSIGNMENTS
		dispframe = _YCrCb.clone();
		i=targets.size();
		while (i--)
		{
			rectangle(dispframe,targets[i].face.boundingRect(),CV_RGB(255,0,0));
			writeText(dispframe,format("%d",i),targets[i].face.center,0.2,CV_RGB(255,0,0));
		}
		i=newFaces.size();
		while (i--)
		{
			rectangle(dispframe,newFaces[i], CV_RGB(0,0,255));
			writeText(dispframe,format("%d",i),Point(newFaces[i].x,newFaces[i].y),0.2,CV_RGB(0,0,255));
		}
#endif //DEBUG_ASSIGNMENTS
		//Transposed distance matrix, rows are the faces, columns are the targets
		Mat distances(facenum,targetnum,CV_64F);

		double cost;
		for (i=0;i<targetnum;i++)
		{
			targetSize=norm2(targets[i].face.size.width,targets[i].face.size.height);
			for (j=0;j<facenum;j++)
			{
				dist=distOfRects(newFaces[j],targets[i].face.boundingRect());
				if (dist<MAXDIST2MERGE)
					distances.at<double>(j,i)=dist;
				else
					distances.at<double>(j,i)=MAXABS_64F;
			}
		}
		//Assignment optimal uses transposed distance matrix so we give cols/rows transposed
		assignmentoptimal(&targetAssigns[0],&cost,distances.ptr<double>(0),targetnum,facenum);
		int selectedFace;
		//Update the reverse assignments of faces to targets
		for (i=0;i<targetnum;i++)
		{
			selectedFace = targetAssigns[i];
			if (selectedFace>=0)
			{
				faceAssigns[selectedFace]=i;
			}
		}
	}

	//Initialise/Update target model based on detections
	for (i=0; i<facenum; i++)
	{
		if (use1DHist)
			calcFaceHist1D(histimg, newFaces[i], face_hist);
		else
			calcFaceHist(IyCrCb_tracker,newFaces[i],face_hist);

		double maxval; //Used for normalize histograms
		minMaxIdx(face_hist,NULL,&maxval);
		//Multiply in order to scale to 0-255, as normalize doesn't work on 3dimensional matrixes
		face_hist*=255/maxval;

		//if the face was assigned to a previous target, update
		//detection and color model
		if (faceAssigns[i]>=0)
		{
			targetStruct *curTarget=&targets[faceAssigns[i]];
			//Retrain only if good colour matching, nkat: check correct conditions
			//if (curTarget->LcompMax[1]>0 && -log(curTarget->LcompMax[1])*2*SIGMA2_COLOR>0.5)
			{
				face_hist=(1-histUpdateRate)*curTarget->colorModel+histUpdateRate*face_hist;
			}
			//Re-normalize the histogram and scale it to 0-255
			normalize(face_hist, face_hist, 1, 0, NORM_L1);
			minMaxIdx(face_hist,NULL,&maxval);
			curTarget->colorModel=face_hist*255/maxval;
			curTarget->detected_time=frameTime;
			curTarget->trackWindow=curTarget->latestDetection=newFaces[i];
		}
		else
		{
			//else create new target
			targetStruct newFace;
			newFace.trackWindow=newFace.latestDetection=newFaces[i];
			newFace.colorModel=face_hist.clone();
			newFace.target_id=targetcount++;
			newFace.created_time=newFace.detected_time=frameTime;
			newFace.recognised_time=-10000000;
			newFace.facing_duration=0;
			newFace.facing_time=-10000000;
			newFace.isFrontal=newFace.isGlancing=false;
			newFace.genderConf.resize(NUM_GENDERS);
			newFace.genderCounts.resize(NUM_GENDERS);
			newFace.agesWeightedSum=newFace.agesConfSum=newFace.agesCount=0;
			newFace.ageEst=-1; newFace.ageEstConf=0;
#ifdef USE_PF_TRACKER
			//Create a new vector because initPFtracker requires one
			vector<faceDetection> targetface;
			faceDetection tmp;
 			tmp.rect=newFaces[i];
			tmp.eyeR=tmp.eyeL=cvPoint(-1,-1);
			tmp.L=-1;
			targetface.push_back(tmp);
			initPFtracker(targetface, &newFace);
#else
			newFace.Lmax=newFace.spread=-1;
#endif /*USE_PF_TRACKER*/
			statistics.faceCounter++;
			targets.push_back(newFace);
		}
	}

	if (targets.size()>1)
		mergeTargets(targets);


//	cout<<"target track"<<endl;
	//Track all current targets
	if (targets.size()>0)
	{
#ifdef USE_PF_TRACKER
		updatePFtracker(targets, frameTime);
#else
		for (i=(int)targets.size()-1;i>=0;i--)
			updateCamShiftTracker(targets[i]);
#endif /*USE_PF_TRACKER*/
		killTargets(targets, frameTime);
	}

//	cout<<"target glance"<<endl;
	updateGlances(targets, frameTime);

	//update statistics
	statistics.facesPerFrame=(1-statLearningRate)*statistics.facesPerFrame+statLearningRate*targets.size();

	return 0;
}

void mergeFaces(faceDetection *faces, int *numFaces)
{
	CvMat	*D=cvCreateMat(*numFaces,*numFaces,CV_64FC1);
	double	dmin=0, d;
	CvPoint	minLoc;
	int		numMerged=0, mergedOutIdx[100];

	// Initialise distance matrix
	cvSet(D,cvScalar(MAXDIST2MERGE));
	for(int row=0;row<*numFaces-1;row++)
		for(int col=row+1;col<*numFaces;col++)
		{
			d=distOfRects((faces+row)->rect,(faces+col)->rect);
			if(d>MAXDIST2MERGE)
				d=MAXDIST2MERGE;
			cvmSet(D,row,col,d);
		}
		// While more merging is possible
		while(dmin<MAXDIST2MERGE && *numFaces-numMerged>1)
		{
			// If minimum distance is less than MAXDIST2MERGE and there is more than one face, merge
			cvMinMaxLoc(D,&dmin,NULL,&minLoc,NULL);
			if(dmin<MAXDIST2MERGE)
			{
				// Averaged face after merging
				(faces+minLoc.y)->rect.x=((faces+minLoc.y)->rect.x+(faces+minLoc.x)->rect.x)/2;
				(faces+minLoc.y)->rect.y=((faces+minLoc.y)->rect.y+(faces+minLoc.x)->rect.y)/2;
				(faces+minLoc.y)->rect.width=((faces+minLoc.y)->rect.width+(faces+minLoc.x)->rect.width)/2;
				(faces+minLoc.y)->rect.height=((faces+minLoc.y)->rect.height+(faces+minLoc.x)->rect.height)/2;
				// Face not to be used
				mergedOutIdx[numMerged++]=minLoc.x;
				// Updatethe distances in the matrix
				cvmSet(D,minLoc.y,minLoc.x,MAXDIST2MERGE);
				for(int col=minLoc.y+1;col<*numFaces;col++)
					if(col!=minLoc.x && cvmGet(D,minLoc.y,col)<MAXDIST2MERGE)
					{
						d=distOfRects((faces+minLoc.y)->rect,(faces+col)->rect);
						if(d>MAXDIST2MERGE)
							d=MAXDIST2MERGE;
						cvmSet(D,minLoc.y,col,d);
					}
					for(int row=0;row<minLoc.x;row++)
						cvmSet(D,row,minLoc.x,MAXDIST2MERGE);
			}
		}
		// If some rectangle were merged, remove their pairs from the list
		if(numMerged>0)
		{
			faceDetection newFaces[100];
			int counter=0;
			for(int i=0;i<*numFaces;i++)
			{
				int notCopy=0;
				for(int j=0;j<numMerged;j++)
					if(i==mergedOutIdx[j])
					{
						notCopy=1;
						break;
					}
					if(notCopy==0)
					{
						newFaces[counter].L=faces[i].L;
						newFaces[counter].rect.height=faces[i].rect.height;
						newFaces[counter].rect.width=faces[i].rect.width;
						newFaces[counter].rect.x=faces[i].rect.x;
						newFaces[counter++].rect.y=faces[i].rect.y;
					}
			}
			for(int i=0;i<counter;i++)
			{
				faces[i].L=newFaces[i].L;
				faces[i].rect.height=newFaces[i].rect.height;
				faces[i].rect.width=newFaces[i].rect.width;
				faces[i].rect.x=newFaces[i].rect.x;
				faces[i].rect.y=newFaces[i].rect.y;
			}
			*numFaces=counter;
		}

		cvReleaseMat(&D);
}

vector<faceDetection> mergeFaces2(const vector<faceDetection>& faces)
{
	int minNeighbors=3;
	int i,j;
	int mergedNum, numFaces;
	double maxL;
	numFaces=faces.size();
	vector<Rect> rectList;
	vector<faceDetection> mergedFaces;
	rectList.resize(numFaces);
	for (i=0;i<numFaces;i++)
		rectList[i]=faces[i].rect;
	if (numFaces>2)
		numFaces;//to be able to stop in debugging
	groupRectangles(rectList, std::max(minNeighbors, 1), 0.2);
	mergedNum=rectList.size();
	//Keep the maximum likelihood of the grouped rectangles
	mergedFaces.resize(mergedNum);
	for (i=0;i<mergedNum;i++)
	{
		maxL=0;
		for (j=0;j<numFaces;j++)
		{
			double d = distOfRects(rectList[i],faces[j].rect);
			if (d<MAXDIST2MERGE && faces[j].L>maxL)
				mergedFaces[i].L=maxL=faces[j].L;
		}
	}

	numFaces=rectList.size();
	for (i=0;i<numFaces;i++)
	{
		mergedFaces[i].rect=rectList[i];
	}
	return mergedFaces;
}

//inline double averageInSumRect(cv::InputArray _Isum, cv::Rect r)
//{
//	Mat			Isum = _Isum.getMat();
//	int			d;
//	int			x1=cvRound(r.x/mydec), y1=cvRound(r.y/mydec), x2=cvRound((r.x+r.width)/mydec)-1, y2=cvRound((r.y+r.height)/mydec)-1;
//
//	d=Isum.at<int>(y2,x2);
//	d+=Isum.at<int>(y1,x1);
//	d-=Isum.at<int>(y2,x1);
//	d-=Isum.at<int>(y1,x2);
//	return (double)d/((x2-x1+1)*(y2-y1+1));
//}

void clearStatistics()
{
	//clear statistics
	statistics.facesPerFrame=0;
	statistics.faceCounter=0;
	statistics.genderCounter[0]=statistics.genderCounter[1]=0;
	statistics.durationHist.assign(statistics.durationHist.size(),0);
	statistics.pastTargetsTimes.resize(0);
}

void initStatistics(int firstDuration=15*1000, int durationBins=5, int pastTargetSize=5, int pastTargetBinTime=1000)
{
	//init statistics
	statistics.durationHist.resize(durationBins,0); //5 bins
	statistics.durationHistLimits.resize(statistics.durationHist.size()-1);
	statistics.durationHistLimits[0]=firstDuration; //up to 15 seconds will be in 1st bin
	for (unsigned int i=1; i<statistics.durationHist.size()-1; i++)
		statistics.durationHistLimits[i]=statistics.durationHistLimits[i-1]<<1;
	statistics.pastTargetsTimes.reserve(500);
	clearStatistics();
}

int initFaceTracker(int trackerType, string& trackerName, int modalities, const string& cascadeName, const string& serverName)
{
	if (trackerType!=TRACKER_CAMSHIFT && trackerType!=TRACKER_PF)
		return -1;
	if (trackerType==TRACKER_PF && createParticleTracker(modalities, cascadeName.c_str())<0)
		return -1;
	initStatistics();
	TRACKER_MODALITIES = modalities;
#ifdef USE_PF_TRACKER
	cout << "Using PF tracker (" << NP << " particles) with the following modalities: ";
	trackerName="PF-";
	if ((TRACKER_MODALITIES & CUE_FACE)!=0)
	{
		cout << "FACE,";
		trackerName+="F";
	}
	if ((TRACKER_MODALITIES & CUE_COLOR)!=0)
	{
		cout << "COLOR,";
		trackerName+="C";
	}
	if ((TRACKER_MODALITIES & CUE_MOTION)!=0)
	{
		cout << "MOTION,";
		trackerName+="M";
	}
	cout << "\b, PARALLEL_FOR_STRIPES=" << PAR_FOR_STRIPES << endl;
#else
	cout << "Using CAM-Shift tracker\n";
	trackerName="CS";
#endif
	if (serverName.compare("localhost")!=0)
	{
		if (curl_init(serverName.c_str())!=0)
			cout << "Warning, CURL could not be initialized, data for targets will NOT be sent to a server";
		else
		{
			postMessages=true;
			cout << "Sending data for targets to: " << serverName << endl;
		}
	}
	return 0;
}

void stopFaceTracker()
{
	if (postMessages)
		curl_cleanup();
}

void drawStatistics(Mat &Irgb, int64 frameTime, cv::Rect statRect)
{
	const Scalar ORANGE = Scalar(0,63,255);
	const Scalar PINK   = Scalar(63,0,255);
	const Scalar CYAN   = Scalar(255,63,0);
	static double fontScale = statRect.height/232.0;
	int i;
	int recentSize=6; //number of bins for recent target deaths
	int recentTime=30000; //time between target death bins

    Mat tmp = Mat();

    static Size textSize = writeText(tmp,"100.0%",Point(),fontScale);
	//keep statRect inside the image
	statRect&=Rect(0,0,Irgb.cols,Irgb.rows);
	//graphs have landscape orientation, make sure that height<=width/2
	if (statRect.height>statRect.width/2)
		statRect.height=statRect.width/2;
	
	Point position;
	int delta = statRect.height/4;

	bool drawElipse=true;
	if (drawElipse)
	{
		//Ellipse almost fills rectangle height, therefore has axes slightly smaller
		Size ellipseAxes=Size(cvRound(statRect.height*.46),cvRound(statRect.height*.46));
		//Start ellipse leaving enough space on left for text
		Point ellipseCenter=Point(statRect.x+std::max(textSize.width,ellipseAxes.width)+delta,statRect.y+statRect.height/2);
		int ypos = statRect.y+textSize.height+statRect.height/16;

		int64 sum=statistics.genderCounter[0]+statistics.genderCounter[1];
		if (sum>0)
		{
			double men_p= (double)statistics.genderCounter[0]*100/sum;
			if (men_p>0)
				ellipse(Irgb,ellipseCenter,ellipseAxes,0,-90,-90+men_p*3.6,CYAN,-1,CV_AA);
			if (men_p<100)
				ellipse(Irgb,ellipseCenter,ellipseAxes,0,-90+men_p*3.6,270,PINK,-1,CV_AA);
			writeText(Irgb, format("%5.1f%%M", men_p), Point(ellipseCenter.x+delta,ypos), fontScale, CYAN);
			writeText(Irgb, format("%5.1f%%F", 100-men_p), Point(ellipseCenter.x-textSize.width-delta,ypos), fontScale, PINK);
		}
		else
		{
            static Size gendersize = writeText(tmp,"Gender N/A",Point(),fontScale);
			ellipse(Irgb,ellipseCenter,ellipseAxes,0,0,360,ORANGE,-1,CV_AA);
			writeText(Irgb, string("Gender N/A"), Point(ellipseCenter.x-gendersize.width/2,ypos), fontScale, ORANGE);
		}

		position=Point(statRect.x+ 2*(std::max(textSize.width,ellipseAxes.width)+delta) ,ellipseCenter.y-3*textSize.height/2);
	}
	else
		position=Point(statRect.x ,statRect.y);


	Rect drawr=Rect(position.x, statRect.y, (statRect.width-position.x)/2, statRect.height);
	vector<string> labels;
	int statlen=statistics.durationHist.size();
	labels.resize(statlen+1);
	labels[0]=string("0s");
	for (i=1;i<statlen;i++)
		labels[i]=format("%.0fs", statistics.durationHistLimits[i-1]/1000.0f);
	labels[statlen]=string("inf");

	Size plotsz=drawPlot(Irgb,drawr,statistics.durationHist,"%d",
		"Histogram of track durations", labels,0,ORANGE,CYAN);

	drawr.x+=plotsz.width+textSize.width;
	drawr.width=statRect.width-drawr.x;
	static vector<string> pastTgtLabels(1);
	if (pastTgtLabels.size()==1)
	{
		pastTgtLabels.resize(recentSize+1);
		pastTgtLabels[recentSize]=string("0s");
		for (i=recentSize-1;i>=0;i--)
			pastTgtLabels[i]=format("%.1fs", (i-recentSize)*recentTime/1000.0f);
	}
	static vector<int> pastTgtData(recentSize);
	pastTgtData.assign(recentSize,0);
	//If the last target death is recent enough
	if ((statlen=statistics.pastTargetsTimes.size())>0 && 
		frameTime-statistics.pastTargetsTimes[statlen-1] <= recentSize*recentTime)
	{
		for (i=statlen-1;i>=0;i--)
		{
			int bin = (int) (frameTime-statistics.pastTargetsTimes[i])/recentTime;
			if (bin>=recentSize)
				break;
			else
				pastTgtData[recentSize-bin-1]++;
		}
	}

	drawPlot(Irgb,drawr,pastTgtData,"%d",
		"Past targets", pastTgtLabels,1,CYAN,ORANGE);


//	string statText = format("Avg faces/frame: %4.1lf, total persons: %lld", statistics.facesPerFrame, statistics.faceCounter);
//	writeText(Irgb, statText, Point(startPoint.x,position.y), fontScale, ORANGE);

	//	writeText(Irgb, statText, position+Point(0,3*textSize.height/2), fontScale, ORANGE);

}
