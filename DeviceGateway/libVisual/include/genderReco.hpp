//#include "recognition.h"
#include "preprocessFace.h"     // Easily preprocess face images, for face recognition.
#include "tracker.h"
#include <opencv2/contrib/contrib.hpp>

#define NOMINMAX //Fix MIN/MAX conflict for std::min,std::max http://support.microsoft.com/kb/143208
#include "dirent.h"

#define GENDER_MALE 0
#define GENDER_FEMALE 1

enum GenderAgeType
{
	BABY =0,
	F_BEGIN = 1,
	F_CHILD,
	F_TEEN,
	F_YOUNG,
	F_ADULT,
	F_SENIOR,
	F_END,
	M_BEGIN = 21,
	M_CHILD,
	M_TEEN,
	M_YOUNG,
	M_ADULT,
	M_SENIOR,
	M_END
};


typedef struct genderStats
{
	int	m_corr;
	int	m_errors;
	int	f_corr;
	int	f_errors;
}genderStats;

cv::Ptr<cv::FaceRecognizer> trainGenderRec(std::string facerecAlgorithm, std::vector<std::string> paramnames, std::vector<int> paramvalues, 
	std::string cropType, std::string basePath, int setNum, int nfold, 
	std::set<int>& trainedMaleIds, std::set<int>& trainedFemaleIds, bool savemodel=true);
int	testGenderRec(cv::Ptr<cv::FaceRecognizer> model, std::string cropType, std::string imgPath, 
	const std::set<int>& skipMaleIds=std::set<int>(), const std::set<int>& skipFemaleIds=std::set<int>(), genderStats* stats=NULL);
cv::Ptr<cv::FaceRecognizer> loadGenderRec(std::string facerecAlgorithm, std::string modelFileName);

void setGenderClassifiers(const cv::CascadeClassifier& _fine_cascade, const cv::CascadeClassifier& _cascadeLeftEye, const cv::CascadeClassifier& _cascadeRightEye);
void setGenderFaceWidth(int newWidth);
int createSets(int nfold, std::string targetPath);
int runGenderTests(int argc, char* argv[]);

//Returns n unique integers in range [1, max]
std::set<int> getPermutation(int n, int max);

void recogniseFaces(std::vector<targetStruct>& targets, const cv::Mat& Igray, int64 frameTime, cv::Ptr<cv::FaceRecognizer> model, int decObj);
void recogniseGenderAge(std::vector<targetStruct>& targets, const cv::Mat& Igray, int64 frameTime, int decObj);
std::string decisions2string(double decisions);
void decisions2gender(double decisions, int &gender, int &percent);
cv::Ptr<cv::FaceRecognizer> trainAgeGender(std::string imgPath, std::string facerecAlgorithm, 
	std::vector<std::string> *paramnames = NULL, std::vector<int> *paramvalues = NULL, std::string *modelName = NULL);
int createNormImages(std::string basePath, std::vector<cv::Mat> *images = NULL);
std::string groupIdToName(int id);
int testAgeGender(std::string imgPath, cv::Ptr<cv::FaceRecognizer> model);
int loadData(std::string filename);
int testNormImages(std::vector<cv::Mat>&normImages,std::vector<int>& genders,std::vector<double>& genderConfs,std::vector<int>& ages,std::vector<double>& ageConfs);
int faceAnalytics(const cv::Mat& normImage, int *gender, double *genderConf, int *age, double *ageConf);

#define NUM_GENDERS 2
#define NUM_AGES 5

//Test recognizer
namespace cv
{
class  EigenfacesAIT : public FaceRecognizer
{
public:
};

}
