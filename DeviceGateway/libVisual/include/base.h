#ifndef BASE_H

#define BASE_H

#include <vector>
#include "VJ.h"
#include "colorhist.hpp"

int		globalInit(cv::Size imgSize);
//int		faceInit(faceDetection *faces, int64 framenum);
double	averageInRect(IplImage *Isum, CvRect r);
double	distOfRects(cv::Rect r1, cv::Rect r2);
void	mergeFaces(faceDetection *faces, int *numFaces);
std::vector<faceDetection> mergeFaces2(const std::vector<faceDetection>& faces);
void	detectEyes(std::vector<faceDetection>& faces);
//void	detectEyes(faceDetection *faces, int *numFaces);
void	printMat(CvMat *A);

#endif
