#ifndef PF_H

#define PF_H

#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif

#include <vector>
//#include <time.h>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/core/core.hpp>
#include "VJ.h"
#include "colorhist.hpp"
#include "tracker.h"

#define MMSE_STATE_ESTIMATION	1
#define MAP_STATE_ESTIMATION	2

#define SATFACTOR				1e64 //Saturate minimum likelihoods to maxL/SATFACTOR before multiplication

int createParticleTracker(int modalities, const char* cascadeName = NULL);

void resample(particle *particles);
State stateEstimation(particle *particles, int type=MMSE_STATE_ESTIMATION);
//void gaussianUniformObjectModel(particle *particles, Mat C, double spread);
//void weightUpdate(particle *particles, cv::Mat colorModel, double *Lmax);
//void weightUpdate(particle *particles, CvHistogram *colorModel, double *Lmax);
void measureFaceLikelihood(particle *particles, double *Lface, int np=NP);
void measureColorLikelihood(particle *particles, cv::Mat colorModel, double *Lcolor, int np=NP);
//void measureColorLikelihood(particle *particles, CvHistogram *colorModel, double *Lcolor, int np=NP);
void measureMotionLikelihood(particle *particles, double *Lmotion, int np=NP);
double particleSpread(particle *particles);
//void initColourModel(CvHistogram **colorModel);

void createLikelihoodPlots(const targetStruct& target, int64 framenum);

#endif