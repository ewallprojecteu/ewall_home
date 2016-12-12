#ifndef COLORHIST_HPP

#define COLORHIST_HPP

#include <opencv2/imgproc/imgproc.hpp>
#include "tracker.h"

int calcFaceHist(const cv::Mat& YCrCb, cv::Rect facerect, cv::OutputArray _face_hist);
void printHist(cv::Mat hist);
void backProject(cv::InputArray _YCrCb, cv::InputArray hist, cv::OutputArray _backProjection);

void imageForHist1D(cv::InputArray _Chan1, cv::InputArray _Chan2, cv::InputArray _Chan3, cv::OutputArray _histImg);
int calcFaceHist1D(cv::InputOutputArray _histImg, cv::Rect facerect, cv::OutputArray _face_hist);
void backProject1D(cv::InputArray _histImg, cv::InputArray hist, cv::OutputArray _backProjection);

// Calculates the histogram at the specified region, set accumulate to true to avoid zeroing out the previous histogram
void calcImageHist(cv::InputArray _YCrCb, cv::Rect regionrect, cv::InputOutputArray _hist, bool accumulate=false);
void calcImageHist1D(cv::InputArray _histImg, cv::Rect regionrect, cv::InputOutputArray _hist, bool accumulate=false);

void face2rectsColor(cv::Rect faceRect, cv::Rect limit, cv::Rect out[FACE_RECTS_SIZE]);

#endif /*COLORHIST_HPP*/
