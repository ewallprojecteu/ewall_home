#include "Communication.h"
#include "time_funcs.h"

using namespace std;

vector<targetStruct> newTargets()
{
	vector<targetStruct> targets;
	targetStruct target;
	target.target_id = 0;
	target.face = cv::RotatedRect(cv::Point2f(1,2),cv::Size(20,30),0);
	target.spread = 0;
	target.emotion = "Happy";
	target.emotionConf = 0.8;
	target.gender = 0.8;
	target.ageEst = 30;
	target.ageEstConf = 0.8;
	targets.push_back(target);

	target.target_id = 1;
	target.face = cv::RotatedRect(cv::Point2f(51,82),cv::Size(30,45),0);
	target.spread = 0;
	target.emotion = "Sad";
	target.emotionConf = 0.5;
	target.gender = 0.3;
	target.ageEst = 50;
	target.ageEstConf = 0.7;
	targets.push_back(target);

	return targets;
}

int main(int argc, char *argv[])
{
//	Communication couchdb("http://localhost:5984/face_processing/");
	Communication couchdb("localhost");
	couchdb.sendMessage(getMillis(),newTargets());
	return 0; //TODO: Currently video processing not finalized
}
