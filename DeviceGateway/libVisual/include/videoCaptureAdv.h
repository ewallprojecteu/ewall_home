#include <opencv2/highgui/highgui.hpp>

//This class extends videocapture to return correct timestanps from live cameras
namespace cv
{
class CV_EXPORTS_W VideoCaptureAdv : public VideoCapture
{
public:
    CV_WRAP VideoCaptureAdv();

	CV_WRAP virtual bool open(const string& filename);
    CV_WRAP virtual bool open(int device);

    CV_WRAP virtual bool grab();

    CV_WRAP virtual bool set(int propId, double value);
    CV_WRAP virtual double get(int propId);

private:
    int64 capTicks;    //ticks of last captured frame
	int64 startTicks;  //ticks of first captured frame
	bool isLive;       //whether we have a live camera or video
	double framerate;  //the actual camera framerate
	int64 posFrames;   //position in frames for live cameras
};

}
