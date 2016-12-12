#ifndef VJ_H

#define VJ_H
//#define CV_HAAR_USE_SSE

#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <vector>

// Num of old C-style Haar Cascades for speeding-up parallel processing (recommended >=4)
#define MAX_HAAR_CASCADES_C	5 

#define MAX_VJ					.14

typedef int sumtype;
typedef double sqsumtype;

typedef struct CvHidHaarFeature
{
    struct
    {
        sumtype *p0, *p1, *p2, *p3;
        float weight;
    }
    rect[CV_HAAR_FEATURE_MAX];
}
CvHidHaarFeature;

typedef struct CvHidHaarTreeNode
{
    CvHidHaarFeature feature;
    float threshold;
    int left;
    int right;
}
CvHidHaarTreeNode;

typedef struct CvHidHaarClassifier
{
    int count;
    //CvHaarFeature* orig_feature;
    CvHidHaarTreeNode* node;
    float* alpha;
}
CvHidHaarClassifier;

typedef struct CvHidHaarStageClassifier
{
    int  count;
    float threshold;
    CvHidHaarClassifier* classifier;
    int two_rects;

    struct CvHidHaarStageClassifier* next;
    struct CvHidHaarStageClassifier* child;
    struct CvHidHaarStageClassifier* parent;
}
CvHidHaarStageClassifier;

struct CvHidHaarClassifierCascade
{
    int  count;
    int  is_stump_based;
    int  has_tilted_features;
    int  is_tree;
    double inv_window_area;
    CvMat sum, sqsum, tilted;
    CvHidHaarStageClassifier* stage_classifier;
    sqsumtype *pq0, *pq1, *pq2, *pq3;
    sumtype *p0, *p1, *p2, *p3;

    void** ipp_stages;
};

#define calc_sum(rect,offset) \
    ((rect).p0[offset] - (rect).p1[offset] - (rect).p2[offset] + (rect).p3[offset])

double aitRunHaarClassifierCascade( const CvHaarClassifierCascade* _cascade, CvPoint pt, int start_stage=0 );

typedef struct faceDetection
{
	CvRect	rect;
	CvPoint	eyeL;
	CvPoint	eyeR;
	double	L;
}faceDetection;

inline double averageInSumRect(const cv::Mat& Isum, cv::Rect r)
{
	int			d;
	int			x1=r.x, y1=r.y, x2=r.x+r.width-1, y2=r.y+r.height-1;

	d=Isum.at<int>(y2,x2);
	d+=Isum.at<int>(y1,x1);
	d-=Isum.at<int>(y2,x1);
	d-=Isum.at<int>(y1,x2);
	return (double)d/(r.width*r.height);
}

int initCascades(const std::string& faceFile, const std::string& leftEyeFile, const std::string& rightEyeFile);
std::vector<faceDetection> detailedFaceDetect(int64 framenum, const cv::Mat& Igray, const cv::Mat& frgFull);

int initSearchLines(const std::string& cascadeFile);
void drawSearchLines(cv::Mat& Irgb, int64 framenum, cv::Scalar color);
int quickFaceSearch(std::vector<faceDetection>& faces, int64 framenum, const cv::Mat& frgFull);


//In OpenCV 2.3.0 up to 2.4.7(current latest) getOriginalWindowSize() is broken for old classifiers
//This class fixes the issue
namespace cv
{
class CV_EXPORTS_W CascadeClassifierFix : public CascadeClassifier
{
public:
	Size getOriginalWindowSize() const
	{
		if (isOldFormatCascade())
			return oldCascade->orig_window_size;
		else
			return data.origWinSize;
	}
};

}

#endif /*VJ_H*/
