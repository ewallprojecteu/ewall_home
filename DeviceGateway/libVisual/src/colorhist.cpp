#include "colorhist.hpp"
#include <iostream>
#include <iomanip>
#include <opencv2/highgui/highgui.hpp>
using namespace std;
using namespace cv;

double histUpdateRate=0.002; //color histogram learning rate (default 0.002)

//Histogram parameters
int crbins = 16, cbbins = 16, ybins = 8; //Number of bins for 3D histograms
int LEVELSPERCOLOUR = 16; //Number of bins for 1D histogram


//1D hist variables
int histSize_1D = LEVELSPERCOLOUR*LEVELSPERCOLOUR*LEVELSPERCOLOUR;
float imrange[] = { 0, (float) (LEVELSPERCOLOUR*LEVELSPERCOLOUR*LEVELSPERCOLOUR) } ;
const float* range1D[] = { imrange };
//Additional variables for creating 16bit image for 1D histogram
int LEVELSPERCOLOUR2=LEVELSPERCOLOUR*LEVELSPERCOLOUR;
double c_1D=256.0/LEVELSPERCOLOUR, c2_1D=c_1D/2.0, ci_1D=1/c_1D;

//3D hist variables
int histSize[] = {crbins, cbbins, ybins};
float crranges[] = { 0, 256 }, cbranges[] = { 0, 256 }, yranges[] = { 0, 256 };
const float* ranges[] = { crranges, cbranges, yranges };
int channels[] = {1, 2, 0};

void face2rectsColor(Rect faceRect, Rect limit, Rect out[FACE_RECTS_SIZE])
{
	//Parameters for the rectangles in face color histograms
	double xConstrainColor=0.05; //How much to constrain the face on each side of the x axis, default 0.05
	double yExpandColor=0.35;	//How much to expand the face above the top on y axis, default 0.35
	double expansionColor=0.5; //How much to expand the resulting rectangle of the face for background color estimation, default 0.5
	face2rects(faceRect, limit, out, expansionColor, xConstrainColor, yExpandColor);
}

// Prints the provided color histogram, if 3D it first spans the 3rd dimension, which is 
// assumed to be the brightness component and then the other 2
void printHist(Mat hist)
{
	std::streamsize prec=cout.precision(4);
	if (hist.dims==2)
	{
		for (int i=0;i<hist.size[0];i++)
		{
			for (int j=0;j<hist.size[1];j++)
			{
				std::cout.width(10);
				std::cout << hist.at<float>(i,j);
			}
			std::cout<<std::endl;
		}
		std::cout<<std::endl;
	}
	else if (hist.dims==3)
	{
		for (int k=0;k<hist.size[2];k++)
		{
			for (int i=0;i<hist.size[0];i++)
			{
				for (int j=0;j<hist.size[1];j++)
				{
					std::cout.width(10);
					std::cout << hist.at<float>(i,j,k);
				}
				std::cout<<std::endl;
			}
			std::cout<<std::endl;
		}
		std::cout<<std::endl;
	}
	cout.precision(prec);
}

//Combines the given face and background histograms
//into the face hist
void combineHist(InputOutputArray _face_hist, InputArray _bkg_hist)
{
	Mat face_hist = _face_hist.getMat();
	Mat bkg_hist = _bkg_hist.getMat();
	size_t numelements = face_hist.total();

	CV_Assert( bkg_hist.depth() == CV_32F && face_hist.depth() == CV_32F &&
		 face_hist.dims == bkg_hist.dims && numelements == bkg_hist.total() &&
		 face_hist.isContinuous() && bkg_hist.isContinuous());

	// Find the minimum value of the background that is larger than 0
	float minhb = FLT_MAX;
	float hm,hb;

	float* bkgPtr = bkg_hist.ptr<float>(0);
	for (unsigned int i=0;i<numelements;i++)
	{
			hb=bkgPtr[i];
			if (hb>0 && hb<minhb)
				minhb = hb;
	}

	float* facePtr = face_hist.ptr<float>(0);
		// Attenuate the background colors in the face histogram
	for (unsigned int i=0;i<numelements;i++)
	{
		hm=facePtr[i];
		if (hm<=0) continue;
		hb=bkgPtr[i];
		if (hb>hm/8)
			facePtr[i]=hm*minhb/hb;
	}
//	cout<<"new face hist"<<endl;
//	printHist(face_hist);
	normalize(face_hist, face_hist, 1, 0, NORM_L1);
}

int calcFaceHist(const Mat& YCrCb, Rect facerect, OutputArray _face_hist)
{
//	Mat YCrCb = _YCrCb.getMat();
	Rect imgrect = Rect(0,0,YCrCb.cols,YCrCb.rows);

	_face_hist.create(3, histSize, CV_32F);
	Mat face_hist = _face_hist.getMat();

	Mat bkg_hist;

	Rect facerects[FACE_RECTS_SIZE];
	face2rectsColor(facerect,imgrect,facerects);

// Using variable to avoid
// http://stackoverflow.com/questions/16481490/error-taking-address-of-temporary-fpermissive
    Mat roi = YCrCb(facerects[0]);

    calcHist(&roi, 1, channels, Mat(), face_hist, 3, histSize, ranges);
	//cout<<"face hist"<<endl;
	//printHist(face_hist);
	//	normalize(face_hist, face_hist, 0, 255, CV_MINMAX);
	normalize(face_hist, face_hist, 1, 0, NORM_L1);
	//cout<<"face hist normalized"<<endl;
	//printHist(face_hist);

#if 1 //Set to 0 to disable background colour modelling
    roi = YCrCb(facerects[1]);
    calcHist(&roi, 1, channels, Mat(), bkg_hist, 3, histSize, ranges);
	//	cout << endl << "bkg_hist" << bkg_hist << endl;
    roi = YCrCb(facerects[2]);
    calcHist(&roi, 1, channels, Mat(), bkg_hist, 3, histSize, ranges,true,true);
	//	cout << endl << "bkg_hist" << bkg_hist << endl;
    roi = YCrCb(facerects[3]);
    calcHist(&roi, 1, channels, Mat(), bkg_hist, 3, histSize, ranges,true,true);
	//	cout << endl << "bkg_hist" << bkg_hist << endl;
	//cout<<"bkg hist"<<endl;
	//printHist(bkg_hist);
	normalize(bkg_hist, bkg_hist, 1, 0, NORM_L1);
	//cout<<"bkg hist normalized"<<endl;
	//printHist(bkg_hist);
	//	normalize(bkg_hist, bkg_hist, 0, 255, CV_MINMAX);
	//	cout << endl << "bkg_hist" << bkg_hist << endl;

	//	cout << endl << "face_hist before" << face_hist << endl;
	combineHist(face_hist,bkg_hist);
	//	cout << endl << "face_hist after" << face_hist << endl;
	// cout<<"new face hist normalized"<<endl;
	//printHist(face_hist);
#endif

	return 0;
}

void backProject(InputArray _YCrCb, InputArray hist, OutputArray _backProjection)
{
	Mat YCrCb = _YCrCb.getMat();
	Size imsize = YCrCb.size();
	_backProjection.create(imsize,CV_8U);
	Mat backProjection=_backProjection.getMat();
	calcBackProject(&YCrCb, 1, channels, hist, backProjection, ranges);
}


void imageForHist1D(InputArray _Chan1, InputArray _Chan2, InputArray _Chan3, InputOutputArray _histImg)
{
	Mat Chan1 = _Chan1.getMat();
	Mat Chan2 = _Chan2.getMat();
	Mat Chan3 = _Chan3.getMat();

	int depth = Chan1.depth();
	Size imsize = Chan1.size();

	CV_Assert( depth == Chan2.depth() && depth == Chan3.depth() &&
		 imsize == Chan2.size() && imsize == Chan3.size() &&
		 Chan1.dims == 2 && Chan1.dims == 2 && Chan3.dims ==2);

	Mat histImg=_histImg.getMat();
	if (histImg.size()!=imsize || _histImg.depth()!=CV_16U)
	{
		_histImg.create(imsize,CV_16U);
		histImg=_histImg.getMat();
	}
	Mat tempImg;

	Chan1.convertTo(tempImg,CV_16U,1,-c2_1D);
	histImg = tempImg/c_1D;

	Chan2.convertTo(tempImg,CV_16U,1,-c2_1D);
	tempImg/=c_1D;
	histImg += LEVELSPERCOLOUR*tempImg;

	Chan3.convertTo(tempImg,CV_16U,1,-c2_1D);
	tempImg/=c_1D;
	histImg += LEVELSPERCOLOUR2*tempImg;
}

int calcFaceHist1D(InputOutputArray _histImg, Rect facerect, OutputArray _face_hist)
{
	Mat roi;

	_face_hist.create(histSize_1D, 1 , CV_32F);
	Mat face_hist = _face_hist.getMat();

	Mat histImg=_histImg.getMat();
	Rect imgrect = Rect(0,0,histImg.rows,histImg.cols);

	Mat bkg_hist, tmp_img;

	Rect facerects[FACE_RECTS_SIZE];
	face2rectsColor(facerect,imgrect,facerects);

	//Mat disp=Irgb.clone();
	//rectangle(disp,facerect,Scalar(0,0,255));
	//rectangle(disp,facerects[0],Scalar(0,255,0));
	//imshow("face histograms", disp);

	histImg(facerects[0]).convertTo(tmp_img,CV_32F);
//	cout << "\nface_hist\n" <<  face_hist << endl;

	calcHist(&tmp_img, 1, 0, Mat(), face_hist, 1, &histSize_1D, range1D);
//	cout << "\nface_hist_1\n" << face_hist << endl;
	//cout << endl << "face_hist" << face_hist << endl;
	normalize(face_hist, face_hist, 1, 0, NORM_L1);

	histImg(facerects[1]).convertTo(tmp_img,CV_32F);
	calcHist(&tmp_img, 1, 0, Mat(), bkg_hist, 1, &histSize_1D, range1D);
	//cout << endl << "bkghist" << bkg_hist << endl;
	histImg(facerects[2]).convertTo(tmp_img,CV_32F);
	calcHist(&tmp_img, 1, 0, Mat(), bkg_hist, 1, &histSize_1D, range1D,true,true);
	//cout << endl << "bkghist" << bkg_hist << endl;
	histImg(facerects[3]).convertTo(tmp_img,CV_32F);
	calcHist(&tmp_img, 1, 0, Mat(), bkg_hist, 1, &histSize_1D, range1D,true,true);
	normalize(bkg_hist, bkg_hist, 1, 0, NORM_L1);
	//cout << endl << "bkghist" << endl << setw(10) << bkg_hist << endl;

//	Mat face_hist2 = face_hist.clone();

	//cout << endl << "face_hist before" << endl  << setw(10) << face_hist << endl;
	combineHist(face_hist,bkg_hist);
	//cout << endl << "face_hist after" << endl << setw(10) << face_hist << endl;
	// cout<<"new face hist normalized"<<endl;
	//printHist(face_hist);

//	Mat backproj1, backproj2;
	//double maxval;
	//minMaxIdx(face_hist2,NULL,&maxval);
	//face_hist2.convertTo(face_hist2,-1,255/maxval,0);
	//calcBackProject(&histImg, 1, 0, face_hist2, backproj1, &histrange);
	//minMaxIdx(face_hist,NULL,&maxval);
	//face_hist.convertTo(face_hist,-1,255/maxval,0);

	////Use convert instead of normalize, as it doesn't work on 3dimensional matrixes
	//	//face_hist=(255/maxval)*face_hist;
	//calcBackProject(&histImg, 1, 0, face_hist, backproj2, &histrange);
	//backproj1.convertTo(backproj1,CV_8U);
	//backproj2.convertTo(backproj2,CV_8U);
	//imshow("backproj without attenuation", backproj1);
	//imshow("backproj with attenuation", backproj2);
	//waitKey();

	return 0;
}

void backProject1D(InputArray _histImg, InputArray hist, OutputArray _backProjection)
{
	Mat histImg = _histImg.getMat();
	Size imsize = histImg.size();
	_backProjection.create(imsize,CV_8U);
	Mat backProjection=_backProjection.getMat();
	Mat backprojtmp;
	calcBackProject(&histImg, 1, 0, hist, backprojtmp, range1D);
//	cout << "\nhistimg_2\n" << histImg << endl;
//	cout << "\nbackprojtmp\n" << backprojtmp << endl;

	//For 1D histograms, the returned backprojection has to be converted to 8U
	backprojtmp.convertTo(backProjection,CV_8U);
}

// Calculates the histogram at the specified region, set accumulate to true to avoid zeroing out the previous histogram
void calcImageHist(InputArray _YCrCb, Rect regionrect, InputOutputArray _hist, bool accumulate)
{
	Mat YCrCb = _YCrCb.getMat();
	Mat hist = _hist.getMat();
	if (hist.dims==0)
	{
		_hist.create(3, histSize, CV_32F);
		hist = _hist.getMat();
	}
    Mat roi = YCrCb(regionrect);
    calcHist(&roi, 1, channels, Mat(), hist, 3, histSize, ranges,true,accumulate);
}

// Calculates the histogram at the specified region, set accumulate to true to avoid zeroing out the previous histogram
void calcImageHist1D(InputArray _histImg, Rect regionrect, InputOutputArray _hist, bool accumulate)
{
	Mat histImg = _histImg.getMat();
	Mat hist = _hist.getMat();
	if (hist.dims==0)
	{
		_hist.create(histSize_1D, 1 , CV_32F);
		hist = _hist.getMat();
	}
    Mat roi = histImg(regionrect);
    calcHist(&roi, 1, 0, Mat(), hist, 1, &histSize_1D, range1D, true, accumulate);
}
