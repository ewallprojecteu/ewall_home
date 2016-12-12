#include "VJ.h"
#include <iostream>
#include <cstdio>

using namespace std;
using namespace cv;

//Required globals from main
extern Mat  Igray; //Grayscale image from main

extern double SIGMA2_FACE; //From PF
//extern Mat	frgFull; //Full size foreground mask from main

//CvMemStorage			*storage = 0;
CascadeClassifier fine_cascade, cascadeRightEye, cascadeLeftEye;
Ptr<CvHaarClassifierCascade> haarcascades[MAX_HAAR_CASCADES_C];

// Face initialisation parameters, will be updated after finding out the video type
// These values are for old apne laptop webcam (Capture_20121217.wmv)
double					as=1.32, bs=-149.0;
int						yc=166, ye=30; //Center y and range

// Variables used for quick face search
vector <int>	searchLines;//Lines to be searched for faces in image
int				ySize;

// Foreground sum for both quick and detailed face search
Mat frgSum;

Mat vjSum,vjSqsum,vjTiltsum;


//double icvEvalHidHaarClassifier( CvHidHaarClassifier* classifier, double variance_norm_factor, size_t p_offset );

double icvEvalHidHaarClassifier2( CvHidHaarClassifier* classifier, double variance_norm_factor, size_t p_offset )
{
    int idx = 0;
    do
    {
        CvHidHaarTreeNode* node = classifier->node + idx;
        double t = node->threshold * variance_norm_factor;

        double sum = calc_sum(node->feature.rect[0],p_offset) * node->feature.rect[0].weight;
        sum += calc_sum(node->feature.rect[1],p_offset) * node->feature.rect[1].weight;

        if( node->feature.rect[2].p0 )
            sum += calc_sum(node->feature.rect[2],p_offset) * node->feature.rect[2].weight;

        idx = sum < t ? node->left : node->right;
    }
    while( idx > 0 );
    return classifier->alpha[-idx];
}


double aitRunHaarClassifierCascade( const CvHaarClassifierCascade* _cascade, CvPoint pt, int start_stage )
{
    int result = -1;

    int p_offset, pq_offset;
    int i, j;
    double VJmean, variance_norm_factor, likelihood=0;
    CvHidHaarClassifierCascade* cascade;
	double temp;

    if( !CV_IS_HAAR_CLASSIFIER(_cascade) )
        CV_Error( !_cascade ? CV_StsNullPtr : CV_StsBadArg, "Invalid cascade pointer" );

    cascade = _cascade->hid_cascade;
    if( !cascade )
        CV_Error( CV_StsNullPtr, "Hidden cascade has not been created.\n"
            "Use cvSetImagesForHaarClassifierCascade" );

    if( pt.x < 0 || pt.y < 0 ||
        pt.x + _cascade->real_window_size.width >= cascade->sum.width-2 ||
        pt.y + _cascade->real_window_size.height >= cascade->sum.height-2 )
	{
		fprintf(stderr,"aitRunHaarClassifierCascade running out of screen, x=%d,y=%d,w=%d,h=%d\n", pt.x, pt.y, _cascade->real_window_size.width, _cascade->real_window_size.height);
        return -1;
	}

    p_offset = pt.y * (cascade->sum.step/sizeof(sumtype)) + pt.x;
    pq_offset = pt.y * (cascade->sqsum.step/sizeof(sqsumtype)) + pt.x;
    VJmean = calc_sum(*cascade,p_offset)*cascade->inv_window_area;
    variance_norm_factor = cascade->pq0[pq_offset] - cascade->pq1[pq_offset] -
                           cascade->pq2[pq_offset] + cascade->pq3[pq_offset];
    variance_norm_factor = variance_norm_factor*cascade->inv_window_area - VJmean*VJmean;
    if( variance_norm_factor >= 0. )
        variance_norm_factor = sqrt(variance_norm_factor);
    else
        variance_norm_factor = 1.;

    if( cascade->is_tree )
    {
        CvHidHaarStageClassifier* ptr;
        assert( start_stage == 0 );

        result = 1;
        ptr = cascade->stage_classifier;

        while( ptr )
        {
            double stage_sum = 0;

            for( j = 0; j < ptr->count; j++ )
            {
                stage_sum += icvEvalHidHaarClassifier2( ptr->classifier + j,
                    variance_norm_factor, p_offset );
            }

            if( stage_sum >= ptr->threshold )
            {
                ptr = ptr->child;
				likelihood+=(1+(stage_sum-ptr->threshold)/fabs(ptr->threshold))*pow((double)10.0f,(double)(j-ptr->count));
            }
            else
            {
                while( ptr && ptr->next == NULL ) ptr = ptr->parent;
                if( ptr == NULL )
                    return likelihood;
                ptr = ptr->next;
            }
        }
    }
    else if( cascade->is_stump_based )
    {
        for( i = start_stage; i < cascade->count; i++ )
        {
#ifndef CV_HAAR_USE_SSE
            double stage_sum = 0;
#else
            __m128d stage_sum = _mm_setzero_pd();
#endif

            if( cascade->stage_classifier[i].two_rects )
            {
                for( j = 0; j < cascade->stage_classifier[i].count; j++ )
                {
                    CvHidHaarClassifier* classifier = cascade->stage_classifier[i].classifier + j;
                    CvHidHaarTreeNode* node = classifier->node;
#ifndef CV_HAAR_USE_SSE
                    double t = node->threshold*variance_norm_factor;
                    double sum = calc_sum(node->feature.rect[0],p_offset) * node->feature.rect[0].weight;
                    sum += calc_sum(node->feature.rect[1],p_offset) * node->feature.rect[1].weight;
                    stage_sum += classifier->alpha[sum >= t];
#else
                    // ayasin - NHM perf optim. Avoid use of costly flaky jcc
                    __m128d t = _mm_set_sd(node->threshold*variance_norm_factor);
                    __m128d a = _mm_set_sd(classifier->alpha[0]);
                    __m128d b = _mm_set_sd(classifier->alpha[1]);
                    __m128d sum = _mm_set_sd(calc_sum(node->feature.rect[0],p_offset) * node->feature.rect[0].weight +
                                             calc_sum(node->feature.rect[1],p_offset) * node->feature.rect[1].weight);
                    t = _mm_cmpgt_sd(t, sum);
                    stage_sum = _mm_add_sd(stage_sum, _mm_blendv_pd(b, a, t));
#endif
                }
            }
            else
            {
                for( j = 0; j < cascade->stage_classifier[i].count; j++ )
                {
                    CvHidHaarClassifier* classifier = cascade->stage_classifier[i].classifier + j;
                    CvHidHaarTreeNode* node = classifier->node;
#ifndef CV_HAAR_USE_SSE
                    double t = node->threshold*variance_norm_factor;
                    double sum = calc_sum(node->feature.rect[0],p_offset) * node->feature.rect[0].weight;
                    sum += calc_sum(node->feature.rect[1],p_offset) * node->feature.rect[1].weight;
                    if( node->feature.rect[2].p0 )
                        sum += calc_sum(node->feature.rect[2],p_offset) * node->feature.rect[2].weight;
                    
                    stage_sum += classifier->alpha[sum >= t];
#else
                    // ayasin - NHM perf optim. Avoid use of costly flaky jcc
                    __m128d t = _mm_set_sd(node->threshold*variance_norm_factor);
                    __m128d a = _mm_set_sd(classifier->alpha[0]);
                    __m128d b = _mm_set_sd(classifier->alpha[1]);
                    double _sum = calc_sum(node->feature.rect[0],p_offset) * node->feature.rect[0].weight;
                    _sum += calc_sum(node->feature.rect[1],p_offset) * node->feature.rect[1].weight;
                    if( node->feature.rect[2].p0 )
                        _sum += calc_sum(node->feature.rect[2],p_offset) * node->feature.rect[2].weight;
                    __m128d sum = _mm_set_sd(_sum);
                    
                    t = _mm_cmpgt_sd(t, sum);
                    stage_sum = _mm_add_sd(stage_sum, _mm_blendv_pd(b, a, t));
#endif
                }
            }

			//printf("Stage %d, relative sum %f (threshold %f)\n",i,(stage_sum-cascade->stage_classifier[i].threshold)/fabs(cascade->stage_classifier[i].threshold),cascade->stage_classifier[i].threshold);
#ifndef CV_HAAR_USE_SSE
            if( stage_sum < cascade->stage_classifier[i].threshold )
#else
            __m128d i_threshold = _mm_set_sd(cascade->stage_classifier[i].threshold);
            if( _mm_comilt_sd(stage_sum, i_threshold) )
#endif
                return likelihood;
			temp=(stage_sum-cascade->stage_classifier[i].threshold)/fabs(cascade->stage_classifier[i].threshold);
			if (temp>.9)
				temp=.9;
			likelihood+=(1+10.0*temp)*pow((double)10.0f,(double)(i-cascade->count));
        }
    }
    else
    {
        for( i = start_stage; i < cascade->count; i++ )
        {
            double stage_sum = 0;

            for( j = 0; j < cascade->stage_classifier[i].count; j++ )
            {
                stage_sum += icvEvalHidHaarClassifier2(
                    cascade->stage_classifier[i].classifier + j,
                    variance_norm_factor, p_offset );
            }
            if( stage_sum < cascade->stage_classifier[i].threshold )
                return likelihood;
			temp=(stage_sum-cascade->stage_classifier[i].threshold)/fabs(cascade->stage_classifier[i].threshold);
			if (temp>.9)
				temp=.9;
			likelihood+=(1+10.0*temp)*pow((double)10.0f,(double)(i-cascade->count));
        }
    }

    return likelihood;
}

// VJ classifier initialisation of the face and eye cascades
int initCascades(const string& faceFile, const string& leftEyeFile, const string& rightEyeFile)
{
	// Load the cascades
	if( !fine_cascade.load(faceFile)){
		cerr << "ERROR: Could not load face classifier cascade\n";
		return -1;
	};
	if( !cascadeLeftEye.load(leftEyeFile)){
		cerr << "ERROR: Could not load left eye classifier cascade\n";
		return -1;
	};
	if( !cascadeRightEye.load(rightEyeFile)){
		cerr << "ERROR: Could not load right eye classifier cascade\n";
		return -1;
	};

	// Haar cascades will be initialised from PF if face modality is enabled

	//cvReleaseHaarClassifierCascade(&cascade);
	//cascade = (CvHaarClassifierCascade *)cvLoad(faceFile.c_str(), 0, 0, 0);
	//if (!cascade)
	//{
	//	cerr << "ERROR: Could not load face classifier cascade\n";
	//	return -1;
	//}
//	storage = cvCreateMemStorage(0);
	return 0;
}

//Perform detailed face detection
std::vector<faceDetection> detailedFaceDetect(int64 framenum, const cv::Mat& Igray, const cv::Mat& frgFull)
{
	int			x, y, numFaces=0, step;
	double		scale, result, d;
	int			minwidth=200, maxwidth = 500;
	//int			minwidth=25, maxwidth = 150;

	vector<faceDetection> faces;

	cv::integral(frgFull,frgSum, CV_32S);

	// VJ integral image initialisation
//	cvClearMemStorage(storage);
	//cv::Mat grayROI = Igray(cv::Rect(0,ROIy,Igray.cols,ROIheight));
	cv::equalizeHist(Igray,Igray);
	cv::integral(Igray,vjSum,vjSqsum,vjTiltsum,CV_32S);
	//	imshow("Gray", Igray);
	IplImage vjSum_c = vjSum;
	IplImage vjSqsum_c = vjSqsum;
	IplImage vjTiltsum_c = vjTiltsum;
	int facewidth;
	for (double scalewidth=minwidth;scalewidth<maxwidth;scalewidth*=1.05)
	{
		facewidth=cvRound(scalewidth);
		scale=scalewidth/haarcascades[0]->orig_window_size.width;
		cvSetImagesForHaarClassifierCascade(haarcascades[0],&vjSum_c,&vjSqsum_c,&vjTiltsum_c,scale);
		step = std::max(cvRound(sqrt(facewidth/10.0)),2);
		for(y=0;y<Igray.rows-haarcascades[0]->real_window_size.height-1;y+=step)
		{
			for(x=0;x<Igray.cols-haarcascades[0]->real_window_size.width-1;x+=step)
			{
				d=averageInSumRect(frgSum, cv::Rect(x,y,haarcascades[0]->real_window_size.width,haarcascades[0]->real_window_size.height));
				// If the rectangle contains significant foreground
				if (d>115)
				{
					//				result = aitRunHaarClassifierCascade(cascade,cvPoint(x,y));
					result = aitRunHaarClassifierCascade(haarcascades[0],cvPoint(x,y));
					if (result==-1)
						cout << result << endl;
					if(result>MAX_VJ)
						result=MAX_VJ;
					if(result>.12)
					{
						faceDetection newFace;
						newFace.rect.height=haarcascades[0]->real_window_size.height;
						newFace.rect.width=haarcascades[0]->real_window_size.width;
						newFace.rect.x=x;
						newFace.rect.y=y;
						newFace.eyeL.x=newFace.eyeR.x=-1;
						//					cout << endl << result << " " << cv::Rect((faces+numFaces)->rect);
//						newFace.L=exp(-LFACE*(MAX_VJ-result));
						newFace.L=exp(-(MAX_VJ-result)/(2*SIGMA2_FACE));
						faces.push_back(newFace);
					}
				}
			}
		}
	}
	return faces;
}

//Estimate the step required based on the given faceSize
inline int calcStep(double faceSize, int minStep)
{
		//step = cvRound(sqrt(faceSize(y[ySize]))/10.0);
		//step = cvRound(faceSize(currentY)/15.0);
	//int step = cvRound(sqrt(faceSize/10.0));
	int step = cvRound(faceSize/15.0);
	step = step>5?5:step;
	return step<minStep?minStep:step;
}

//Calculate the expected mean face size at the given image height
inline double faceSize(double yc)
{
	return (as*yc+bs);
}

int initSearchLines(const std::string& cascadeFile)
{
	int currentY;
	// Face initialisation y
	searchLines.reserve(2*ye);//Reserve the maximum possible size
	currentY=yc-ye;
	do
	{
		searchLines.push_back(currentY);
		//Increase each Y by the corresponding step
		currentY+=calcStep(faceSize(currentY),4);
	}
	while (currentY<=yc+ye);
	ySize=searchLines.size();
	if (!haarcascades[0])
	{
		// Tracker does not contain face modality, need to load the first haarcascade
		haarcascades[0] = (CvHaarClassifierCascade *)cvLoad(cascadeFile.c_str(), 0, 0, 0);
		if (!haarcascades[0])
		{
			cerr << "initSearchLines ERROR: Could not load face classifier cascade: " << string(cascadeFile) << endl;
			return -1;
		}
	}
	return 0;
}


int	faceInit(std::vector<faceDetection>& faces, int64 framenum, int currentFaceNum=0)
{
	static int calls = 0;
	int			x, y, numFaces=currentFaceNum, step;
	double		scale, result, d, scaleFactor[]={.8,.9,1,1.1,1.2};

	int MAX_FACES=currentFaceNum+100;

	faces.resize(MAX_FACES);

	calls++;
	int ySize=searchLines.size();
	//TODO: If size>20, search on more than one line
	int yc=searchLines[framenum%ySize];
//	yc+=offset;
	// VJ integral image initialisation
//	cvClearMemStorage(storage);
	int ROIheight = cvRound(scaleFactor[4]*faceSize(yc));
	if (ROIheight<10) {
		//Check for case that should not happen normally,
		//but avoid crash if as and bs parameters have error
		cerr << "\nWarning: probably incorrect as, bs parameters\n";
		faces.resize(numFaces);
		return numFaces;
	}
	int ROIy = std::max(0,yc-cvRound(ROIheight/2));
	cv::Mat grayCopy = Igray.clone();
	cv::Mat grayROI = grayCopy(cv::Rect(0,ROIy,Igray.cols,ROIheight));
	cv::equalizeHist(grayROI,grayROI);
	//	cv::integral(Igray,vjSum,vjSqsum,vjTiltsum,CV_32S);
	// Add 2 rows below roi for correct calculations
	cv::integral(grayCopy(cv::Rect(0,ROIy,Igray.cols,ROIheight+2)),vjSum,vjSqsum,vjTiltsum,CV_32S);
	//	imshow("Gray", Igray);
	IplImage vjSum_c = vjSum;
	IplImage vjSqsum_c = vjSqsum;
	IplImage vjTiltsum_c = vjTiltsum;

	bool foundres=false;
	for (int s=0;s<sizeof(scaleFactor)/sizeof(double);s++)
	{
		scale=scaleFactor[s]*faceSize(yc)/haarcascades[0]->orig_window_size.width;
		cvSetImagesForHaarClassifierCascade(haarcascades[0],&vjSum_c,&vjSqsum_c,&vjTiltsum_c,scale);
		step = calcStep(faceSize(yc),2);
		y = std::max(0,yc-haarcascades[0]->real_window_size.height/2);
		for(x=0;x<Igray.cols-haarcascades[0]->real_window_size.width-1;x+=step)
		{
//			if (calls==2 && s == 2 && x == 0)
//				cout << "calls=" << calls << ", s=" << s << ", x=" << x << endl;
			d=averageInSumRect(frgSum, cv::Rect(x,y,haarcascades[0]->real_window_size.width,haarcascades[0]->real_window_size.height));
			// If the rectangle contains significant foreground
			if (d>115)
			{
				//				result = aitRunHaarClassifierCascade(cascade,cvPoint(x,y));
				result = aitRunHaarClassifierCascade(haarcascades[0],cvPoint(x,y-ROIy));
				if (result==-1)
					cout << result << endl;
				if(result>MAX_VJ)
					result=MAX_VJ;
				if(result>.12)
				{
					if (numFaces==MAX_FACES)
					{
						MAX_FACES=cvRound(1.5*MAX_FACES);
						faces.resize(MAX_FACES);
					}
					foundres=true;
					faces[numFaces].rect.height=haarcascades[0]->real_window_size.height;
					faces[numFaces].rect.width=haarcascades[0]->real_window_size.width;
					faces[numFaces].rect.x=x;
					faces[numFaces].rect.y=y;
					faces[numFaces].eyeL.x=faces[numFaces].eyeR.x=-1;//TODO: Check if needed
					//					cout << endl << result << " " << cv::Rect((faces+numFaces)->rect);
//					faces[numFaces++].L=exp(-LFACE*(MAX_VJ-result));
					faces[numFaces++].L=exp(-(MAX_VJ-result)/(2*SIGMA2_FACE));
				}
			}
		}
	}
	//	if (foundres)	cout << endl;
	faces.resize(numFaces);
	return numFaces;
}


int quickFaceSearch(vector<faceDetection>& faces, int64 framenum, const cv::Mat& frgFull)
{
	cv::integral(frgFull,frgSum, CV_32S);
	int numFaces=faceInit(faces,framenum);
	//Search at more lines
	if (ySize>20)
		numFaces=faceInit(faces,framenum+cvRound(ySize/2.0),numFaces);
	if (ySize>40)
	{
		numFaces=faceInit(faces,framenum+cvRound(ySize/4.0),numFaces);
		numFaces=faceInit(faces,framenum+cvRound(3*ySize/4.0),numFaces);
	}
	return numFaces;
}

void drawSearchLines(Mat& Irgb, int64 framenum, Scalar color)
{
	Size imgSize=Irgb.size();
	//Draw a line at the center y that is being searched
	cv::line(Irgb,cv::Point(0,searchLines[framenum%ySize]),cv::Point(imgSize.width,searchLines[framenum%ySize]),color);
	//Draw the additional searched lines
	if (ySize>20)
		cv::line(Irgb,cv::Point(0,searchLines[(framenum+cvRound(ySize/2.0))%ySize]),cv::Point(imgSize.width,searchLines[(framenum+cvRound(ySize/2.0))%ySize]),color);
	if (ySize>40)
	{
		cv::line(Irgb,cv::Point(0,searchLines[(framenum+cvRound(ySize/4.0))%ySize]),cv::Point(imgSize.width,searchLines[(framenum+cvRound(ySize/4.0))%ySize]),color);
		cv::line(Irgb,cv::Point(0,searchLines[(framenum+cvRound(3*ySize/4.0))%ySize]),cv::Point(imgSize.width,searchLines[(framenum+cvRound(3*ySize/4.0))%ySize]),color);
	}
}

void detectEyes(vector<faceDetection>& faces)
{
	cv::Mat		It, Iface;
	cv::Rect	faceRect, detectRect, eyeReg;
	int			i;
	double		scale;
	int numFaces=faces.size();

	vector<cv::Rect> detections;

	for(i=0;i<numFaces;i++)
	{
		faceRect=faces[i].rect;
		faces[i].eyeR.x=-1;
		faces[i].eyeL.x=-1;
		//Enlarge found rectangle by 10%
		faceRect.x-=cvRound(faceRect.width*.1);
		if(faceRect.x<0) faceRect.x=0; 
		faceRect.y-=cvRound(faceRect.height*.1);
		if(faceRect.y<0) faceRect.y=0;
		faceRect.width=cvRound(faceRect.width*1.2);
		if(faceRect.x+faceRect.width>Igray.cols) faceRect.width=Igray.cols-faceRect.x;
		faceRect.height=cvRound(faceRect.height*1.2);
		if(faceRect.y+faceRect.height>Igray.rows) faceRect.height=Igray.rows-faceRect.y;

		It=Igray(cv::Rect(faceRect)).clone();
		cv::equalizeHist(It,It);

		fine_cascade.detectMultiScale(It, detections, 1.05, 1, CASCADE_FIND_BIGGEST_OBJECT );
		if (detections.size()<=0)
			continue;

		detectRect = detections[0];
			if (detectRect.x <0 || detectRect.y <0)
				cout << endl << "face detected on " << detectRect << endl;
			faces[i].rect.x=detectRect.x+faceRect.x;
			faces[i].rect.y=detectRect.y+faceRect.y;
			faces[i].rect.width=detectRect.width;
			faces[i].rect.height=detectRect.height;

		faceRect=faces[i].rect;
		eyeReg.x=cvRound(faceRect.width*.55)+faceRect.x;
		eyeReg.y=cvRound(faceRect.height*.15)+faceRect.y;
		eyeReg.width=cvRound(faceRect.width*.45);
		eyeReg.height=cvRound(faceRect.height*.4);

		It=Igray(cv::Rect(eyeReg)).clone();
		cv::equalizeHist(It,It);
		scale=48.0/eyeReg.width;
		if(scale>1)
		{
			Iface.create(cv::Size(cvRound(eyeReg.width*scale),cvRound(eyeReg.height*scale)), CV_8UC1);
			cv::resize(It,Iface,Iface.size(),0,0,INTER_LINEAR);
		}
		else
		{
			Iface=It.clone();
			scale=1;
		}
		cascadeRightEye.detectMultiScale(Iface, detections, 1.05, 1, CASCADE_FIND_BIGGEST_OBJECT );
		if (detections.size()>0)
		{
			detectRect = detections[0];
			faces[i].eyeR.x=cvRound((detectRect.x+detectRect.width/2)/scale)+eyeReg.x;
			faces[i].eyeR.y=cvRound((detectRect.y+detectRect.height/2)/scale)+eyeReg.y;
		}

		eyeReg.x=cvRound(faceRect.width*.05)+faceRect.x;
		It=Igray(eyeReg).clone();
		cv::equalizeHist(It,It);
		scale=48.0/eyeReg.width;
		if(scale>1)
			cv::resize(It,Iface,Iface.size(),0,0,INTER_LINEAR);
		else
			Iface=It;
		cascadeLeftEye.detectMultiScale(Iface, detections, 1.05, 1, CASCADE_FIND_BIGGEST_OBJECT );
		if (detections.size()>0)
		{
			detectRect = detections[0];
			faces[i].eyeL.x=cvRound((detectRect.x+detectRect.width/2)/scale)+eyeReg.x;
			faces[i].eyeL.y=cvRound((detectRect.y+detectRect.height/2)/scale)+eyeReg.y;
		}
		//printf("Eye region: (%d, %d, %d, %d)\n",eyeReg.x,eyeReg.y,eyeReg.width,eyeReg.height);
	}
}
