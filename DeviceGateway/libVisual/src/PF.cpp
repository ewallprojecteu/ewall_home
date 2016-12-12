#include "PF.h"
#include "base.h"
#include "tracker.h"
#include "colorhist.hpp"

#include <iostream>
#include <cstdio>
//#include <iomanip>
//#include <fstream>

using namespace cv;
using namespace std;


//#define LCOLOR					8
#define MAX_COLOR        1.0 //0.74
double SIGMA2_COLOR	 = 0.0625; //2^-4, 1/(2*LCOLOR);

//#define LMOTION					(1.0/64)	//0.06125
#define MAX_MOT					255 //217
double SIGMA2_MOTION = 0.125; //2^-3, 1/(2*LMOTION*MAX_MOT)=0.12549;

//#define LFACE					256		//36 = 1/(2*SIGMA2_FACE*MAX_VJ)
double SIGMA2_FACE	=	0.015625; //=2^-6 (ideally 1/(2*LFACE*MAX_V)=0.013950893)

// Large particle spread threshold
double LARGE_SPREAD = 0.5;

// Weight of top rectangle in motion likelihood
double TOP_WEIGHT_MOTION = 4.0;

uint64				seed=0;//time(NULL);
RNG	rng=RNG(seed); //nkat: use this in functions that will be called unconditionally
RNG	rng2=RNG(seed); //nkat: use this in conditional branches

//Required globals from main
extern Mat	IyCrCb; //Input image in YCrb colorspace from main
extern Mat  Igray; //Grayscale image from main
extern Mat	Irgb; //For displaying debug images

//extern double mydec;
//extern IplImage		*Icolhist, *IdSum;
//extern FILE			*fid;
//extern double		expand;

// Required global from VJ for face modality
extern Ptr<CvHaarClassifierCascade> haarcascades[MAX_HAAR_CASCADES_C];
//extern CascadeClassifier fine_cascade; //to enable when tracker uses full res

// Required globals from tracker
extern Mat frg_tracker; //Foreground mask from tracker
extern double LARGE_LIKELIHOOD_DROP; //Likelihood drop above which some particles are forced back to the state
extern bool use1DHist; //Whether to use 1D or 3D histograms
extern Mat histimg; //This image is needed for 1D histogram calculations

int TRACKING_CUE = CUE_COLORMOTION;

bool debugPF=false; //enables detailed display of all PF steps

int PAR_FOR_STRIPES=-1; //Parallel_for_ stripes

// Images for face likelihood calculations
Mat graySum,graySqsum,grayTiltSum;
IplImage graySumC, graySqSumC, grayTiltSumC;

// Motion evidence sum image for motion likelihood calculation
Mat mevSum, motionEvidence;

void saveParticles(const particle *particles, int np)
{
	FILE *fp=fopen("particles.bin","w");
	fwrite(particles,sizeof(particle)*np,1,fp);
	fclose(fp);
}

void loadParticles(particle *particles, int np)
{
	FILE *fp=fopen("particles.bin","r");
	fread(particles,sizeof(particle)*np,1,fp);
	fclose(fp);
}

void testFaceMeasurement()
{
	Mat img=imread("test.png");
	cvtColor(img,Igray,CV_RGB2GRAY);
	Mat graySum,graySqSum,grayTiltSum;
	cv::integral(Igray,graySum,graySqSum,grayTiltSum);
	graySumC=graySum, graySqSumC=graySqSum, grayTiltSumC=grayTiltSum;
	particle particles[NP];
	loadParticles(particles,NP);
	drawParticles(img,particles,NP);
	imshow("particles",img);
	waitKey(0);
	double Lface[NP];
	measureFaceLikelihood(particles,Lface,NP);
	getchar();
}

void resample(particle *particles)
{
	double			u, cumsum[NP];
	unsigned int	i, j;
	State			state[NP];

	cumsum[0]=particles->w;
	for(i=1;i<NP;i++)
		cumsum[i]=cumsum[i-1]+(particles+i)->w;
	u=rng.uniform((double)0,(double)1)/NP;
	for (i = 0; i < NP; i++)
	{
		j = 0;
		while (cumsum[j++] < u) ;
		state[i].angle = (particles+(--j))->state.angle;
		state[i].size = (particles+j)->state.size;
		state[i].x = (particles+j)->state.x;
		state[i].y = (particles+j)->state.y;
		u += 1.0f / NP;
	}
	for (i = 0; i < NP; i++)
	{
		(particles+i)->w=1.0f/NP;
		(particles+i)->state.angle=state[i].angle;
		(particles+i)->state.size=state[i].size;
		(particles+i)->state.x=state[i].x;
		(particles+i)->state.y=state[i].y;
	}
}


State stateEstimation(particle *particles, int type)
{
	State	xo,state;
	double	maxw=0,w;
	int		i, idx;

	if (type==MAP_STATE_ESTIMATION)
	{
		for (i=0;i<NP;i++)
			if (maxw<(particles+i)->w)
			{
				maxw=(particles+i)->w;
				idx=i;
			}
			state=(particles+idx)->state;
			xo.angle=state.angle;
			xo.size=state.size;
			xo.x=state.x;
			xo.y=state.y;
	}
	else if (type==MMSE_STATE_ESTIMATION)
	{
		xo.angle=0;
		xo.size=0;
		xo.x=0;
		xo.y=0;
		for (i=0;i<NP;i++)
		{
			state=(particles+i)->state;
			w=(particles+i)->w;
			xo.angle+=w*state.angle;
			xo.size+=w*state.size;
			xo.x+=w*state.x;
			xo.y+=w*state.y;
		}
	}
	return xo;
}


void gaussianObjectModel(targetStruct *target)
{
	Mat			x(target->C.cols,NP,CV_64F);
	int			i, j;

	for (i=0;i<x.rows;i++)
		for (j=0;j<NP;j++)
			x.at<double>(i,j)=rng.gaussian((double)1);
	x=target->C*x;
	for (i=0;i<NP;i++)
	{
		State *state = &(target->particles[i].state);
		state->x+=x.at<double>(0,i);
		state->y+=x.at<double>(1,i);
		state->size+=x.at<double>(2,i);
		if (state->size<20)
			state->size=20;
		state->angle+=x.at<double>(3,i);
	}
}


void gaussianUniformObjectModel(targetStruct *target)
{
	Mat		xg(target->C.cols,NP,CV_64F), xu(target->C.cols,NP,CV_64F);
	int		i, j;
	double	uniRange=target->C.at<double>(2,2)*50*target->spread;
	double	mode, std;

	if (uniRange>IyCrCb.rows/2)
		uniRange=IyCrCb.rows/2;
	//TODO: replace the below loop with rng fill
	//rng.fill(xg,RNG::NORMAL,0,1);
	//rng.fill(xu,RNG::UNIFORM,-uniRange,uniRange);
	for (i=0;i<xg.rows;i++)
	{
		for (j=0;j<NP;j++)
		{
			xg.at<double>(i,j)=rng.gaussian((double)1);
			xu.at<double>(i,j)=rng2.uniform(-uniRange,uniRange);
		}
	}
	xg=target->C*xg;
	std=4*sqrt(target->C.at<double>(2,2));
	for (i=0;i<NP;i++)
	{
		State *state = &(target->particles[i].state);
		mode=rng2.uniform((double)0,(double)1);
		if (mode>target->spread || mode>.25)
		{
			state->x+=xg.at<double>(0,i);
			state->y+=xg.at<double>(1,i);
		}
		else
		{
			state->x+=xu.at<double>(0,i);
			state->y+=xu.at<double>(1,i);
		}
//		if (state->size<40 && state->size/target->spread<800)
//			xg.at<double>(2,i)+=std;
		state->size+=xg.at<double>(2,i);
		state->angle+=xg.at<double>(3,i);
		if (state->size<20)
			state->size=20;
		if (state->x<0)
			state->x=0;
		if (state->x>IyCrCb.cols)
			state->x=IyCrCb.cols;
		if (state->y>IyCrCb.rows)
			state->y=IyCrCb.rows;
	}
}


void gaussianUniformObjectModelOld(particle *particles, Mat C, double spread)
{
	Mat		xg(C.cols,NP,CV_64F), xu(C.cols,NP,CV_64F);
	int		i, j;
	double	uniRange=C.at<double>(2,2)*50*spread;
	double	mode, std;

	if (uniRange>IyCrCb.rows/2)
		uniRange=IyCrCb.rows/2;
	for (i=0;i<xg.rows;i++)
		for (j=0;j<NP;j++)
		{
			xg.at<double>(i,j)=rng.gaussian((double)1);
			xu.at<double>(i,j)=rng.uniform(-uniRange,uniRange);
		}
		xg=C*xg;
		std=4*sqrt(C.at<double>(2,2));
		for (i=0;i<NP;i++)
		{
			mode=rng.uniform((double)0,(double)1);
			if (mode>spread || mode>.25)
			{
				(particles+i)->state.x+=xg.at<double>(0,i);
				(particles+i)->state.y+=xg.at<double>(1,i);
			}
			else
			{
				(particles+i)->state.x+=xu.at<double>(0,i);
				(particles+i)->state.y+=xu.at<double>(1,i);
			}
			if ((particles+i)->state.size<40 && (particles+i)->state.size/spread<800)
				xg.at<double>(2,i)+=std;
			(particles+i)->state.size+=xg.at<double>(2,i);
			(particles+i)->state.angle+=xg.at<double>(3,i);
			if ((particles+i)->state.size<20)
				(particles+i)->state.size=20;
			if ((particles+i)->state.x<0)
				(particles+i)->state.x=0;
			if ((particles+i)->state.x>mevSum.cols)
				(particles+i)->state.x=mevSum.cols;
			if ((particles+i)->state.y>mevSum.rows)
				(particles+i)->state.y=mevSum.rows;
		}
}

#if 0
void weightUpdate(particle *particles, cv::Mat colorModel, double *Lmax)
{
	double	Lface[NP], Lcol[NP], Lm[NP], L[NP], Lsum=0;
	int		i;
	double	maxFace=0, maxColor=0, maxMotion=0, Lmean[3]={0,0,0};

	*Lmax=0;
	if ((TRACKING_CUE & CUE_FACE)!=0)
		measureFaceLikelihood(particles,Lface);
	if ((TRACKING_CUE & CUE_COLOR)!=0)
		measureColorLikelihood(particles,colorModel,Lcol);
	if ((TRACKING_CUE & CUE_MOTION)!=0)
		measureMotionLikelihood(particles,Lm);
	for(i=0;i<NP;i++)
	{
		if (maxFace>Lface[i])
			maxFace=Lface[i];
		if (maxColor>Lcol[i])
			maxColor=Lcol[i];
		if (maxMotion>Lm[i])
			maxMotion=Lm[i];
	}
	maxFace/=SATFACTOR;
	maxColor/=SATFACTOR;
	maxMotion/=SATFACTOR;
	for(i=0;i<NP;i++)
	{
		if (Lface[i]<maxFace)
			Lface[i]=maxFace;
		if (Lcol[i]<maxColor)
			Lcol[i]=maxColor;
		if (Lm[i]<maxMotion)
			Lm[i]=maxMotion;
		L[i]=1;
		if ((TRACKING_CUE & CUE_FACE)!=0)
			L[i]*=Lface[i];
		if ((TRACKING_CUE & CUE_MOTION)!=0)
			L[i]*=Lm[i];
		if ((TRACKING_CUE & CUE_COLOR)!=0)
			L[i]*=Lcol[i];
		//if (TRACKING_CUE==CUE_FACE)
		//	L[i]=Lface[i];
		//else if (TRACKING_CUE==CUE_MOTION)
		//	L[i]=Lm[i];
		//else if (TRACKING_CUE==CUE_COLOR)
		//	L[i]=Lcol[i];
		//else if (TRACKING_CUE==CUE_FACEMOTION)
		//	L[i]=Lface[i]*Lm[i];
		//else if (TRACKING_CUE==CUE_FACECOLOR)
		//	L[i]=Lface[i]*Lcol[i];
		//else if (TRACKING_CUE==CUE_COLORMOTION)
		//	L[i]=Lcol[i]*Lm[i];
		//else if (TRACKING_CUE==CUE_FACECOLORMOTION)
		//	L[i]=Lface[i]*Lcol[i]*Lm[i];
		Lsum+=L[i];
		if (*Lmax<L[i])
			*Lmax=L[i];
	}
	//if (SAVE_IMAGES)
	//{
	//	for(i=0;i<NP;i++)
	//	{
	//		Lmean[0]+=Lface[i];
	//		Lmean[1]+=Lcol[i];
	//		Lmean[2]+=Lm[i];
	//	}
	//	Lmean[0]/=NP;
	//	Lmean[1]/=NP;
	//	Lmean[2]/=NP;
	//	for(i=0;i<NP;i++)
	//		if (*Lmax==L[i])
	//		{
	//			fprintf(fid," %05.3g %05.3g %05.3g %05.3g %05.3g %05.3g",Lface[i],Lcol[i],Lm[i],Lface[i]/Lmean[0],Lcol[i]/Lmean[1],Lm[i]/Lmean[2]);
	//			break;
	//		}
	//}
	for(i=0;i<NP;i++)
	{
		if(Lsum==0)
			(particles+i)->w=1.0f/NP;
		else
			(particles+i)->w=L[i]/Lsum;
	}
}
#endif

double particleSpread(particle *particles)
{
	int		i;
	State	xm, xm2, xs;
	double	spread, t;

	xm.angle=0;
	xm.size=0;
	xm.x=0;
	xm.y=0;
	xm2.angle=0;
	xm2.size=0;
	xm2.x=0;
	xm2.y=0;
	for(i=0;i<NP;i++)
	{
		xm.angle+=(particles+i)->state.angle;
		xm.size+=(particles+i)->state.size;
		xm.x+=(particles+i)->state.x;
		xm.y+=(particles+i)->state.y;
		xm2.angle+=(particles+i)->state.angle*(particles+i)->state.angle;
		xm2.size+=(particles+i)->state.size*(particles+i)->state.size;
		xm2.x+=(particles+i)->state.x*(particles+i)->state.x;
		xm2.y+=(particles+i)->state.y*(particles+i)->state.y;
	}
	xm.angle/=NP;
	xm.size/=NP;
	xm.x/=NP;
	xm.y/=NP;
	xm2.angle/=NP;
	xm2.size/=NP;
	xm2.x/=NP;
	xm2.y/=NP;
	t=xm2.angle-xm.angle*xm.angle;
	if (t>0)
		xs.angle=sqrt(t);
	else
		xs.angle=0;
	t=xm2.size-xm.size*xm.size;
	if (t>0)
		xs.size=sqrt(t);
	else
		xs.size=0;
	t=xm2.x-xm.x*xm.x;
	if (t>0)
		xs.x=sqrt(t);
	else
		xs.x=0;
	t=xm2.y-xm.y*xm.y;
	if (t>0)
		xs.y=sqrt(t);
	else
		xs.y=0;
	spread=xs.size;
	if (xs.x>spread)
		spread=xs.x;
	if (xs.y>spread)
		spread=xs.y;
	spread/=xm.size;

	return spread;
}

#define FACELKHOOD_ONESIZE 1

double faceLikelihood(int x, int y, int size, const IplImage *imgSum, const IplImage *imgSqsum, const IplImage *imgTiltsum, cv::Mutex* mtx = NULL, int cascadeno = 0)
{
	double	L, scale;
	CvSize	VJsize=haarcascades[0]->orig_window_size;
	int		xs, ys;

	if(cascadeno!=0 && mtx==NULL)
	{
		fprintf(stderr,"Called parallel face likelihood without mutex");
		return -1;
	}

	// Ensure that the rectangle fits the sum image
	if (size>imgSum->width-3)
		size=imgSum->width-3;
	if (size>imgSum->height-3)
		size=imgSum->height-3;

	if(x<0) x=0;
	if(y<0) y=0;

	if(x+size >=imgSum->width-2)
		x=imgSum->width-3-size;
	if(y+size >=imgSum->height-2)
		y=imgSum->height-3-size;
	// Lock mutex to perform all calculations at specified scale
	if (mtx!=NULL) mtx->lock();

	//Init haar classifier if not initialized or if scale/size is incorrect
	if(!haarcascades[cascadeno]->hid_cascade || size!=haarcascades[cascadeno]->real_window_size.width || 
		size!=haarcascades[cascadeno]->real_window_size.height ||
		imgSum->height!=haarcascades[cascadeno]->hid_cascade->sum.height || 
		imgSum->width!=haarcascades[cascadeno]->hid_cascade->sum.width)
	{
		scale=(double)size/(double)VJsize.width;
		cvSetImagesForHaarClassifierCascade(haarcascades[cascadeno],imgSum,imgSqsum,imgTiltsum,scale);
	}
	L = 4*aitRunHaarClassifierCascade(haarcascades[cascadeno],cvPoint(x,y));
	xs=x-cvRound(.05*size);
	if (xs<0)
		xs=0;
	L += aitRunHaarClassifierCascade(haarcascades[cascadeno],cvPoint(xs,y));
	xs=x+cvRound(.05*size);
	if (xs+size>=imgSum->width-2)
		xs=imgSum->width-3-size;
	L += aitRunHaarClassifierCascade(haarcascades[cascadeno],cvPoint(xs,y));
	ys=y-cvRound(.05*size);
	if (ys<0)
		ys=0;
	L += aitRunHaarClassifierCascade(haarcascades[cascadeno],cvPoint(x,ys));
	ys=y+cvRound(.05*size);
	if (ys+size>=imgSum->height-2)
		ys=imgSum->height-3-size;
	L += aitRunHaarClassifierCascade(haarcascades[cascadeno],cvPoint(x,ys));
	if (mtx!=NULL) mtx->unlock();
#if FACELKHOOD_ONESIZE
	L /=8;
#else //FACELKHOOD_ONESIZE
	int ss;
	ss=(int)(.95*size);
	if (ss<20)
		ss=20;
	scale=(double)ss/(double)VJsize.width;
	if (mtx!=NULL) mtx->lock();
	cvSetImagesForHaarClassifierCascade(haarcascades[cascadeno],imgSum,imgSqsum,imgTiltsum,scale);
	xs=x+ss/2;
	if (xs+ss>=imgSum->width-2)
		xs=imgSum->width-3-ss;
	ys=y+ss/2;
	if (ys+ss>=imgSum->height-2)
		ys=imgSum->height-3-ss;
	L += aitRunHaarClassifierCascade(haarcascades[cascadeno],cvPoint(xs,ys));
	if (mtx!=NULL) mtx->unlock();
	ss=(int)(1.05*size);
	if (ss>=imgSum->height-2)
		ss=imgSum->height-3;
	scale=(double)ss/(double)VJsize.width;
	if (mtx!=NULL) mtx->lock();
	cvSetImagesForHaarClassifierCascade(haarcascades[cascadeno],imgSum,imgSqsum,imgTiltsum,scale);
	xs=x-ss/2;
	if (xs<0)
		xs=0;
	ys=y-ss/2;
	if (ys<0)
		ys=0;
	L += aitRunHaarClassifierCascade(haarcascades[cascadeno],cvPoint(xs,ys));
	if (mtx!=NULL) mtx->unlock();
	L /=10;
#endif //FACELKHOOD_ONESIZE
	if (L>MAX_VJ)
		L=MAX_VJ;

	return L;
}

void measureFaceLikelihood(particle *particles, double *Lface, int np)
{
	int		i;
	Rect	rect, limitRect=Rect(0,0,Igray.cols,Igray.rows);
//	ofstream facefile("../../results/Lface.txt",ios::app);
//	double Lmax=0;

	for(i=0;i<np;i++)
	{
		//		rect=constraintRect(cvRect(cvRound((particles+i)->state.x-0.5*(particles+i)->state.size), cvRound((particles+i)->state.y-0.5*(particles+i)->state.size), cvRound((particles+i)->state.size), cvRound((particles+i)->state.size)), limitRect);
		rect=Rect(cvRound((particles+i)->state.x-0.5*(particles+i)->state.size), cvRound((particles+i)->state.y-0.5*(particles+i)->state.size), 
			cvRound((particles+i)->state.size), cvRound((particles+i)->state.size)) & limitRect;
		if (rect.width>rect.height)
			rect.width=rect.height;
		if (rect.width<20)
			rect.width=20;
		if ((particles+i)->state.size>1.2*rect.width)
		{
			// Particle too much out of screen, minimum likelihood
			//Lface[i]=exp(-LFACE*MAX_VJ);
			Lface[i]=exp(-1/(2*SIGMA2_FACE));
		}
		else
		{
			double L = faceLikelihood(rect.x, rect.y, rect.width, &graySumC, &graySqSumC, &grayTiltSumC);
//			if (L>Lmax)
//				Lmax=L;
			//Lface[i]=exp(-LFACE*(MAX_VJ-L));
			Lface[i]=exp(-(MAX_VJ-L)/(2*SIGMA2_FACE*MAX_VJ));
		}
		//cout << "rect[" << i << "]=" << rect << endl;
		//cout << "Lface[" << i << "]=" << Lface[i] << endl;
	}
	//cout << "end serial" << endl;
//	facefile << Lmax << endl;
}

void measureColorLikelihood(particle *particles, Mat colorModel, double *Lcolor, int np)
{
	int			i, k;
	Rect		r[FACE_RECTS_SIZE], limitRect=Rect(0,0,IyCrCb.cols,IyCrCb.rows);
	Mat			hist, hext;
	bool		accumulate=false;
//	ofstream colourfile("../../results/Lcolour.txt",ios::app);
//	double Lmax=0;

	for(i=0;i<np;i++)
	{
		//nkat:no need to calculate all rectangles
		Rect particlerect(cvRound((particles+i)->state.x-(particles+i)->state.size/2.0),
			cvRound((particles+i)->state.y-(particles+i)->state.size/2.0),
			cvRound((particles+i)->state.size),	cvRound((particles+i)->state.size));
		Rect inScreen=particlerect&limitRect;
		if (inScreen.area()<0.5*particlerect.area())
		{
			// Particle too much out of screen, minimum likelihood
			//Lcolor[i]=exp(-(double)LCOLOR);
			Lcolor[i]=exp(-1/(2*SIGMA2_COLOR));
		}
		else
		{
			face2rectsColor(particlerect, limitRect, r);
			if (!use1DHist)
				calcImageHist(IyCrCb,r[0],hist);
			else
				calcImageHist1D(histimg,r[0],hist);
			normalize(hist, hist, 1, 0, NORM_L1);
			for (k=1; k<4; k++)
				if (r[k].width>1 && r[k].height>1)
				{
					if (!use1DHist)
						calcImageHist(IyCrCb,r[k],hext,accumulate);
					else
						calcImageHist1D(histimg,r[k],hext,accumulate);
					accumulate=true;
				}
				normalize(hext, hext, 1, 0, NORM_L1);
				double L = pow(cv::compareHist(hext,colorModel,CV_COMP_BHATTACHARYYA),2)-pow(cv::compareHist(hist,colorModel,CV_COMP_BHATTACHARYYA),2);
				if (L>MAX_COLOR) L=MAX_COLOR;
//				if (L>Lmax)
//					Lmax=L;
			//Lcolor[i]=exp(-LCOLOR*(1-L));
				Lcolor[i]=exp(-(MAX_COLOR-L)/(2*SIGMA2_COLOR*MAX_COLOR));
		}
	}
//	colourfile << Lmax<< endl;
}

void measureMotionLikelihood(particle *particles, double *Lmotion, int np)
{
	int			i, k;
	//	Scalar	m;
	double		L;
	Rect		r[FACE_RECTS_SIZE], limitRect=Rect(0,0,IyCrCb.cols,IyCrCb.rows);
//	ofstream motionfile("../../results/Lmotion.txt",ios::app);
//	double Lmax=0;

	for(i=0;i<np;i++)
	{
		//nkat:no need to calculate all rectangles ??
		Rect particlerect(cvRound((particles+i)->state.x-(particles+i)->state.size/2.0),
			cvRound((particles+i)->state.y-(particles+i)->state.size/2.0),
			cvRound((particles+i)->state.size),	cvRound((particles+i)->state.size));
		face2rectsMotion(particlerect, limitRect, r);
//		Mat tmp;
		if (r[0].area()<0.8*(particles+i)->state.size*(particles+i)->state.size)
		{
			// Particle too much out of screen, minimum likelihood
			//Lmotion[i]=exp(-LMOTION*MAX_MOT);
			Lmotion[i]=exp(-1/(2*SIGMA2_MOTION));
		}
		else
		{
//			cv::cvtColor(motionEvidence, tmp, CV_GRAY2BGR);
//			rectangle(tmp,particlerect,Scalar(255,0,0));
			L=0;
			//Calculate background motion (negative)
			for (k=1; k<4; k++)
			{
				if (r[k].width!=0 && r[k].height!=0)
				{
//					rectangle(tmp,r[k],Scalar(0,0,255));
					if (k==1) //Increased negative weight for top rectangle
						L+=TOP_WEIGHT_MOTION*averageInSumRect(mevSum,r[k])*r[k].height*r[k].width;
					else
						L+=averageInSumRect(mevSum,r[k])*r[k].height*r[k].width;
				}
			}
//			rectangle(tmp,r[4],Scalar(0,255,0));
			L/=-r[4].width*r[4].height+r[0].width*r[0].height;

			//Add foreground motion
			L+=averageInSumRect(mevSum,r[0]);
			if (L<0)
				L=0;
			else if (L>MAX_MOT)
				L=MAX_MOT;
//			if (L>Lmax)	Lmax=L;
			//Lmotion[i]=exp(-LMOTION*(MAX_MOT-L));
			Lmotion[i]=exp(-(MAX_MOT-L)/(2*SIGMA2_MOTION*MAX_MOT));
//			if(np==27) cout<< endl<< i << ": " << L << ", " << Lmotion[i];
		}
	}
//	motionfile << Lmax << endl;
}

class Parallel_measureLikelihoods: public cv::ParallelLoopBody
{   
private:
	particle *particles;
	double *Lface;
	double *Lcolor;
	double *Lmotion;
	Rect limitRect;
	Mat colorModel;
	vector<Mutex *> mtx;

public:
	Parallel_measureLikelihoods(particle *givenParticles, double *givenLface, double *givenLcolor, double *givenLmotion, Mat givenColorModel, 
		vector<Mutex *> givenMtx)
		: particles(givenParticles), Lface(givenLface), Lcolor(givenLcolor), Lmotion(givenLmotion),
		colorModel(givenColorModel), limitRect(Rect(0,0,IyCrCb.cols,IyCrCb.rows))
	{
		mtx.resize(MAX_HAAR_CASCADES_C);
		for (int i=0;i<MAX_HAAR_CASCADES_C;i++)
			mtx[i]=givenMtx[i];
	}

	virtual void operator()( const cv::Range &rng ) const {
		Rect		r[FACE_RECTS_SIZE];
		Mat			hist, hext;
		int k;
		double L;
		for (int i = rng.start; i != rng.end; i++)
		{

			if ((TRACKING_CUE & CUE_FACE)!=0)
			{
				// FACE
				Rect rect=Rect(cvRound((particles+i)->state.x-0.5*(particles+i)->state.size), cvRound((particles+i)->state.y-0.5*(particles+i)->state.size), 
					cvRound((particles+i)->state.size), cvRound((particles+i)->state.size)) & limitRect;
				if (rect.width>rect.height)
					rect.width=rect.height;
				if (rect.width<20)
					rect.width=20;

				if ((particles+i)->state.size>1.2*rect.width)
				{
					// Particle too much out of screen, minimum likelihood
					//Lface[i]=exp(-LFACE*MAX_VJ);
					Lface[i]=exp(-1/(2*SIGMA2_FACE));
				}
				else
				{
					int cascadeno=i%MAX_HAAR_CASCADES_C;
					L = faceLikelihood(rect.x, rect.y, rect.width, &graySumC, &graySqSumC, &grayTiltSumC, mtx[cascadeno],cascadeno);
					//Lface[i]=exp(-LFACE*(MAX_VJ-L));
					Lface[i]=exp(-(MAX_VJ-L)/(2*SIGMA2_FACE*MAX_VJ));
				}
			}
			else
				Lface[i]=0;

			Rect particlerect(cvRound((particles+i)->state.x-(particles+i)->state.size/2.0),
				cvRound((particles+i)->state.y-(particles+i)->state.size/2.0),
				cvRound((particles+i)->state.size),	cvRound((particles+i)->state.size));

			// COLOR
			if ((TRACKING_CUE & CUE_COLOR)!=0)
			{
				Rect inScreen=particlerect&limitRect;
				if (inScreen.area()<0.5*particlerect.area())
				{
					// Particle too much out of screen, minimum likelihood
//					Lcolor[i]=exp(-(double)LCOLOR);
					Lcolor[i]=exp(-1/(2*SIGMA2_COLOR));
				}
				else
				{
					bool accumulate=false;
					face2rectsColor(particlerect, limitRect, r);
					if (!use1DHist)
						calcImageHist(IyCrCb,r[0],hist);
					else
						calcImageHist1D(histimg,r[0],hist);
					normalize(hist, hist, 1, 0, NORM_L1);
					for (k=1; k<4; k++)
						if (r[k].width>1 && r[k].height>1)
						{
							if (!use1DHist)
								calcImageHist(IyCrCb,r[k],hext,accumulate);
							else
								calcImageHist1D(histimg,r[k],hext,accumulate);
							accumulate=true;
						}
					if(accumulate)
					{
						normalize(hext, hext, 1, 0, NORM_L1);
						L= pow(cv::compareHist(hext,colorModel,CV_COMP_BHATTACHARYYA),2)-pow(cv::compareHist(hist,colorModel,CV_COMP_BHATTACHARYYA),2);
					}
					else
						L = 1-pow(cv::compareHist(hist,colorModel,CV_COMP_BHATTACHARYYA),2);
					if (L>MAX_COLOR) L=MAX_COLOR;
					//Lcolor[i]=exp(-LCOLOR*(1-L));
					Lcolor[i]=exp(-(MAX_COLOR-L)/(2*SIGMA2_COLOR*MAX_COLOR));
				}
			}
			else
				Lcolor[i]=0;

			//MOTION
			if ((TRACKING_CUE & CUE_MOTION)!=0)
			{
				//nkat:no need to calculate all rectangles?
				face2rectsMotion(particlerect, limitRect, r);
				if (r[0].area()<0.8*particlerect.area())
				{
					// Particle too much out of screen, minimum likelihood
					//Lmotion[i]=exp(-LMOTION*MAX_MOT);
					Lmotion[i]=exp(-1/(2*SIGMA2_MOTION));
				}
				else
				{
					L=0;
					//Calculate background motion (negative)
					for (k=1; k<4; k++)
						if (r[k].width!=0 && r[k].height!=0)
						{
							if (k==1) //Increased negative weight for top rectangle
								L+=TOP_WEIGHT_MOTION*averageInSumRect(mevSum,r[k])*r[k].height*r[k].width;
							else
								L+=averageInSumRect(mevSum,r[k])*r[k].height*r[k].width;
						}
						L/=-r[4].width*r[4].height+r[0].width*r[0].height;

						//Add foreground motion
						L+=averageInSumRect(mevSum,r[0]);
						if (L<0)
							L=0;
						else if (L>MAX_MOT)
							L=MAX_MOT;
						//Lmotion[i]=exp(-LMOTION*(MAX_MOT-L));
						Lmotion[i]=exp(-(MAX_MOT-L)/(2*SIGMA2_MOTION*MAX_MOT));
				}
			}
			else
				Lmotion[i]=0;

		}
	}
};

void measureLikelihoods(particle *particles, double *Lface, double *Lcolor, double *Lmotion, Mat colorModel, int np)
{
	if ((TRACKING_CUE & CUE_FACE)!=0)
		measureFaceLikelihood(particles,Lface, np);
	else
		memset(Lface,0, np*sizeof(double));
	if ((TRACKING_CUE & CUE_COLOR)!=0)
		measureColorLikelihood(particles,colorModel,Lcolor,np);
	else
		memset(Lcolor,0, np*sizeof(double));
	if ((TRACKING_CUE & CUE_MOTION)!=0)
		measureMotionLikelihood(particles,Lmotion,np);
	else
		memset(Lmotion,0, np*sizeof(double));}

void gaussianMeasurementObjectModel(targetStruct *target)
{
	Mat			x(target->C.cols,NP,CV_64F), xg(target->C.cols,1,CV_64F);
	particle	grid[3*3*3];
	int			i, j, idx[3*3*3], Ng=0;
	double		Lface[3*3*3], Lcol[3*3*3], Lm[3*3*3];
	double		maxFace=0, maxColor=0, maxMotion=0;
	double		mode;
	//	double		Ltest[3*3*3];
	//	static int64 serial=0, parallel=0, called=0, remainTicks=0;
	//	static Scalar sumDiff;

	//	int64 start=getTickCount();

	for (i=0;i<x.rows;i++)
		for (j=0;j<NP;j++)
			x.at<double>(i,j)=rng.gaussian((double)1);
	x=target->C*x;
	for (i=0;i<NP;i++)
	{
		State *state = &(target->particles[i].state);
		state->x+=x.at<double>(0,i);
		state->y+=x.at<double>(1,i);
		state->size+=x.at<double>(2,i);
		if (state->size<20)
			state->size=20;
		state->angle+=x.at<double>(3,i);
	}
	// Search on grid
	for (int g3=-1;g3<2;g3++)
		for (int g2=-1;g2<2;g2++)
			for (int g1=-1;g1<2;g1++)
				//	int g3=0, g2=0, g1=0;
			{
				int indx=(g3+1)*3*3+(g2+1)*3+g1+1;
				//				int indx=(g1+1)*3*3+(g2+1)*3+g3+1;
				grid[indx].state.angle=0;
				grid[indx].state.x=target->xo.x+target->xo.size*.5*g1;
				grid[indx].state.y=target->xo.y+target->xo.size*.5*g2;
				grid[indx].state.size=std::max(20.0,target->xo.size*(1+.1*g3));
			}

			static vector<Mutex *> myMutex;
			if (PAR_FOR_STRIPES!=0 && myMutex.size()==0)
			{
				myMutex.resize(MAX_HAAR_CASCADES_C);
				for (int i=0;i<MAX_HAAR_CASCADES_C;i++)
					myMutex[i]=new Mutex();
			}

			if (PAR_FOR_STRIPES==0)
				//		measureLikelihoods(&grid[13],Lface,Lcol,Lm,target->colorModel,1);
					measureLikelihoods(grid,Lface,Lcol,Lm,target->colorModel,3*3*3);
			else
				parallel_for_(cv::Range(0,3*3*3), Parallel_measureLikelihoods(grid,Lface,Lcol,Lm,target->colorModel,myMutex),PAR_FOR_STRIPES);
			//	parallel+=cv::getTickCount()-start;

			//	start=cv::getTickCount();
			//	sumDiff+=sum(abs(Mat(1,3*3*3,CV_64F,Lface)-Mat(1,3*3*3,CV_64F,Ltest)));

			//nkat: are these max or min values? probably min
			for(i=0;i<3*3*3;i++)
			{
				if ((TRACKING_CUE & CUE_FACE)!=0 && maxFace<Lface[i])
					maxFace=Lface[i];
				if ((TRACKING_CUE & CUE_COLOR)!=0 && maxColor<Lcol[i])
					maxColor=Lcol[i];
				if ((TRACKING_CUE & CUE_MOTION)!=0 && maxMotion<Lm[i])
					maxMotion=Lm[i];
			}
			//nkat: initialise Lmax
			if (target->Lmax==0)
				target->Lmax=std::max(maxFace,std::max(maxColor,maxMotion));
			maxFace/=SATFACTOR;
			maxColor/=SATFACTOR;
			maxMotion/=SATFACTOR;
			for(i=0;i<3*3*3;i++)
			{
				idx[i]=-1;
				if ((TRACKING_CUE & CUE_FACE)!=0 && Lface[i]<maxFace)
					Lface[i]=maxFace;
				if ((TRACKING_CUE & CUE_COLOR)!=0 && Lcol[i]<maxColor)
					Lcol[i]=maxColor;
				if ((TRACKING_CUE & CUE_MOTION)!=0 && Lm[i]<maxMotion)
					Lm[i]=maxMotion;

				grid[i].w=1;
				if ((TRACKING_CUE & CUE_FACE)!=0)
					grid[i].w*=Lface[i];
				if ((TRACKING_CUE & CUE_MOTION)!=0)
					grid[i].w*=Lm[i];
				if ((TRACKING_CUE & CUE_COLOR)!=0)
					grid[i].w*=Lcol[i];

				if (grid[i].w>target->Lmax)
					idx[Ng++]=i;
			}
			if (debugPF)
			{
				//		saveParticles(grid,27);
				Mat rgbTmp=Irgb.clone();
				drawParticles(rgbTmp,grid,3*3*3);
				namedWindow("grid",CV_WINDOW_NORMAL);
				imshow("grid",rgbTmp);
				//		waitKey(10);
			}
			// If points on grid with good match have been found
			//printf("%d good grid points\n",Ng);
			if (Ng>0)
			{
				// Move 25% of the particles to the good grid points
				for(i=0;i<NP;i++)
				{
					mode=rng2.uniform((double)0,(double)1);
					if (mode>.75)
					{
						State *state = &(target->particles[i].state);
						mode=rng2.uniform(0,Ng);
						State *stateG = &(grid[idx[(int)mode]].state);
						for (j=0;j<xg.rows;j++)
							xg.at<double>(j)=rng2.gaussian((double)1);
						xg=target->C*xg/4;
//						cout << "\nstateG (" << stateG->x << ", " << stateG->y << ", " << stateG->size <<") , xg: [" << xg.at<double>(0) << ", " << xg.at<double>(1) << "]";
						state->x = stateG->x + xg.at<double>(0);
						state->y = stateG->y + xg.at<double>(1);
						state->size = stateG->size + xg.at<double>(0);
						if (state->size<20)
							state->size=20;
						state->angle = stateG->angle + xg.at<double>(0);
					}
				}
//				cout << endl;
			}
			//	remainTicks+=cv::getTickCount()-start;

			//if(++called%50==0)
			//	cout << "\ngaussianMeasurement Lface-Ltest=" << sumDiff[0] <<", serial/parallel=" << (double)serial/parallel 
			//		<<", remainTicks/parallel=" << (double)remainTicks/parallel << endl;	
}


//void measureMotionLikelihood(particle *particles, double *Lmotion, int np)
//{
//	int			i, k;
//	CvScalar	m;
//	double		L, d;
//	CvRect		r[5], limitRect=cvRect(0,0,frgSum.cols,frgSum.rows);
//
//	for(i=0;i<np;i++)
//	{
//		state2rectMotion((particles+i)->state, .5 , limitRect, r);
//		if (r[0].width*r[0].height<0.8*(particles+i)->state.size*(particles+i)->state.size)
//		{
//			// Particle too much out of screen, minimum likelihood
//			Lmotion[i]=exp(-LMOTION*MAX_MOT);
//		}
//		else
//		{
//			L=0;
//			for (k=1; k<4; k++)
//				if (r[k].width!=0 && r[k].height!=0)
//				{
//					m=cvGet2D(IdSum, r[k].y+r[k].height-1, r[k].x+r[k].width-1);
//					d=m.val[0];
//					m=cvGet2D(IdSum, r[k].y, r[k].x);
//					d+=m.val[0];
//					m=cvGet2D(IdSum, r[k].y+r[k].height-1, r[k].x);
//					d-=m.val[0];
//					m=cvGet2D(IdSum, r[k].y, r[k].x+r[k].width-1);
//					d-=m.val[0];
//					L+=d;
//				}
//			L/=-r[4].width*r[4].height+r[0].width*r[0].height;
//			m=cvGet2D(IdSum, r[0].y+r[0].height-1, r[0].x+r[0].width-1);
//			d=m.val[0];
//			m=cvGet2D(IdSum, r[0].y, r[0].x);
//			d+=m.val[0];
//			m=cvGet2D(IdSum, r[0].y+r[0].height-1, r[0].x);
//			d-=m.val[0];
//			m=cvGet2D(IdSum, r[0].y, r[0].x+r[0].width-1);
//			d-=m.val[0];
//			d/=r[0].width;
//			d/=r[0].height;
//			L+=d;
//			if (L<0)
//				L=0;
//			else if (L>MAX_MOT)
//				L=MAX_MOT;
//			Lmotion[i]=exp(-LMOTION*(MAX_MOT-L));
//		}
//	}
//}

//void initColourModel(CvHistogram **colorModel)
//{
//	// Colour model
//	float	**ranges=(float **)malloc(sizeof(float *));
//	int		sizes[1];
//
//	sizes[0]=LEVELSPERCOLOUR_3;
//	ranges[0] = (float *)malloc(2*sizeof(float));
//	ranges[0][0]=0;ranges[0][1]=LEVELSPERCOLOUR_3-1;
//	*colorModel=cvCreateHist(1,sizes,CV_HIST_ARRAY,ranges,1);
//	cvClearHist(*colorModel);
//}

void particleInit(vector<faceDetection> faces, particle *particles)
{
	double			sumL=0, u;
	unsigned int	i, j, redirect[NP];

	for(i=0;i<faces.size();i++)
		sumL+=faces[i].L;
	for(i=0;i<faces.size();i++)
		faces[i].L=faces[i].L/sumL;
	for(i=0;i<NP;i++)
		(particles+i)->w=1.0f/NP;
	for(i=1;i<faces.size();i++)
		faces[i].L=faces[i-1].L+faces[i].L;
	u=rng.uniform((double)0,(double)1)/NP;
	for (i = 0; i < NP; i++)
	{
		j = 0;
		while (faces[j++].L < u) ;
		redirect[i] = --j;
		u += 1.0f / NP;
	}
	for (i = 0; i < NP; i++)
	{
		(particles+i)->w=1.0f/NP;
		(particles+i)->state.angle=0;
		(particles+i)->state.size=(faces[redirect[i]].rect.height+faces[redirect[i]].rect.width)/2.0f;
		(particles+i)->state.x=faces[redirect[i]].rect.x+faces[redirect[i]].rect.width/2.0f;
		(particles+i)->state.y=faces[redirect[i]].rect.y+faces[redirect[i]].rect.height/2.0f;
	}
}

void weightUpdate2(targetStruct *target)
{
	double	Lface[NP], Lcol[NP], Lm[NP], L[NP], Lsum=0;
	int		i;
	double	minFace=0, minColor=0, minMotion=0, Lmean[3]={0,0,0};

	//	static int64 serial=0, parallel=0, called=0, remainTicks=0;
	//	static Scalar sumDiff;
	//	double Ltest[NP];
	static vector<Mutex *> myMutex;

	int64 start = cv::getTickCount();
	particle *particles=target->particles;
	double *Lmax=&target->Lmax;

	target->LcompMax[0]=target->LcompMax[1]=target->LcompMax[2]=0;

	if (myMutex.size()==0)
	{
		myMutex.resize(MAX_HAAR_CASCADES_C);
		for (int i=0;i<MAX_HAAR_CASCADES_C;i++)
			myMutex[i]=new Mutex();
	}

	*Lmax=0;

	if (PAR_FOR_STRIPES==0)
		measureLikelihoods(particles,Lface,Lcol,Lm,target->colorModel,NP);
	else
		parallel_for_(cv::Range(0,NP), Parallel_measureLikelihoods(particles,Lface,Lcol,Lm,target->colorModel,myMutex),PAR_FOR_STRIPES);

	//	sumDiff+=sum(abs(Mat(1,NP,CV_64F,Lface)-Mat(1,NP,CV_64F,Ltest)));

	for(i=0;i<NP;i++)
	{
		if (target->LcompMax[0]<Lface[i])
			target->LcompMax[0]=Lface[i];
		if (target->LcompMax[1]<Lcol[i])
			target->LcompMax[1]=Lcol[i];
		if (target->LcompMax[2]<Lm[i])
			target->LcompMax[2]=Lm[i];
	}
	minFace=target->LcompMax[0]/SATFACTOR;
	minColor=target->LcompMax[1]/SATFACTOR;
	minMotion=target->LcompMax[2]/SATFACTOR;
	for(i=0;i<NP;i++)
	{
		L[i]=1;
		if ((TRACKING_CUE & CUE_FACE)!=0)
		{
			if (Lface[i]<minFace)
				Lface[i]=minFace;
			L[i]*=Lface[i];
		}
		if ((TRACKING_CUE & CUE_COLOR)!=0)
		{
			if (Lcol[i]<minColor)
				Lcol[i]=minColor;
			L[i]*=Lcol[i];
		}
		if ((TRACKING_CUE & CUE_MOTION)!=0)
		{
			if (Lm[i]<minMotion)
				Lm[i]=minMotion;
			L[i]*=Lm[i];
		}
		Lsum+=L[i];
		if (*Lmax<L[i])
			*Lmax=L[i];
	}
	for(i=0;i<NP;i++)
	{
		(particles+i)->w=L[i]/Lsum;
	}
	//remainTicks+=getTickCount()-start;
	//if(++called%50==0)
	//	cout << "weightUpdate2 Lface-Ltest=" << sumDiff[0] <<", serial/parallel=" << (double)serial/parallel 
	//		<<", remainTicks/parallel=" << (double)remainTicks/parallel << endl;	
}


int updatePFtracker(vector<targetStruct> & targets, int64 frameTime)
{
	Mat rgbTmp, resampleTmp;
	double	/*smallScale, largeScale, scale,*/ mode;
	int i,tgt;
	static int initCounter=0;
	static Mat prevGray;

//	cout << "Update motionEvidence" << endl;
	//Update motion evidence
	if ((TRACKING_CUE & CUE_MOTION)!=0)
	{
		if(prevGray.rows>0 && prevGray.rows==Igray.rows && prevGray.cols == Igray.cols)
			motionEvidence=0.9*frg_tracker+2*abs(prevGray-Igray);
		else
			motionEvidence=frg_tracker;
		cv::integral(motionEvidence,mevSum);
		prevGray=Igray.clone();
	}

//	cout << "Calculate images for face likelihood" << endl;
	//Calculate images for face likelihood
	if ((TRACKING_CUE & CUE_FACE)!=0)
	{
		integral(Igray,graySum,graySqsum,grayTiltSum);
		graySumC=graySum, graySqSumC=graySqsum, grayTiltSumC=grayTiltSum;
	}

	if (debugPF)
	{
		resampleTmp=Irgb.clone();
	}

	for (tgt=0;tgt<(int)targets.size();tgt++)
	{
		targetStruct* target=&targets[tgt];
		// If spread is increased, force some particles back to the state
		if (target->reinitDuration>0 && (target->Linit/target->Lmax>LARGE_LIKELIHOOD_DROP || target->spread>LARGE_SPREAD))
		{
			for (i=0;i<NP;i++)
			{
				mode=rng.uniform((double)0,(double)1);
				if (mode>.9*LARGE_SPREAD/target->spread)
				{
					target->particles[i].state.x=target->xo.x;
					target->particles[i].state.y=target->xo.y;
					target->particles[i].state.size=target->xo.size;
					target->particles[i].state.angle=target->xo.angle;
				}
			}
		}
		if (debugPF)
		{
			rgbTmp=Irgb.clone();
			drawParticles(rgbTmp,target->particles);
			namedWindow("before gaussian object model",CV_WINDOW_NORMAL);
			imshow("before gaussian object model",rgbTmp);
			//			waitKey(0);
		}
		// Apply Gaussian object model	
		//gaussianObjectModel(target);
		//gaussianUniformObjectModel(target);
		gaussianMeasurementObjectModel(target);
		if (debugPF)
		{
			rgbTmp=Irgb.clone();
			drawParticles(rgbTmp,target->particles);
			namedWindow("after gaussian object model",CV_WINDOW_NORMAL);
			imshow("after gaussian object model",rgbTmp);
			//			waitKey(0);
		}
		// Weight update
		//weightUpdate(target->particles, target->colorModel, &target->Lmax);
		weightUpdate2(target);
		if (debugPF)
		{
			//Show motionEvidence if debugging motion-only tracker
			if (TRACKING_CUE == CUE_MOTION)
				cvtColor(motionEvidence,rgbTmp,CV_GRAY2BGR);
			else
				rgbTmp=Irgb.clone();
			drawParticles(rgbTmp,target->particles);
			//drawState(rgbTmp,*target);
			namedWindow("after weight update",CV_WINDOW_NORMAL);
			imshow("after weight update",rgbTmp);
			//			waitKey(0);
		}
		if (target->reinitDuration==1)
			target->Linit=target->Lmax;
		// State estimation
		if (target->reinitDuration==0 || (target->Linit/target->Lmax<LARGE_LIKELIHOOD_DROP && target->spread<LARGE_SPREAD))
			target->xo=stateEstimation(target->particles,MMSE_STATE_ESTIMATION);
		if (debugPF)
		{
			//			rgbTmp=Irgb.clone();
			//			drawState(rgbTmp,*target);
			//			namedWindow("after state",CV_WINDOW_NORMAL);
			//			imshow("after state",rgbTmp);
			//			waitKey(0);
		}
		// Resample
		resample(target->particles);
		if (debugPF)
		{
			drawParticles(resampleTmp,target->particles);
			namedWindow("after resample",CV_WINDOW_NORMAL);
			imshow("after resample",resampleTmp);
			//waitKey(0);
		}
		target->spread=particleSpread(target->particles);



		if (target->reinitDuration>0 && (target->Linit/target->Lmax>LARGE_LIKELIHOOD_DROP || target->spread>LARGE_SPREAD) && (frameTime/1000)%2==0)
		{
			// Spread is large, so look for new initialisation
			cout << "\nRe-initialisation not available\n";
			return -1;
		}
		else if (target->spread<=.5)
			initCounter=0;
		// Object model covariance matrix
		target->C.at<double>(0,0)=target->xo.size/4;
		target->C.at<double>(1,1)=target->xo.size/4;
		target->C.at<double>(2,2)=target->xo.size/22;
		// Update the track window
		target->face.center.x=(float) target->xo.x;
		target->face.center.y=(float) target->xo.y;
		target->face.size.width=(float) target->xo.size;
		target->face.size.height=(float) target->xo.size;
		static Rect imgRect=Rect(0,0,IyCrCb.cols,IyCrCb.rows);
		target->trackWindow=target->face.boundingRect()&imgRect;
		//cout << "target-end: " << tgt << " " << target->Lmax << " " << target->LcompMax[0] << " " << target->LcompMax[1] << " "<< target->LcompMax[2] << endl;

		//Check whether the target is frontal
		if ((TRACKING_CUE & CUE_FACE)!=0)
		{
			target->isFrontal=false;
			//Skip if the face modality does not indicate a face present
			//We search the main window with weight 4 and the 4 nearest with weight 1
			//Since min likelihood in each of those is 0.1, on average min likelihood is 0.05
			static double limit=exp(-(MAX_VJ-0.05)/MAX_VJ/2/SIGMA2_FACE);
			if (target->LcompMax[0]>limit)
			{
				target->isFrontal=true;
				//Second check if motion is running, make sure we have enough foreground in the face window
				if ((TRACKING_CUE & CUE_MOTION)!=0)
				{
					double sr=averageInSumRect(mevSum,target->trackWindow);
					if (sr<175)
						target->isFrontal=false;
					//nkat: To enable when tracker uses full res
					//Rect faceRect;
					//Mat facergb=Igray(target->trackWindow);
					//detectLargestObject(facergb, fine_cascade, faceRect);
					//if (faceRect.width>0)
					//{
					//	faceRect.x-=target->trackWindow.x;
					//	faceRect.y-=target->trackWindow.y;
					//	Mat facergb=Igray(faceRect);
					//	Mat facefrg=motionEvidence(faceRect);
					//}
				}
			}

		}
		else if (target->spread<FACING_SPREAD)
			target->isFrontal=true;
		else
			target->isFrontal=false;
	}
	return 0;
}

void initPFtracker(vector<faceDetection> faces, targetStruct *target)
{
	static Rect imgRect=Rect(0,0,IyCrCb.cols,IyCrCb.rows);
	// Pepare NP particles by resampling the detected faces
	particleInit(faces,target->particles);
	// State estimation
	target->xo=stateEstimation(target->particles,MMSE_STATE_ESTIMATION);
	// Object model covariance matrix
	target->C=Mat::eye(4, 4, CV_64F);
	target->C.at<double>(0,0)=target->xo.size/4;
	target->C.at<double>(1,1)=target->xo.size/4;
	target->C.at<double>(2,2)=target->xo.size/22;
	// Spread initialisation
	target->spread=.1;
	target->reinitDuration=0;
	target->Linit=0;
	target->Lmax=0;//nkat: isn't this needed?
	target->LcompMax[0]=target->LcompMax[1]=target->LcompMax[2]=0;
	// Update the track window
	target->face.center.x=(float) target->xo.x;
	target->face.center.y=(float) target->xo.y;
	target->face.size.width=(float) target->xo.size;
	target->face.size.height=(float) target->xo.size;
	target->trackWindow=target->face.boundingRect()&imgRect;
}


void drawParticles(cv::InputOutputArray _Irgb, particle *particles, int numParticles, int dec)
{
	const Scalar YELLOW = Scalar(0,200,200);
	const Scalar GREEN = Scalar(0,255,0);
	const Scalar DARK_RED = Scalar(0,0,127);
	Scalar color;
	Mat Irgb=_Irgb.getMat();
	double maxW=particles[0].w;

	//get max weight;
	for (int i=0; i<numParticles; i++)
		if (particles[i].w>maxW)
			maxW=particles[i].w;

	//draw particles
	for(int i=0; i<numParticles; i++ )
	{
		if (particles[i].w>0.7*maxW)
			color=GREEN;
		else if (particles[i].w<0.3*maxW)
			color=DARK_RED;
		else
			color=YELLOW;
		circle( Irgb, Point(cvRound(particles[i].state.x*dec),cvRound(particles[i].state.y*dec)), 
			cvRound(particles[i].state.size*0.5*dec), color);
	}
	//draw state
	//circle( Irgb, cvPoint(cvRound(target.xo.x),cvRound(target.xo.y)), 
	//	cvRound(target.xo.size*0.5), CV_RED, 4);	
}

void drawState(cv::InputOutputArray _Irgb, targetStruct target, int dec)
{
	Mat Irgb=_Irgb.getMat();
	int thickness=2;
	Scalar color=Scalar(0,95,191); //Orange
	if (target.isFrontal)
		color=Scalar(127,0,0); // Blue
	else if (target.isGlancing)
		color=Scalar(0,127,0); // Green
	else if (target.spread>LARGE_SPREAD*0.8)
		color=Scalar(0,0,191); //Red
	circle( Irgb, Point(cvRound(target.xo.x*dec),cvRound(target.xo.y*dec)), 
		cvRound(target.xo.size*0.5*dec), color,thickness);
}

int createParticleTracker(int modalities, const char* cascadeName)
{
	if (modalities<=0 || modalities>CUE_FACECOLORMOTION)
	{
		cerr << "create PF tracker: invalid modalities " << modalities << endl;
		return -1;
	}
	TRACKING_CUE = modalities;
	if ((TRACKING_CUE & CUE_FACE)!=0)
	{
		if (cascadeName == NULL)
		{
			cerr << "create PF tracker: no cascade given for face modality\n";
			return -1;
		}
		else
			for (int i=0;i<MAX_HAAR_CASCADES_C;i++)
			{
				//		CvHaarClassifierCascade* casc=cascade[i]
				//		cvReleaseHaarClassifierCascade(&(&cascade[i]));
				haarcascades[i] = (CvHaarClassifierCascade *)cvLoad(cascadeName, 0, 0, 0);
				if (!haarcascades[i])
				{
					cerr << "ERROR: Could not load face classifier cascade: " << string(cascadeName) << endl;
					return -1;
				}
			}
	}
	return 0;
}

void createLikelihoodPlots(const targetStruct& target, int64 framenum)
{
	int NP2=101;
	int			xo, yo, so;
	vector<double>  L(NP2);
	vector<particle> probeParticles(NP2);
	char		filemode[5]="at";
//	static int  figcount=1;
	switch (framenum)
	{
	case 139:
		filemode[0]='w'; //reset the file on first open
		so=79; xo=69+so/2; yo=132+so/2;
		break;
	case 529:
		so=108; xo=235+so/2; yo=132+so/2;
		break;
	case 670:
		so=90; xo=286+so/2; yo=128+so/2;
		break;
	case 764:
		so=89; xo=269+so/2; yo=135+so/2;
		break;
	}
	for (int i=0;i<NP2;i++)
	{
		probeParticles[i].state.angle=0;
		probeParticles[i].state.size=so;//+(i-(NP-1)/2);//168;
		probeParticles[i].state.x=xo+(i-(NP2-1)/2);//292;
		probeParticles[i].state.y=yo;//219;
	}
	imwrite(format("tracked_colour%lld.png",framenum),Irgb);
	imwrite(format("motion%lld.png",framenum),motionEvidence);
	FILE *fid=fopen("../../score/FTlikelihoods.m",filemode);
//	fprintf(fid,"NP=%d;x=((0:NP-1)-(NP-1)/2);\n",NP2);
//	fprintf(fid,"frame=%lld; xo=%f; yo=%f; so=%f;\n",framenum,target.xo.x,target.xo.y,target.xo.size);
	measureMotionLikelihood(&probeParticles[0], &L[0],NP2);
    fprintf(fid,"Lm%ld=[",framenum);
	for (int i=0;i<NP2;i++)
		fprintf(fid,"%4.3g ",L[i]);
	fprintf(fid,"];\n");
	measureFaceLikelihood(&probeParticles[0], &L[0], NP2);
    fprintf(fid,"Lf%ld=[",framenum);
	for (int i=0;i<NP2;i++)
		fprintf(fid,"%4.3g ",L[i]);
	fprintf(fid,"];\n");
	measureColorLikelihood(&probeParticles[0], target.colorModel, &L[0],NP2);
    fprintf(fid,"Lc%ld=[",framenum);
	for (int i=0;i<NP2;i++)
		fprintf(fid,"%4.3g ",L[i]);
	fprintf(fid,"];\n");
    fprintf(fid,"L%ld=Lm%ld.*Lf%ld.*Lc%ld;\n",framenum,framenum,framenum,framenum);
//	fprintf(fid,"figure(%d),plot(x,Lm%lld/max(Lm%lld),'k',x,Lf%lld/max(Lf%lld)-.5,'r',x,Lc%lld/max(Lc%lld)-1,'b',x,L%lld/max(L%lld)-1.5,'g','linewidth',2),grid on\nlegend({'Motion','Face','Colour','Fused'}),xlabel('Horizontal offset'),ylabel('Relative likelihood')\n",
//		figcount++,framenum,framenum,framenum,framenum,framenum,framenum,framenum,framenum);
//	fprintf(fid,"title(sprintf('Frame %%d, X0=(%%.1f %%.1f %%.1f)', frame, xo, yo, so))\n");
	if (framenum==764)
	{
		fprintf(fid,"figure(1),semilogy(x,Lc139,'k',x,Lc529,'r',x,Lc670,'b--',x,Lc764,'g','linewidth',1.5),grid on\n");
		fprintf(fid,"legend({'Normal','Profile','Exposure change','Occlusion'}),xlabel('Horizontal offset'),ylabel('Colour matching likelihood')\n");
		fprintf(fid,"figure(2),semilogy(x,Lm139,'k',x,Lm529,'r',x,Lm670,'b--',x,Lm764,'g','linewidth',1.5),grid on\n");
		fprintf(fid,"legend({'Normal','Profile','Exposure change','Occlusion'}),xlabel('Horizontal offset'),ylabel('Foreground segmentation likelihood')");
		fprintf(fid,"figure(3),semilogy(x,Lf139,'k',x,Lf529,'r',x,Lf670,'b--',x,Lf764,'g','linewidth',1.5),grid on\n");
		fprintf(fid,"legend({'Normal','Profile','Exposure change','Occlusion'}),xlabel('Horizontal offset'),ylabel('Object presence likelihood')\n");
		fprintf(fid,"figure(4),semilogy(x,L139,'k',x,L529,'r',x,L670,'b--',x,L764,'g','linewidth',1.5),grid on\n");
		fprintf(fid,"legend({'Normal','Profile','Exposure change','Occlusion'}),xlabel('Horizontal offset'),ylabel('Fused likelihood')\n");
	}
	fclose(fid);
}
