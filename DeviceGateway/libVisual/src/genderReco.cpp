#include "genderReco.hpp"
#include <iomanip>
#include <iostream>
#include <iterator>
#include <fstream>
#include <functional>
#include "tools.h"
#include <opencv2/core/internal.hpp>
#include "display.h"

using namespace std;
using namespace cv;

int minRecognitionTime = 200; //Minimum time in ms between face recognitions

int normFaceWidth=80;
CascadeClassifier gfine_cascade, gcascadeLeftEye, gcascadeRightEye;
int preprocessLeftAndRightSeparately=3;

Mat Xpr; //projected class centers
Mat W; //projection matrix (W=Wlda'*Wpca')
Mat M; //training samples mean
Mat Mlda; //centering correction for unbalanced training classes
Mat sub2class; //corresponding class for each subclass

//// convert a set into a vector
//template<typename _Tp>
//inline vector<_Tp> set2vector(const set<_Tp>& src) {
//	vector<_Tp> res;
//	res.resize(src.size());
//	int i=0;
//    for (set<int>::const_iterator it = src.begin(); it != src.end(); ++it)
//        res[i++]=*it;
//	return res;
//}

// convert a set into a mat<int>
inline Mat set2mat(const set<int>& src) {
	if (src.size()<=0)
	{
		cerr << "set2mat: Empty set given\n";
		return Mat();
	}
	Mat res=Mat(1,src.size(),CV_32SC1);
	int i=0;
    for (set<int>::const_iterator it = src.begin(); it != src.end(); ++it)
        res.at<int>(i++)=*it;
	return res;
}

// convert a mat into a set<int>
inline set<int> mat2set(const cv::Mat& src) {
	set<int> res;
	if (src.type() != CV_32SC1)
	{
		cerr << "mat2set: Only CV_32SC1 matrixes can be converted to set<int>\n";
		return res;
	}
	if (src.total()<=0)
	{
		cerr << "mat2set: Empty Mat given\n";
		return res;
	}
	for (int i=0;i<(int) src.total();i++)
		res.insert(src.at<int>(i));
	return res;
}

//Returns n unique integers in range [1, max]
set<int> getPermutation(int n, int max) {
	static bool srand_called=false;

	set<int> v;

	if (n>max)
		return v;

	//seed with local variable, thread-safe as opposed to seed using time
	if (!srand_called)
	{
		srand_called=true;
        srand((unsigned long) &v );
	}

	for (int i = 0; i < n; i++) {
		while (true) {
			int num = (int) (rand() % max + 1);
			//if random number doesnt aready exist in vector, add
			if(v.count(num) != 1) {
				v.insert(num);
				break;
			}
		}
	}
	return v;
}

void printSet(const string& name, const set<int>& myset)
{
	cout << name << ":";
	for(set<int>::iterator iter=myset.begin(); iter!=myset.end();++iter)
	{
		cout << " " << setw(2) << (*iter);
	}
	cout << endl;
}

int loadSet(const string& fileName, set<int>& maleSet, set<int>& femaleSet)
{
	//string setFilename=format("trIds-set%03d-fold%02d.xml",setNum,nfold);
	////If missing slash at end of path, add it
	//char lastChar=path[path.length()-1];
	//if (lastChar!='/' && lastChar !='\\')
	//	setFilename="/"+setFilename;

	FileStorage fs(fileName, FileStorage::READ);
	if (fs.isOpened())
	{
		try {   //Try to load ids, avoid crashing if incorrect file
			Mat tmp;
			fs["trained_male_ids"]>>tmp;
			maleSet=mat2set(tmp);
			fs["trained_female_ids"]>>tmp;
			femaleSet=mat2set(tmp);
		} catch (cv::Exception e) {
			cerr << "Could not load ids from file \"" + fileName + "\"\n";
			return -1;
		}
	}
	else
	{
		//cerr << "Could not open file \"" + fileName + "\" for reading\n";
		return -1;
	}
	return 0;
}

int saveSet(const string& fileName, const set<int>& maleSet, const set<int>& femaleSet)
{
	//string setFilename=format("trIds-set%03d-fold%02d.xml",setNum,nfold);
	////If missing slash at end of path, add it
	//char lastChar=path[path.length()-1];
	//if (lastChar!='/' && lastChar !='\\')
	//	setFilename="/"+setFilename;

	FileStorage	fs(fileName, FileStorage::WRITE);
	if (fs.isOpened())
	{
		try {   ///Try to save ids, avoid crashing if an error happens
			fs << "trained_male_ids" << set2mat(maleSet);
			fs << "trained_female_ids" << set2mat(femaleSet);
		} catch (cv::Exception e) {
			cerr << "Could not write ids to file \"" + fileName + "\"\n";
			return -1;
		}
	}
	else
	{
		cerr << "Could not open file \"" + fileName + "\" for writing\n";
		return -1;
	}
	return 0;
}

int createSets(int nfold, string targetPath)
{
	int totalMales, totalFemales;

	if (nfold<1 || nfold>100)
	{
		cout << "nfold is " << nfold << ", should be between 1 and 100\n";
		return -1;
	}
	/*TODO: Either list the contents of the folder and find the number of males/females
		or detect whether the folder contains pictures from one of the predefined
		databases and set these numbers accordingly
	*/
	if (targetPath.find("ARDatabase")>0)
	{
		//AR database
		totalMales=76;
		totalFemales=60;
	}
	else 
	{
		cout << "Currently only works with AR database, located in Resources/jpg folder";
		return -1;
	}


	//Find how many male/female IDs we need for nfold
	int maleSize=totalMales-cvRound(totalMales*nfold/100.0);
	int femaleSize=totalFemales-cvRound(totalFemales*nfold/100.0);
	int maxStep;
	if (totalMales==totalFemales) maxStep=totalMales;
	else maxStep=std::max(totalMales,totalFemales)*2;

	//If missing slash at end of path, add it
	char lastChar=targetPath[targetPath.length()-1];
	if (lastChar!='/' && lastChar !='\\')
		targetPath=targetPath+"/";

	for (int i=0;i<maxStep;i++)
	{
		int curMale=i%(totalMales), curFemale=i%(totalFemales);
		set<int> males, females;
		string setFilename=targetPath+format("set%03d-fold%02d.xml",i,nfold);
		//Skip already existing sets
		if (loadSet(setFilename,males,females)==0)
			continue;
		//create a set
		for (int j=0;j<maleSize;j++)
		{
			males.insert(++curMale);
			curMale=(curMale)%(totalMales);
		}
		for (int j=0;j<femaleSize;j++)
		{
			females.insert(++curFemale);
			curFemale=(curFemale)%(totalFemales);
		}
		cout << "Set " << setFilename << endl;
		printSet("males  ",males);
		printSet("females",females);
		cout << endl;
		if (saveSet(setFilename,males,females)<0)
			return -1;
	}
	return 0;
}

void setGenderClassifiers(const CascadeClassifier& _fine_cascade, const CascadeClassifier& _cascadeLeftEye, const CascadeClassifier& _cascadeRightEye)
{
	gfine_cascade=_fine_cascade;
	gcascadeLeftEye=_cascadeLeftEye;
	gcascadeRightEye=_cascadeRightEye;
}

void setGenderFaceWidth(int newWidth)
{
	if(newWidth>0)
		normFaceWidth=newWidth;
}

int getInfoAR(const string& filename, int& label, int &id)
{
	std::string tmp[] = {"01", "03", "05", "06", "07", "14", "16", "18", "19", "20"};
	static set<string> validFaces(tmp, tmp + sizeof(tmp) / sizeof(tmp[0]));
	int fnameSize=filename.size();
	//process only jpg/png files
	if (filename.compare(fnameSize-3,3,"jpg") != 0 && filename.compare(fnameSize-3,3,"png") != 0)
		return -1;
	//skip files not in list
	else if (validFaces.count(filename.substr(6,2))!=1)
		return -1;
	else if (filename.compare(0,2,"m-") == 0)
		label=GENDER_MALE;
	else if (filename.compare(0,2,"w-") == 0)
		label=GENDER_FEMALE;
	else
	{
		cout << "\nCouldn't infer gender for image " << filename << endl;
		return -1;
	}
//	string test=filename.substr(6,4);
//		int person_id = atoi(imgname.substr(2,3).c_str());
	id=atoi(filename.substr(2,3).c_str());
	return 0;
}


Mat loadNormImage(const string& path, const string& cropType, const string& filename, bool suppressOut=true)
{
	Mat img;
	string pngname=path+ "/../cropped/" + cropType + "/" + filename.substr(0,filename.size()-3)+"png";
	if (!suppressOut) cout << "\rReading " << path << filename;
	//First try to load the normalised img
	img=imread(pngname,CV_LOAD_IMAGE_GRAYSCALE);
	if (!img.data)
	{
		//If not found, check if an error file exists
		FileStorage fs(path+ "/../cropped/" + cropType + "/" + filename.substr(0,filename.size()-4)+"-error.xml",FileStorage::READ);
		if (fs.isOpened())
			return Mat();
		//If not found, load the original
		img=imread(path+filename);
		if (!img.data)
		{
			if (!suppressOut) cout << " could not load image\n";
			return Mat();
		}
		else
		{
			Mat preprocessedFace = getPreprocessedFace(img, normFaceWidth, gfine_cascade, gcascadeLeftEye, gcascadeRightEye, preprocessLeftAndRightSeparately);
			if (!preprocessedFace.data)
			{
				fs.open(path+ "/../cropped/" + cropType + "/" + filename.substr(0,filename.size()-4)+"-error.xml",FileStorage::WRITE);
				fs<<"eyes"<<-1;
				if (!suppressOut) cout << " could not find eyes\n";
				return Mat();
			}
			imwrite(pngname,preprocessedFace);
			img = preprocessedFace;
		}
	}
	return img;
}

Ptr<FaceRecognizer> trainGenderRec(string facerecAlgorithm, vector<string> paramnames, vector<int> paramvalues, 
	string cropType, string basePath, 
	int setNum, int nfold, set<int>& trainedMaleIds, set<int>& trainedFemaleIds, bool savemodel)
{
	vector<Mat> preprocessedFaces;
	vector<int> faceLabels;
	Ptr<FaceRecognizer> model;
	bool m_debug=true;
	int MAX_FACES=500; //Initial size
	string modelFileName,idsFileName;
	const string fileType = ".xml.gz";

	//If missing slash at end of path, add it
	char lastChar=basePath[basePath.length()-1];
	if (lastChar!='/' && lastChar !='\\')
		basePath=basePath+"/";
	string imgPath=basePath+"jpg/";
	string resultsPath=basePath+"results/";
	string setPath=resultsPath+"sets/";

	int readFaces=0;

//    cout << "Training gender recognition using the [" << facerecAlgorithm << "] algorithm ..." << endl;

    // Make sure the "contrib" module is dynamically loaded at runtime.
    // Requires OpenCV v2.4.1 or later (from June 2012), otherwise the FaceRecognizer will not compile or run!
    bool haveContribModule = initModule_contrib();
    if (!haveContribModule) {
        cerr << "ERROR: The 'contrib' module is needed for FaceRecognizer but has not been loaded into OpenCV!\n";
        return NULL;
    }

    // Use the new FaceRecognizer class in OpenCV's "contrib" module:
    // Requires OpenCV v2.4.1 or later (from June 2012), otherwise the FaceRecognizer will not compile or run!
    model = Algorithm::create<FaceRecognizer>("FaceRecognizer."+facerecAlgorithm);
    if (model.empty()) {
        cerr << "ERROR: The FaceRecognizer algorithm [" << facerecAlgorithm << "] is not available in your version of OpenCV. Please update to OpenCV v2.4.1 or newer.\n";
        return NULL;
    }

	string setName=format("set%03d-fold%02d",setNum,nfold);
	string paramStr="-";

	if(paramnames.size()!=paramvalues.size())
	{
        cerr << "ERROR: trainGenderRec called with " << paramnames.size() << " parameter names and " << paramvalues.size() 
			<< " parameter values, they should both be the same\n";
        return NULL;
	}

	//Get the available parameters for the chosen algorithm
	vector<string> params;
	model->info()->getParams(params);

	for (size_t i=0;i<paramnames.size();i++)
	{
		//Issue error if invalid parameter
		if(find(params.begin(),params.end(),paramnames[i])==params.end())
		{
			cerr << "Parameter \"" << paramnames[i] << "\" not found for algorithm " << facerecAlgorithm << endl;
			return NULL;
		}

		try	{
			model->setInt(paramnames[i],paramvalues[i]);
			paramStr=paramStr + paramnames[i] + format("%03d",paramvalues[i]) + "-";
		} catch (cv::Exception e) {
			cerr << "\nError setting param " << paramnames[i] << " to value " << paramvalues[i] << endl;
			return NULL;
		}
	}


	//if (facerecAlgorithm.compare("Eigenfaces")==0 || facerecAlgorithm.compare("Fisherfaces")==0)
	//{
	//	model->setInt("ncomponents",paramvalue);
	//	paramStr=format("-ncomp%03d-",paramvalue);
	//}
	//else if (facerecAlgorithm.compare("LBPH")==0)
	//{
	//	model->setInt("radius",paramvalue);
	//	paramStr=format("-radius%03d-",paramvalue);
	//}

	modelFileName=resultsPath+"models/"+cropType+"/"+facerecAlgorithm+paramStr+cropType+"-"+setName+fileType;
	idsFileName=setPath+setName+".xml";

	if (loadSet(idsFileName,trainedMaleIds,trainedFemaleIds)<0)
	{
		cerr << "Could not load trained IDs for set "<< setName << endl;
		return NULL;
	};

	cout << "Opening \"" + modelFileName + "\" ...";

	try {   // Surround the OpenCV calls by a try/catch block so we don't crash if model can not be loaded
		FileStorage fs(modelFileName, FileStorage::READ);
		if (fs.isOpened())
		{
			model->load(fs);
			Mat labels=model->getMat("labels");
			int numMales=countNonZero(labels==GENDER_MALE);
			int len=std::max(labels.cols,labels.rows);
			int numFemales=len-numMales;
			cout<<"\nLoaded " << len << " training vectors, " << numMales << " males, " << numFemales <<" females\n";
			return model;
		}
		else
			cout << "\nFile not found, retraining from folder " << imgPath << endl;
	}
	catch (cv::Exception e) {
		cout << "\nError loading model, retraining from folder " << imgPath << endl;
	}

	//If any error happened, retrain

	//Preallocate a large size, will be increased if needed
	preprocessedFaces.resize(MAX_FACES);
	faceLabels.resize(MAX_FACES);

	DIR *dir;
	struct dirent *ent;

	/* Open directory */
	dir = opendir (imgPath.c_str());
    if (dir == NULL) 
	{
		cerr << "Path \" " << imgPath << " not found\n";
		return NULL;
	}

	/* Read entries */
    while ((ent = readdir (dir)) != NULL)
	{
		//skip current and parent dir
		if (strcmp (ent->d_name, ".") == 0 || strcmp (ent->d_name, "..") == 0) 
			continue;

		string imgname=string(ent->d_name);
		int personID, label;
//		label=getInfoAR(imgname,label,personID);

		//Skip if no info could be found
		if (getInfoAR(imgname,label,personID)<0)
		{
//			cerr << "Invalid image name " << imgname << endl;
			continue;
		}

		//Skip male IDs that are not part of trainedMaleIds
		if (label == GENDER_MALE && trainedMaleIds.count(personID)!=1)
			continue;
		//Same with female IDs
		if (label == GENDER_FEMALE && trainedFemaleIds.count(personID)!=1)
			continue;

		Mat img=loadNormImage(imgPath,cropType,imgname);
		//Skip if normalised image can not be loaded or constructed
		//E.g. when eyes were not found
		if (!img.data)
			continue;

		//resize vectors if needed
		if(readFaces==MAX_FACES)
		{
			MAX_FACES=cvRound(MAX_FACES*1.5);
			preprocessedFaces.resize(MAX_FACES);
			faceLabels.resize(MAX_FACES);
		}
		preprocessedFaces[readFaces]=img;
		faceLabels[readFaces++]=label;
		
	}
	closedir (dir);
	//Reset to the true size
	preprocessedFaces.resize(readFaces);
	faceLabels.resize(readFaces);

	//Count number of GENDER_MALE in faceLabels
	int numMales=std::count_if(faceLabels.begin(),faceLabels.end(),bind1st(equal_to<int>(),GENDER_MALE));
	//Females are the remainder
	int numFemales=readFaces-numMales;

	cout<<"\nPreprocessed " << readFaces << " training images, " << numMales << " males, " << numFemales <<" females\n";

    // Check if there is enough data to train from. For Eigenfaces, we can learn just one person if we want, but for Fisherfaces,
    // we need atleast 2 people otherwise it will crash!
	if (facerecAlgorithm.compare("FaceRecognizer.Fisherfaces") == 0 && (numMales == 0 || numFemales == 0)) {
		cout << "Warning: Fisherfaces needs atleast 2 classes, otherwise there is nothing to differentiate! Collect more data ..." << endl;
		return NULL;
    }
    if (readFaces < 1) {
        cout << "Warning: Need some training data before it can be learnt! Collect more data ..." << endl;
        return NULL;
    }


	try	{
		// Training from the collected faces using Eigenfaces or a similar algorithm.
		model->train(preprocessedFaces,faceLabels);
	} catch (cv::Exception e) {
		cerr << "\nError training model, check that given parameters \"";
		for (size_t i=0;i<paramnames.size();i++)
		{
			if(i>0) cerr << ", ";
			cerr << paramnames[i] << "=" << paramvalues[i];
		}
		cerr << "\" are correct\n";
		return NULL;
	}


	// Check if model contains ids (AIT detectors)
	//vector<string> params;
	//model->info()->getParams(params);
	//if (std::find(params.begin(), params.end(), "trained_men_ids") != params.end() &&
	//	std::find(params.begin(), params.end(), "trained_women_ids") != params.end())

	//Save trained model
	if (savemodel)
		model->save(modelFileName);

	return model;
}

int	testGenderRec(Ptr<FaceRecognizer> model, string cropType, string imgPath, const set<int>& skipMaleIds, const set<int>& skipFemaleIds,genderStats* stats)
{
	DIR *dir;
	struct dirent *ent;

	/* Check input */
	if (model==NULL || imgPath.length()==0)
	{
		cout << "Please provide a valid FaceRecognizer and path for testing" << endl;
		return -1;
	}

	try {   // Surround the OpenCV calls by a try/catch block so we don't crash if file can not be loaded
		string name=model->name();
		vector<string> labels;
		model->info()->getParams(labels);
		name=name.substr(name.find_last_of(".")+1);//Remove the "FaceRecognizer." prefix
		bool trainedmodel=false;
		
		if (name.compare("Eigenfaces")==0 || name.compare("Fisherfaces")==0)
		{
			Mat data;
			data=model->getMat("eigenvalues");
			if (!data.empty()) trainedmodel=true;
		}
		else if (name.compare("LBPH")==0)
		{
			vector<Mat> data;
			data=model->getMatVector("histograms");
			if (!data.empty()) trainedmodel=true;
		}

		if(!trainedmodel) {
			// throw error if no data (or simply return -1?)
			string error_message = "This model is not computed yet. Did you call model::train?";
			CV_Error(CV_StsError, error_message);
			return -1;
		}
	} catch (cv::Exception e) {
		cout << "Could not get eigenvalues for model" << endl;
		return -1;
	}

	/* Open directory */
	dir = opendir (imgPath.c_str());
    if (dir == NULL) 
	{
		cerr << "Path \" " << imgPath << " not found\n";
		return -1;
	}
	cout << "Testing on " << imgPath << endl;
	int m_corr=0, f_corr=0, m_errors=0, f_errors=0, total=0;
	double m_conf_corr=0, m_conf_err=0, f_conf_corr=0, f_conf_err=0;

	/* Read entries */
    while ((ent = readdir (dir)) != NULL)
	{
		//skip current and parent dir
		if (strcmp (ent->d_name, ".") == 0 || strcmp (ent->d_name, "..") == 0) 
			continue;

		string imgname=string(ent->d_name);
		int true_label, personID;
		if (getInfoAR(imgname,true_label, personID)<0)
			continue;
		//Skip previously trained images
		if (true_label == GENDER_MALE && skipMaleIds.count(personID)==1)
			continue;
		//Same with female IDs
		if (true_label == GENDER_FEMALE && skipFemaleIds.count(personID)==1)
			continue;

		Mat img=loadNormImage(imgPath,cropType,imgname);
		if (!img.data)
			continue;
		int test_label;
		double conf;
		model->predict(img,test_label,conf);
		if (true_label == 0 && test_label == 0)
			m_corr++, m_conf_corr+=conf;
		else if (true_label == 0 && test_label == 1)
			m_errors++, m_conf_err+=conf;
		else if (true_label == 1 && test_label == 0)
			f_errors++, f_conf_err+=conf;
		else if (true_label == 1 && test_label == 1)
			f_corr++, f_conf_corr+=conf;
		else
		{
			cout << "\nUnknown label " << true_label << endl;
			continue;
		}
		total++;
	}
	cout << "Read " << total << " images (" << m_corr+m_errors << " male, "<< f_corr+f_errors <<" female)" 
		<< format(", male correct %.1f%%",100*((float)m_corr/(m_corr+m_errors))) 
		<< format(", female correct %.1f%%",100*((float)f_corr/(f_corr+f_errors))) 
		<< format(", average %.1f%%\n\n",100*((float)(m_corr+f_corr)/total)) ;

	if (stats!=NULL)
	{
		stats->m_corr=m_corr;
		stats->m_errors=m_errors;
		stats->f_corr=f_corr;
		stats->f_errors=f_errors;
	}

	return 0;
 }

int runGenderTests(int argc, char* argv[])
{
	//Parameters that need recompile to change
	string basePath="D:/git-repos/gender_reco/Resources/ARDatabase/";
	int nfold=20;
	int testWidth=64;
	string cropType=format("square%d",testWidth);

	//Default values for parameters read from command line
	string faceRecAlgorithm="Eigenfaces";
	vector<string> paramnames;
	vector<int> paramvalues;
	int step=10,start=0,end=151;

	bool parseError=false;

	//Check if face/eye detectors have been initialised
	if (gfine_cascade.empty() || gcascadeLeftEye.empty() || gcascadeRightEye.empty())
	{
		cout << "Face/Eye detection cascades have not been initialised, did you call 'setGenderClassifiers'\n";
		return -1;
	}

	//read the first param
	if (argc>1) faceRecAlgorithm=string(argv[1]);
	
	// Check that it is valid
	if (faceRecAlgorithm.compare("Eigenfaces")!=0 &&
		faceRecAlgorithm.compare("Fisherfaces")!=0 &&
		faceRecAlgorithm.compare("LBPH")!=0)
		parseError=true;


	//{
	//	defname="ncomponents";
	//	defvalue=100;
	//}
	//else if (faceRecAlgorithm.compare("Fisherfaces")==0)
	//{
	//	algorithmnames[0]="ncomponents";
	//	algorithmvalues[0]=0;
	//}
	//else if (faceRecAlgorithm.compare("LBPH")==0)
	//{
	//	algorithmnames[0]="radius";
	//	algorithmvalues[0]=1;
	//}
	//else

	int readnumbers=0;

	//read rest of the params
	for (int i=2;i<argc;i++)
	{
		//Stop on error
		if (parseError)
			break;
		if (readnumbers==0)
		{
			string input=string(argv[i]);
			size_t pos=input.find("=");
			if (pos!=-1)
			{
				string name=input.substr(0,pos);
				string value=input.substr(pos+1);
				if (name.compare("croptype")==0)
				{
					if(value.compare(0,6,"square")==0)
					{
						int num=atoi(value.substr(6).c_str());
						if (num>0)
						{
							testWidth=num;
							cropType=format("square%d",testWidth);
							cout << "changing croptype to " << cropType << endl;
						}
					}
					else
						cropType=value;
				}
				else
				{
					paramnames.push_back(name);
					paramvalues.push_back(atoi(value.c_str()));
				}
				continue;
			}
			else
			{
				 step=atoi(argv[i]);
				 readnumbers++;
			}
		}
		else if (readnumbers==1)
		{
			start=atoi(argv[i]);
			readnumbers++;
		}
		else if (readnumbers==2)
		{
			end=atoi(argv[i]);
			readnumbers++;
		}
		else //readnumbers>2, i.e. invalid input, stop parsing
			parseError=true;
	}

	//If too many arguments or invalid algorithm name
	if (parseError)
	{
		//Print usage
		cout << "Gender testing tool\n\n";
		cout << "Usage:\n" << string(argv[0]) << " [faceRecAlgorithm] [paramname=value] [step] [start] [end]\n\n";
		cout << "Parameters:\n";
		cout << "\tfaceRecAlgorithm: Eigenfaces (default), Fisherfaces or LBPH\n";
		cout << "\tparamname: The name of an algorithm parameter\n";
		cout << "\tparamvalue: The integer value of the selected algorithm parameter\n";
		cout << "\t\tPossible parameters and default values:\n";
		cout << "\t\t\tEigenfaces: 'ncomponents=100'\n";
		cout << "\t\t\tFisherfaces: 'ncomponents=0', note that any value has no actual effect\n";
		cout << "\t\t\tLBPH: 'radius=1', 'neighbors=8', 'grid_x=8', 'grid_y=8'\n";
		cout << "\tstart, end, step: For AR database we have 152 possible male/female sets,\n";
		cout << "\tuse these to control which tests will be performed, default start=0, end=151, step=10\n";

//#ifdef _DEBUG
		cout<<"Press ENTER to exit" << endl;
		getchar();
//#endif /*_DEBUG*/
		return -1;	
	}

	//Set default ncomponents for "Eigenfaces" algorithm to 100 if no such parameter given
	if (faceRecAlgorithm.compare("Eigenfaces")==0 && std::find(paramnames.begin(),paramnames.end(),"ncomponents")==paramnames.end())
	{
		paramnames.push_back("ncomponents");
		paramvalues.push_back(100);
	}
	//Set default ncomponents for "Fisherfaces" algorithm to 1
	else if (faceRecAlgorithm.compare("Fisherfaces")==0 && std::find(paramnames.begin(),paramnames.end(),"ncomponents")==paramnames.end())
	{
		paramnames.push_back("ncomponents");
		paramvalues.push_back(1);
	}
	//Set default radius for "LBPH" algorithm to 1
	else if (faceRecAlgorithm.compare("LBPH")==0 && std::find(paramnames.begin(),paramnames.end(),"radius")==paramnames.end())
	{
		if (paramnames.size()==0)
		{
			paramnames.push_back("radius");
			paramvalues.push_back(1);
		}
	}

	setGenderFaceWidth(testWidth);

	static double tickfreq = cv::getTickFrequency();
	Ptr<FaceRecognizer> model;
	genderStats stats,totalStats;
	totalStats.m_corr=totalStats.m_errors=totalStats.f_corr=totalStats.f_errors=0;
//	vector<set<int>> maleSets, femaleSets;
	createSets(nfold,basePath+"results/sets/");
	set<int> maleIds, femaleIds;
	string paramStr="-";

	cout << "Running gender tests using the [" << faceRecAlgorithm << "] algorithm ..." << endl;

	for (size_t i=0;i<paramnames.size();i++)
	{
		paramStr=paramStr+paramnames[i]+format("%03d",paramvalues[i])+"-";
	}

	string resultsname=basePath+"results/stats-"+faceRecAlgorithm+paramStr+cropType+
		format("-fold%02d",nfold)+".txt";
    fstream results(resultsname.c_str(), ios::in | ios::out | ios::app);
	if (!results.is_open())
	{
		cerr << "Can not open results file " << resultsname << endl;
		return -1;	
	}

	string line, header="run_num\ttrain_time\ttest_time\tm_corr\tf_corr\tm_errors\tf_errors";
	int readlines=0;
	//read first line
	getline(results,line);
	set<int> previousRuns;
	//If invalid header, clear the file
	if (line.compare(header)!=0)
	{
		results.close();
        results.open(resultsname.c_str(), ios::out | ios::trunc);
		results << header << endl;
		results.flush();
	}
	else //Read previous existing data
	{
		while (!results.eof())
		{
			getline(results,line);
			if (line.size()>0)
			{
				int linedata[7];
				int numsread;
				stringstream stream(line);
				//read the line data
				for(numsread=0;numsread<7;numsread++) {
				   stream >> linedata[numsread];
				   if(!stream)
					  break;
				}
				//also ignore lines with less data
				if (numsread!=7)
					continue;
				//only save run numbers if they had complete data
				previousRuns.insert(linedata[0]);
			}
		}
		results.clear(); //clear eof flag
	}
	for (int i=start;i<=end;i+=step)
	{
		//skip already completed values
		if (previousRuns.count(i)==1)
			continue;
		int64 starttime = getTickCount();
		model=trainGenderRec(faceRecAlgorithm, paramnames, paramvalues, cropType, basePath, i, nfold, maleIds, femaleIds);
		int elapsedMillis = cvRound(1000*(getTickCount()-starttime)/tickfreq);
		results << i << "\t" << elapsedMillis;
		if (model==NULL)
		{
			results << endl;
			break;
		}
		else
		{
			starttime = getTickCount();
			testGenderRec(model,cropType,basePath+"jpg/",maleIds,femaleIds,&stats);
			elapsedMillis = cvRound(1000*(getTickCount()-starttime)/tickfreq);
			results << "\t" << elapsedMillis << "\t" << stats.m_corr << "\t" << stats.f_corr
				<< "\t" << stats.m_errors << "\t" << stats.f_errors << endl;
			//fs << format("run%03d_test_time_ms",i) << elapsedMillis;
			results.flush();
			totalStats.m_corr+=stats.m_corr;
			totalStats.f_corr+=stats.f_corr;
			totalStats.m_errors+=stats.m_errors;
			totalStats.f_errors+=stats.f_errors;
			//fs << format("run%03d_m_corr",i) << stats.m_corr;
			//fs << format("run%03d_f_corr",i) << stats.f_corr;
			//fs << format("run%03d_m_errors",i) << stats.m_errors;
			//fs << format("run%03d_f_errors",i) << stats.f_errors;
		}
	}

	int total=totalStats.m_corr+totalStats.f_corr+totalStats.m_errors+totalStats.f_errors;
	if (total>0)
	{
		float total_m_corr_perc=100*((float)totalStats.m_corr/(totalStats.m_corr+totalStats.m_errors));
		float total_f_corr_perc=100*((float)totalStats.f_corr/(totalStats.f_corr+totalStats.f_errors));
		float total_avg_corr_perc=100*((float)(totalStats.m_corr+totalStats.f_corr)/total);
		cout << "\nTOTAL STATS:\n Tested " << total << " images (" << totalStats.m_corr+totalStats.m_errors << " male, "
			<< totalStats.f_corr+totalStats.f_errors <<" female), male correct  " << format("%.1f",total_m_corr_perc) 
			<< "%, female correct " << format("%.1f",total_f_corr_perc) <<"%, average "<< format("%.1f%%\n\n\n\n",total_avg_corr_perc);

		//fs << "total_m_corr" << totalStats.m_corr;
		//fs << "total_f_corr" << totalStats.f_corr;
		//fs << "total_m_errors" << totalStats.m_errors;
		//fs << "total_f_errors" << totalStats.f_errors;
		//fs << "total_m_corr_perc" << total_m_corr_perc;
		//fs << "total_f_corr_perc" << total_f_corr_perc;
		//fs << "total_avg_corr_perc" << total_avg_corr_perc;
	}
//	fs.release();
	results.close();

#ifdef _DEBUG
	cout<<"Press ENTER to exit" << endl;
	getchar();
#endif /*_DEBUG*/
	return 0;
}


void recogniseFaces(vector<targetStruct>& targets, const Mat& Igray, int64 frameTime, Ptr<FaceRecognizer> model, int decObj)
{
	bool saveFaces=false;//Whether to save the recognised faces
	int i;
	int numTargets=targets.size();
//	char buff[100];
	static Rect imgRect=Rect(0,0,Igray.cols,Igray.rows);
	//Mat debugFace1, debugFace2;
	for (i=0;i<numTargets;i++)
	{
		//Skip processing if there was a recent recognition
		if (frameTime-targets[i].recognised_time<minRecognitionTime)
			continue;
		//Skip if target is not frontal
		if (!targets[i].isFrontal)
			continue;

		// Find a face and preprocess it to have a standard size and contrast & brightness.
		Rect faceRegion=targets[i].trackWindow; // Position of detected face.
		if (decObj!=1)
		{
			faceRegion.x*=decObj;
			faceRegion.y*=decObj;
			faceRegion.width*=decObj;
			faceRegion.height*=decObj;
			faceRegion&=imgRect;
		}
		Rect origFace=faceRegion;

		//debugFace1=Igray(faceRegion);
		//Enlarge found rectangle by 40%
		faceRegion.x-=cvRound(faceRegion.width*.2);
		faceRegion.y-=cvRound(faceRegion.height*.2);
		faceRegion.width=cvRound(faceRegion.width*1.4);
		faceRegion.height=cvRound(faceRegion.height*1.4);
		faceRegion&=imgRect;
		if (faceRegion.width <=0 || faceRegion.height <=0)
			continue;
		//imshow("face search region before",Igray(faceRegion));
		//waitKey(10);
		origFace.x-=faceRegion.x;
		origFace.y-=faceRegion.y;

		Rect faceRect;  // Position of detected face.
		Rect searchedLeftEye, searchedRightEye; // top-left and top-right regions of the face, where eyes were searched.
        Point leftEye, rightEye;    // Position of the detected eyes.

		//cvtColor(Igray(faceRegion),debugFace2,CV_GRAY2BGR);

		Mat facegray=Igray(faceRegion);

        Mat preprocessedFace = getPreprocessedFace(facegray, normFaceWidth, gfine_cascade, gcascadeLeftEye, gcascadeRightEye, preprocessLeftAndRightSeparately, &faceRect, &leftEye, &rightEye, &searchedLeftEye, &searchedRightEye);

		if (!preprocessedFace.data)
			continue;

		searchedLeftEye.x+=faceRect.x;
		searchedLeftEye.y+=faceRect.y;
		searchedRightEye.x+=faceRect.x;
		searchedRightEye.y+=faceRect.y;
		leftEye.x+=faceRect.x;
		leftEye.y+=faceRect.y;
		rightEye.x+=faceRect.x;
		rightEye.y+=faceRect.y;


		Mat facecopy;
		cvtColor(facegray, facecopy, CV_GRAY2BGR);
		rectangle(facecopy,faceRect,Scalar(0,0,255));
		rectangle(facecopy,searchedLeftEye,Scalar(0,255,0));
		rectangle(facecopy,searchedRightEye,Scalar(255,0,0));
		circle(facecopy,leftEye,5,Scalar(0,255,0));
		circle(facecopy,rightEye,5,Scalar(255,0,0));
		rectangle(facecopy,origFace,Scalar(255,0,255),3);
		continue;

		//searchedLeftEye.x+=faceRect.x;searchedLeftEye.y+=faceRect.y;
		//searchedRightEye.x+=faceRect.x;searchedRightEye.y+=faceRect.y;
		//rectangle(debugFace2,searchedLeftEye,Scalar(0,0,255));
		//rectangle(debugFace2,searchedRightEye,Scalar(0,0,127));
		//circle(debugFace2,leftEye+faceRect.tl(),2,Scalar(255,0,0));
		//circle(debugFace2,rightEye+faceRect.tl(),2,Scalar(0,255,0));
		//imwrite("eyes.png",debugFace2);

		targets[i].recognised_time=frameTime;
		targets[i].latestFace=preprocessedFace;
		//targets[i].genderDecisions.push_back(model->predict(preprocessedFace));
		//Scalar sum= cv::sum(targets[i].genderDecisions);
		//targets[i].gender=(float)sum[0]/targets[i].genderDecisions.size();
		faceRect.x+=faceRegion.x;
		faceRect.y+=faceRegion.y;
		//Mat facefound;
		//cvtColor(Igray(faceRect), facefound, CV_GRAY2BGR);
		//circle(facefound,leftEye,3,cv::Scalar(255,0,0));
		//circle(facefound,rightEye,3,cv::Scalar(0,255,0));
		//imshow("face found",facefound);
		//waitKey(0);
		targets[i].trackWindow=faceRect;
		targets[i].detected_time=frameTime;
		leftEye.x+=faceRect.x;
		leftEye.y+=faceRect.y;
		rightEye.x+=faceRect.x;
		rightEye.y+=faceRect.y;
		targets[i].eyes[0]=leftEye;
		targets[i].eyes[1]=rightEye;
		if (decObj!=1)
		{
			//decimate the found faces in targets
			targets[i].trackWindow.x/=decObj;
			targets[i].trackWindow.y/=decObj;
			targets[i].trackWindow.width/=decObj;
			targets[i].trackWindow.height/=decObj;
			targets[i].eyes[0].x/=decObj;
			targets[i].eyes[0].y/=decObj;
			targets[i].eyes[1].x/=decObj;
			targets[i].eyes[1].y/=decObj;
		}
		targets[i].latestDetection=targets[i].trackWindow;
		if (saveFaces)
		{
			imwrite(format("../../results/faces/target%03ld-%08lldms.png",targets[i].target_id,frameTime),
				preprocessedFace);
		}

		leftEye.x+=faceRegion.x;
		leftEye.y+=faceRegion.y;
		rightEye.x+=faceRegion.x;
		rightEye.y+=faceRegion.y;
	}
}

void recogniseGenderAge(vector<targetStruct>& targets, const Mat& Igray, int64 frameTime, int decObj)
{
	if (Xpr.empty())
	{
		cerr << "Call loadData() first\n";
		return;
	}
	bool saveFaces=false;//Whether to save the recognised faces
	int i;
	int numTargets=targets.size();
//	char buff[100];
	static Rect imgRect=Rect(0,0,Igray.cols,Igray.rows);
	//Mat debugFace1, debugFace2;
	for (i=0;i<numTargets;i++)
	{
		//Skip processing if there was a recent recognition
		if (frameTime-targets[i].recognised_time<minRecognitionTime)
			continue;
		//Skip if target is not frontal
		if (!targets[i].isFrontal)
			continue;

		// Find a face and preprocess it to have a standard size and contrast & brightness.
		Rect faceRegion=targets[i].trackWindow; // Position of detected face.
		if (decObj!=1)
		{
			faceRegion.x*=decObj;
			faceRegion.y*=decObj;
			faceRegion.width*=decObj;
			faceRegion.height*=decObj;
			faceRegion&=imgRect;
		}
		Rect origFace=faceRegion;

		//debugFace1=Igray(faceRegion);
		//Enlarge found rectangle by 40%
		faceRegion.x-=cvRound(faceRegion.width*.2);
		faceRegion.y-=cvRound(faceRegion.height*.2);
		faceRegion.width=cvRound(faceRegion.width*1.4);
		faceRegion.height=cvRound(faceRegion.height*1.4);
		faceRegion&=imgRect;
		if (faceRegion.width <=0 || faceRegion.height <=0)
			continue;
		//imshow("face search region before",Igray(faceRegion));
		//waitKey(10);
		origFace.x-=faceRegion.x;
		origFace.y-=faceRegion.y;

		Rect faceRect;  // Position of detected face.
		Rect searchedLeftEye, searchedRightEye; // top-left and top-right regions of the face, where eyes were searched.
        Point leftEye, rightEye;    // Position of the detected eyes.

		//cvtColor(Igray(faceRegion),debugFace2,CV_GRAY2BGR);

		Mat facegray=Igray(faceRegion);

        Mat preprocessedFace = getPreprocessedFace(facegray, normFaceWidth, gfine_cascade, gcascadeLeftEye, gcascadeRightEye, preprocessLeftAndRightSeparately, &faceRect, &leftEye, &rightEye, &searchedLeftEye, &searchedRightEye);

		if (!preprocessedFace.data)
			continue;

		searchedLeftEye.x+=faceRect.x;
		searchedLeftEye.y+=faceRect.y;
		searchedRightEye.x+=faceRect.x;
		searchedRightEye.y+=faceRect.y;
		leftEye.x+=faceRect.x;
		leftEye.y+=faceRect.y;
		rightEye.x+=faceRect.x;
		rightEye.y+=faceRect.y;


		Mat facecopy;
		cvtColor(facegray, facecopy, CV_GRAY2BGR);
		rectangle(facecopy,faceRect,Scalar(0,0,255));
		rectangle(facecopy,searchedLeftEye,Scalar(0,255,0));
		rectangle(facecopy,searchedRightEye,Scalar(255,0,0));
		circle(facecopy,leftEye,5,Scalar(0,255,0));
		circle(facecopy,rightEye,5,Scalar(255,0,0));
		rectangle(facecopy,origFace,Scalar(255,0,255),3);

		//searchedLeftEye.x+=faceRect.x;searchedLeftEye.y+=faceRect.y;
		//searchedRightEye.x+=faceRect.x;searchedRightEye.y+=faceRect.y;
		//rectangle(debugFace2,searchedLeftEye,Scalar(0,0,255));
		//rectangle(debugFace2,searchedRightEye,Scalar(0,0,127));
		//circle(debugFace2,leftEye+faceRect.tl(),2,Scalar(255,0,0));
		//circle(debugFace2,rightEye+faceRect.tl(),2,Scalar(0,255,0));
		//imwrite("eyes.png",debugFace2);

		targets[i].recognised_time=frameTime;
		targets[i].latestFace=preprocessedFace;
		int gender, age;
		double genderConf, ageConf;
		faceAnalytics(preprocessedFace, &gender, &genderConf, &age, &ageConf);
		targets[i].genderCounts[gender]++;
		targets[i].genderConf[gender]+=genderConf;
		targets[i].gender=targets[i].genderConf[0]/sum(targets[i].genderConf)[0];
		targets[i].agesCount++;
		targets[i].agesConfSum+=ageConf;
		targets[i].agesWeightedSum+=ageConf*age;
		targets[i].ageEst=cvRound(targets[i].agesWeightedSum/targets[i].agesConfSum);
		targets[i].ageEstConf=targets[i].agesConfSum/targets[i].agesCount;


//		targets[i].gender=targets[i].genderConf[0]/sum(targets[i].genderConf)[0];

//		Scalar sum= cv::sum(targets[i].genderDecisions);
//		targets[i].gender=(float)sum[0]/targets[i].genderDecisions.size();
		faceRect.x+=faceRegion.x;
		faceRect.y+=faceRegion.y;
		//Mat facefound;
		//cvtColor(Igray(faceRect), facefound, CV_GRAY2BGR);
		//circle(facefound,leftEye,3,cv::Scalar(255,0,0));
		//circle(facefound,rightEye,3,cv::Scalar(0,255,0));
		//imshow("face found",facefound);
		//waitKey(0);
		targets[i].trackWindow=faceRect;
		targets[i].detected_time=frameTime;
		leftEye.x+=faceRect.x;
		leftEye.y+=faceRect.y;
		rightEye.x+=faceRect.x;
		rightEye.y+=faceRect.y;
		targets[i].eyes[0]=leftEye;
		targets[i].eyes[1]=rightEye;
		if (decObj!=1)
		{
			//decimate the found faces in targets
			targets[i].trackWindow.x/=decObj;
			targets[i].trackWindow.y/=decObj;
			targets[i].trackWindow.width/=decObj;
			targets[i].trackWindow.height/=decObj;
			targets[i].eyes[0].x/=decObj;
			targets[i].eyes[0].y/=decObj;
			targets[i].eyes[1].x/=decObj;
			targets[i].eyes[1].y/=decObj;
		}
		targets[i].latestDetection=targets[i].trackWindow;
		if (saveFaces)
		{
			imwrite(format("../../results/faces/target%03ld-%08lldms.png",targets[i].target_id,frameTime),
				preprocessedFace);
		}

		leftEye.x+=faceRegion.x;
		leftEye.y+=faceRegion.y;
		rightEye.x+=faceRegion.x;
		rightEye.y+=faceRegion.y;
	}
}

void decisions2gender(double decisions, int &gender, int &percent)
{
	const double rate=6.0;
	double x;
	if (decisions>0.5)
	{
		x=1-decisions;
		gender=1;
	}
	else
	{
		x=decisions;
		gender=0;
	}
	percent=100-cvRound(100*((exp(rate*x)-1)/(exp(rate*0.5)-1)));
}

string decisions2string(double decisions)
{
	int gender,percent;
	decisions2gender(decisions,gender,percent);
	const double rate=6.0;
	string result;
	if (gender==1)
		result="F";
	else
		result="M";
	result+=format(" %d%%",percent);
	return result;
}

Ptr<FaceRecognizer> loadGenderRec(string facerecAlgorithm, string modelFileName)
{
	Ptr<FaceRecognizer> model;
    // Make sure the "contrib" module is dynamically loaded at runtime.
    // Requires OpenCV v2.4.1 or later (from June 2012), otherwise the FaceRecognizer will not compile or run!
    bool haveContribModule = initModule_contrib();
    if (!haveContribModule) {
        cerr << "ERROR: The 'contrib' module is needed for FaceRecognizer but has not been loaded into OpenCV!\n";
        return NULL;
    }

    // Use the new FaceRecognizer class in OpenCV's "contrib" module:
    // Requires OpenCV v2.4.1 or later (from June 2012), otherwise the FaceRecognizer will not compile or run!
    model = Algorithm::create<FaceRecognizer>("FaceRecognizer."+facerecAlgorithm);
    if (model.empty()) {
        cerr << "ERROR: The FaceRecognizer algorithm [" << facerecAlgorithm << "] is not available in your version of OpenCV. Please update to OpenCV v2.4.1 or newer.\n";
        return NULL;
    }

//	cout << "Opening \"" + modelFileName + "\" ...";

	try {   // Surround the OpenCV calls by a try/catch block so we don't crash if model can not be loaded
		FileStorage fs(modelFileName, FileStorage::READ);
		if (fs.isOpened())
		{
			model->load(fs);
			Mat labels=model->getMat("labels");
			int numMales=countNonZero(labels==GENDER_MALE);
			int len=std::max(labels.cols,labels.rows);
			int numFemales=len-numMales;
			cout<<"Loaded " << len << " training vectors, " << numMales << " males, " << numFemales <<" females\n";
			return model;
		}
		else
		{
//			cout << "\nFile not found, retraining from folder " << imgPath << endl;
			return NULL;
		}
	}
	catch (cv::Exception e) {
//		cout << "\nError loading model, retraining from folder " << imgPath << endl;
		return NULL;
	}
}


// Creates and returns all normalised images in the specified path.
// Loads them if they already exist in the appropriate subfolders.
int createNormImages(std::string basePath, std::vector<cv::Mat> *images)
{
	//If missing slash at end of path, add it
	char lastChar=basePath[basePath.length()-1];
	if (lastChar!='/' && lastChar !='\\')
		basePath=basePath+"/";
	string imgPath=basePath;
	string resultsPath;
	if (preprocessLeftAndRightSeparately<=2)
		resultsPath=basePath+"norm/";
	else
		resultsPath=basePath+"norm-nofilter/";

	int readFaces=0;

	DIR *dir;
	struct dirent *ent;

	/* Open directory */
	dir = opendir (resultsPath.c_str());
    if (dir != NULL) //If norm folder already exists, load images from there
	{
		if (images==NULL)
		{
			cerr<<"createNormImages can not load images from " << resultsPath << " as it was called with NULL images vector\nIf you wish to recreate all images, please delete the above path.";
			return -1;
		}
		/* Read entries */
		while ((ent = readdir (dir)) != NULL)
		{
			//skip current and parent dir
			if (strcmp (ent->d_name, ".") == 0 || strcmp (ent->d_name, "..") == 0) 
				continue;
			Mat img=imread(resultsPath+string(ent->d_name),CV_LOAD_IMAGE_GRAYSCALE);
			if (img.data)
				images->push_back(img);
		}
		closedir(dir);
		return 0;
	}

	//Else create the norm images
	/* Open directory */
	dir = opendir (imgPath.c_str());
    if (dir == NULL) 
	{
		cerr << "Path \" " << imgPath << " not found\n";
		return -1;
	}

	if (!folderExists(resultsPath.c_str()))
		createFolder(resultsPath.c_str());

	/* Read entries */
    while ((ent = readdir (dir)) != NULL)
	{
		//skip current and parent dir
		if (strcmp (ent->d_name, ".") == 0 || strcmp (ent->d_name, "..") == 0) 
			continue;

		string imgname=string(ent->d_name);
		Mat img=imread(imgPath+imgname);
		if (!img.data)
			continue;

		Rect faceRect;  // Position of detected face.
		Rect searchedLeftEye, searchedRightEye; // top-left and top-right regions of the face, where eyes were searched.
        Point leftEye, rightEye;    // Position of the detected eyes.

		Mat facegray;
		cvtColor(img,facegray,CV_BGR2GRAY);

        Mat preprocessedFace = getPreprocessedFace(facegray, normFaceWidth, gfine_cascade, gcascadeLeftEye, gcascadeRightEye, preprocessLeftAndRightSeparately, &faceRect, &leftEye, &rightEye, &searchedLeftEye, &searchedRightEye);

		if (!preprocessedFace.data)
			continue;

		string cleanName=getName(imgname);
		imwrite(resultsPath+cleanName+string(".png"),preprocessedFace);
		if (images!=NULL) 
			images->push_back(preprocessedFace);

		//Mat facecopy;
		//cvtColor(facegray, facecopy, CV_GRAY2BGR);
		//rectangle(facecopy,faceRect,Scalar(0,0,255),2);
		//rectangle(facecopy,searchedLeftEye,Scalar(0,255,0),2);
		//rectangle(facecopy,searchedRightEye,Scalar(255,0,0),2);
		//circle(facecopy,leftEye,5,Scalar(0,255,0),2);
		//circle(facecopy,rightEye,5,Scalar(255,0,0),2);
		//namedWindow("result",CV_WINDOW_NORMAL);
		//imshow("result",facecopy);
		//waitKey();
	}
	closedir (dir);
	return 0;
}

string groupIdToName(int id)
{
	switch (id) {
		case BABY:
			return "Baby";
		case F_CHILD:
			return "Female child";
		case F_TEEN:
			return "Female teen";
		case F_YOUNG:
			return "Female young adult";
		case F_ADULT:
			return "Female adult";
		case F_SENIOR:
			return "Female senior";
		case M_CHILD:
			return "Male child";
		case M_TEEN:
			return "Male teen";
		case M_YOUNG:
			return "Male young adult";
		case M_ADULT:
			return "Male adult";
		case M_SENIOR:
			return "Male senior";
		default:
			return "ERROR";
	}
}

void initGroups(vector<string>& groupNames, vector<GenderAgeType>& groupIds)
{
	groupNames.push_back("Babies");
	groupIds.push_back(BABY);

	groupNames.push_back("Female children");
	groupIds.push_back(F_CHILD);

	groupNames.push_back("Female teens");
	groupIds.push_back(F_TEEN);

	groupNames.push_back("Female young adults");
	groupIds.push_back(F_YOUNG);

	groupNames.push_back("Female adults");
	groupIds.push_back(F_ADULT);

	groupNames.push_back("Female seniors");
	groupIds.push_back(F_SENIOR);


	groupNames.push_back("Male children");
	groupIds.push_back(M_CHILD);

	groupNames.push_back("Male teens");
	groupIds.push_back(M_TEEN);

	groupNames.push_back("Male young adults");
	groupIds.push_back(M_YOUNG);

	groupNames.push_back("Male adults");
	groupIds.push_back(M_ADULT);

	groupNames.push_back("Male seniors");
	groupIds.push_back(M_SENIOR);
}

cv::Ptr<cv::FaceRecognizer> trainAgeGender(std::string srcImgPath, std::string facerecAlgorithm, 
	std::vector<std::string> *paramnames, std::vector<int> *paramvalues, std::string *modelName)
{
	vector<string> groupNames;
	vector<GenderAgeType> groupIds;

	vector<Mat> preprocessedFaces;
	vector<int> faceLabels;
	preprocessedFaces.reserve(1500);
	faceLabels.reserve(1500);

	initGroups(groupNames,groupIds);

	//If missing slash at end of path, add it
	char lastChar=srcImgPath[srcImgPath.length()-1];
	if (lastChar!='/' && lastChar !='\\')
		srcImgPath=srcImgPath+"/";

	//Load or create the normalized images
	int numGroups=0; //Number of different loaded groups
	for (int i=groupNames.size()-1;i>=0;i--)
	{
		createNormImages(srcImgPath+groupNames[i], &preprocessedFaces);
		int numImg = preprocessedFaces.size()-faceLabels.size();
		if (numImg>0)
			numGroups++;
		cout << "Loaded " << numImg << " images of type: " << groupIdToName(groupIds[i]) << endl;
		faceLabels.insert(faceLabels.end(),preprocessedFaces.size()-faceLabels.size(),groupIds[i]);
	}


	Ptr<FaceRecognizer> model;
    // Make sure the "contrib" module is dynamically loaded at runtime.
    // Requires OpenCV v2.4.1 or later (from June 2012), otherwise the FaceRecognizer will not compile or run!
    bool haveContribModule = initModule_contrib();
    if (!haveContribModule) {
        cerr << "ERROR: The 'contrib' module is needed for FaceRecognizer but has not been loaded into OpenCV!\n";
        return NULL;
    }

    // Use the new FaceRecognizer class in OpenCV's "contrib" module:
    // Requires OpenCV v2.4.1 or later (from June 2012), otherwise the FaceRecognizer will not compile or run!
    model = Algorithm::create<FaceRecognizer>("FaceRecognizer."+facerecAlgorithm);
    if (model.empty()) {
        cerr << "ERROR: The FaceRecognizer algorithm [" << facerecAlgorithm << "] is not available in your version of OpenCV. Please update to OpenCV v2.4.1 or newer.\n";
        return NULL;
    }

	string paramStr="";

	if (paramnames!=NULL && paramvalues!=NULL && paramnames->size()==paramvalues->size())
	{
		//Get the available parameters for the chosen algorithm
		vector<string> params;
		model->info()->getParams(params);

		for (size_t i=0;i<paramnames->size();i++)
		{
			//Issue error if invalid parameter
			if(find(params.begin(),params.end(),paramnames->at(i))==params.end())
			{
				cerr << "Warning: Parameter \"" << paramnames->at(i) << "\" not found for algorithm " << facerecAlgorithm << endl;
				continue;
			}

			try	{
				model->setInt(paramnames->at(i),paramvalues->at(i));
				paramStr=paramStr + paramnames->at(i) + format("%03d",paramvalues->at(i)) + "-";
			} catch (cv::Exception e) {
				cerr << "Warning: Could not set param " << paramnames->at(i) << " to value " << paramvalues->at(i) << endl;
				continue;
			}
		}
	}
	if (modelName!=NULL)
	{
		if (paramStr.size()>0)
			*modelName=facerecAlgorithm+"-"+paramStr;
		else
			*modelName=facerecAlgorithm;
	}

    // Check if there is enough data to train from. For Eigenfaces, we can learn just one person if we want, but for Fisherfaces,
    // we need atleast 2 groups otherwise it will crash!
	if (facerecAlgorithm.compare("Fisherfaces") == 0 && numGroups < 2) {
		cout << "Warning: Fisherfaces needs atleast 2 groups, otherwise there is nothing to differentiate! Collect more data ..." << endl;
		return NULL;
    }
    if (preprocessedFaces.size() < 1) {
        cout << "Warning: Need some training data before it can be learnt! Collect more data ..." << endl;
        return NULL;
    }

	cout << "Training recognizer, please wait ...\n";
	try	{
		// Training from the collected faces using Eigenfaces or a similar algorithm.
		model->train(preprocessedFaces,faceLabels);
	} catch (cv::Exception e) {
		cerr << "\nError training model, check that given parameters \"";
		if (paramnames!=NULL)
		{
			for (size_t i=0;i<paramnames->size();i++)
			{
				if(i>0) cerr << ", ";
				cerr << paramnames->at(i) << "=" << paramvalues->at(i);
			}
		}
		cerr << "\" are correct\n";
		return NULL;
	}


	return model;

}

int testAgeGender(string imgPath, Ptr<FaceRecognizer> model)
{
	DIR *dir;
	struct dirent *ent;

	if (model==NULL || model->info()==NULL)
	{
		return -1;
	}

	//If missing slash at end of path, add it
	char lastChar=imgPath[imgPath.length()-1];
	if (lastChar!='/' && lastChar !='\\')
		imgPath=imgPath+"/";

	/* Open directory */
	dir = opendir (imgPath.c_str());
    if (dir == NULL) 
	{
		cerr << "Path \" " << imgPath << " not found\n";
		return -1;
	}

	/* Read entries */
    while ((ent = readdir (dir)) != NULL)
	{
		//skip current and parent dir
		if (strcmp (ent->d_name, ".") == 0 || strcmp (ent->d_name, "..") == 0) 
			continue;

		string imgname=string(ent->d_name);
		Mat img=imread(imgPath+imgname);
		if (!img.data)
			continue;

		Rect faceRect;  // Position of detected face.
		Rect searchedLeftEye, searchedRightEye; // top-left and top-right regions of the face, where eyes were searched.
        Point leftEye, rightEye;    // Position of the detected eyes.

		Mat facegray;
		cvtColor(img,facegray,CV_BGR2GRAY);

        Mat preprocessedFace = getPreprocessedFace(facegray, normFaceWidth, gfine_cascade, gcascadeLeftEye, gcascadeRightEye, preprocessLeftAndRightSeparately, &faceRect, &leftEye, &rightEye, &searchedLeftEye, &searchedRightEye);

		if (!preprocessedFace.data)
			continue;

		rectangle(img,faceRect,Scalar(0,0,255),2);
		rectangle(img,searchedLeftEye,Scalar(0,255,0),2);
		rectangle(img,searchedRightEye,Scalar(255,0,0),2);
		circle(img,leftEye,5,Scalar(0,255,0),2);
		circle(img,rightEye,5,Scalar(255,0,0),2);

		int res=model->predict(preprocessedFace);
		writeText(img,groupIdToName(res),Point(10,10),0.6,Scalar(255));
		namedWindow("result",CV_WINDOW_NORMAL);
		imshow("result",img);
		namedWindow("normalized",CV_WINDOW_AUTOSIZE);
		imshow("normalized",preprocessedFace);
		if (waitKey()==27)
			break;
	}
	closedir (dir);
	return 0;
}


// Reads a sequence from a FileNode::SEQ with type _Tp into a result vector.
template<typename _Tp>
inline void readFileNodeList(const FileNode& fn, vector<_Tp>& result) {
    if (fn.type() == FileNode::SEQ) {
        for (FileNodeIterator it = fn.begin(); it != fn.end();) {
            _Tp item;
            it >> item;
            result.push_back(item);
        }
    }
}

// Writes the a list of given items to a cv::FileStorage.
template<typename _Tp>
inline void writeFileNodeList(FileStorage& fs, const string& name,
                              const vector<_Tp>& items) {
    // typedefs
    typedef typename vector<_Tp>::const_iterator constVecIterator;
    // write the elements in item to fs
    fs << name << "[";
    for (constVecIterator it = items.begin(); it != items.end(); ++it) {
        fs << *it;
    }
    fs << "]";
}

static Mat asRowMatrix(InputArrayOfArrays src, int rtype, double alpha=1, double beta=0) {
    // make sure the input data is a vector of matrices or vector of vector
    if(src.kind() != _InputArray::STD_VECTOR_MAT && src.kind() != _InputArray::STD_VECTOR_VECTOR) {
        string error_message = "The data is expected as InputArray::STD_VECTOR_MAT (a std::vector<Mat>) or _InputArray::STD_VECTOR_VECTOR (a std::vector< vector<...> >).";
        CV_Error(CV_StsBadArg, error_message);
    }
    // number of samples
    size_t n = src.total();
    // return empty matrix if no matrices given
    if(n == 0)
        return Mat();
    // dimensionality of (reshaped) samples
    size_t d = src.getMat(0).total();
    // create data matrix
    Mat data((int)n, (int)d, rtype);
    // now copy data
    for(unsigned int i = 0; i < n; i++) {
        // make sure data can be reshaped, throw exception if not!
        if(src.getMat(i).total() != d) {
            string error_message = format("Wrong number of elements in matrix #%d! Expected %d was %d.", i, d, src.getMat(i).total());
            CV_Error(CV_StsBadArg, error_message);
        }
        // get a hold of the current row
        Mat xi = data.row(i);
        // make reshape happy by cloning for non-continuous matrices
        if(src.getMat(i).isContinuous()) {
            src.getMat(i).reshape(1, 1).convertTo(xi, rtype, alpha, beta);
        } else {
            src.getMat(i).clone().reshape(1, 1).convertTo(xi, rtype, alpha, beta);
        }
    }
    return data;
}

// Turk, M., and Pentland, A. "Eigenfaces for recognition.". Journal of
// Cognitive Neuroscience 3 (1991), 71-86.
class Eigenfaces2 : public FaceRecognizer
{
private:
    int _num_components;
    double _threshold;
    vector<Mat> _projections;
    Mat _labels;
    Mat _eigenvectors;
    Mat _eigenvalues;
    Mat _mean;

public:
    using FaceRecognizer::save;
    using FaceRecognizer::load;

    // Initializes an empty Eigenfaces model.
    Eigenfaces2(int num_components = 0, double threshold = DBL_MAX) :
        _num_components(num_components),
        _threshold(threshold) {}

    // Initializes and computes an Eigenfaces model with images in src and
    // corresponding labels in labels. num_components will be kept for
    // classification.
    Eigenfaces2(InputArrayOfArrays src, InputArray labels,
            int num_components = 0, double threshold = DBL_MAX) :
        _num_components(num_components),
        _threshold(threshold) {
        train(src, labels);
    }

    // Computes an Eigenfaces model with images in src and corresponding labels
    // in labels.
    void train(InputArrayOfArrays src, InputArray labels);

    // Predicts the label of a query image in src.
    int predict(InputArray src) const;

    // Predicts the label and confidence for a given sample.
    void predict(InputArray _src, int &label, double &dist) const;

    // See FaceRecognizer::load.
    void load(const FileStorage& fs);

    // See FaceRecognizer::save.
    void save(FileStorage& fs) const;

    AlgorithmInfo* info() const;
};

//------------------------------------------------------------------------------
// Eigenfaces
//------------------------------------------------------------------------------
void Eigenfaces2::train(InputArrayOfArrays _src, InputArray _local_labels) {
    if(_src.total() == 0) {
        string error_message = format("Empty training data was given. You'll need more than one sample to learn a model.");
        CV_Error(CV_StsBadArg, error_message);
    } else if(_local_labels.getMat().type() != CV_32SC1) {
        string error_message = format("Labels must be given as integer (CV_32SC1). Expected %d, but was %d.", CV_32SC1, _local_labels.type());
        CV_Error(CV_StsBadArg, error_message);
    }
    // make sure data has correct size
    if(_src.total() > 1) {
        for(int i = 1; i < static_cast<int>(_src.total()); i++) {
            if(_src.getMat(i-1).total() != _src.getMat(i).total()) {
                string error_message = format("In the Eigenfaces method all input samples (training images) must be of equal size! Expected %d pixels, but was %d pixels.", _src.getMat(i-1).total(), _src.getMat(i).total());
                CV_Error(CV_StsUnsupportedFormat, error_message);
            }
        }
    }
    // get labels
    Mat labels = _local_labels.getMat();
    // observations in row
    Mat data = asRowMatrix(_src, CV_64FC1);

    // number of samples
   int n = data.rows;
    // assert there are as much samples as labels
    if(static_cast<int>(labels.total()) != n) {
        string error_message = format("The number of samples (src) must equal the number of labels (labels)! len(src)=%d, len(labels)=%d.", n, labels.total());
        CV_Error(CV_StsBadArg, error_message);
    }
    // clear existing model data
    _labels.release();
    _projections.clear();
    // clip number of components to be valid
    if((_num_components <= 0) || (_num_components > n))
        _num_components = n;

    // perform the PCA
    PCA pca(data, Mat(), CV_PCA_DATA_AS_ROW, _num_components);
    // copy the PCA results
    _mean = pca.mean.reshape(1,1); // store the mean vector
    _eigenvalues = pca.eigenvalues.clone(); // eigenvalues by row
    transpose(pca.eigenvectors, _eigenvectors); // eigenvectors by column
    // store labels for prediction
    _labels = labels.clone();
    // save projections
    for(int sampleIdx = 0; sampleIdx < data.rows; sampleIdx++) {
        Mat p = subspaceProject(_eigenvectors, _mean, data.row(sampleIdx));
        _projections.push_back(p);
    }
}

void Eigenfaces2::predict(InputArray _src, int &minClass, double &minDist) const {
    // get data
    Mat src = _src.getMat();
    // make sure the user is passing correct data
    if(_projections.empty()) {
        // throw error if no data (or simply return -1?)
        string error_message = "This Eigenfaces model is not computed yet. Did you call Eigenfaces::train?";
        CV_Error(CV_StsError, error_message);
    } else if(_eigenvectors.rows != static_cast<int>(src.total())) {
        // check data alignment just for clearer exception messages
        string error_message = format("Wrong input image size. Reason: Training and Test images must be of equal size! Expected an image with %d elements, but got %d.", _eigenvectors.rows, src.total());
        CV_Error(CV_StsBadArg, error_message);
    }
    // project into PCA subspace
    Mat q = subspaceProject(_eigenvectors, _mean, src.reshape(1,1));
    minDist = DBL_MAX;
    minClass = -1;
    for(size_t sampleIdx = 0; sampleIdx < _projections.size(); sampleIdx++) {
        double dist = norm(_projections[sampleIdx], q, NORM_L2);
        if((dist < minDist) && (dist < _threshold)) {
            minDist = dist;
            minClass = _labels.at<int>((int)sampleIdx);
        }
    }
}

int Eigenfaces2::predict(InputArray _src) const {
    int label;
    double dummy;
    predict(_src, label, dummy);
    return label;
}

void Eigenfaces2::load(const FileStorage& fs) {
    //read matrices
    fs["num_components"] >> _num_components;
    fs["mean"] >> _mean;
    fs["eigenvalues"] >> _eigenvalues;
    fs["eigenvectors"] >> _eigenvectors;
    // read sequences
    readFileNodeList(fs["projections"], _projections);
    fs["labels"] >> _labels;
}

void Eigenfaces2::save(FileStorage& fs) const {
    // write matrices
    fs << "num_components" << _num_components;
    fs << "mean" << _mean;
    fs << "eigenvalues" << _eigenvalues;
    fs << "eigenvectors" << _eigenvectors;
    // write sequences
    writeFileNodeList(fs, "projections", _projections);
    fs << "labels" << _labels;
}

CV_INIT_ALGORITHM(Eigenfaces2, "FaceRecognizer.Eigenfaces2",
                  obj.info()->addParam(obj, "ncomponents", obj._num_components);
                  obj.info()->addParam(obj, "threshold", obj._threshold);
                  obj.info()->addParam(obj, "projections", obj._projections, true);
                  obj.info()->addParam(obj, "labels", obj._labels, true);
                  obj.info()->addParam(obj, "eigenvectors", obj._eigenvectors, true);
                  obj.info()->addParam(obj, "eigenvalues", obj._eigenvalues, true);
                  obj.info()->addParam(obj, "mean", obj._mean, true));

Ptr<FaceRecognizer> createEigenFace2Recognizer(int num_components, double threshold)
{
    return new Eigenfaces2(num_components, threshold);
}

bool initModule_contrib2()
{
    Ptr<Algorithm> efaces2 = new Eigenfaces2();
    return efaces2->info() != 0;
}

int loadData(string filename)
{
	std::ifstream ifs( filename.c_str(), ios::binary);
	if (!ifs.is_open())
		return -1;
	double buff1[2];
	int sz, i, N;
	//Read size and data of projected class centers
	ifs.rdbuf()->sgetn( (char*)buff1, sizeof( buff1 ) );
	sz=cvRound(buff1[0])*cvRound(buff1[1]);
	Xpr.create(cvRound(buff1[0]),cvRound(buff1[1]),CV_64FC1);
	ifs.rdbuf()->sgetn((char*)Xpr.data, sz*sizeof(double));
	N=cvRound(buff1[1]);
	for (i=0;i<N;i++)
		Xpr.col(i)/=norm(Xpr.col(i),NORM_L2);

	//Read size and data of projection matrix
	ifs.rdbuf()->sgetn( (char*)buff1, sizeof( buff1 ) );
	sz=cvRound(buff1[0])*cvRound(buff1[1]);
	W.create(cvRound(buff1[0]),cvRound(buff1[1]),CV_64FC1);
	ifs.rdbuf()->sgetn((char*)W.data, sz*sizeof(double));

	//Read size and data of corresponding class for each subclass
	ifs.rdbuf()->sgetn( (char*)buff1, sizeof( buff1 ) );
	sz=cvRound(buff1[0])*cvRound(buff1[1]);
	sub2class.create(cvRound(buff1[0]),cvRound(buff1[1]),CV_64FC1);
	ifs.rdbuf()->sgetn((char*)sub2class.data, sz*sizeof(double));
	sub2class.convertTo(sub2class,CV_32S);

	//Read size and data of training samples mean
	ifs.rdbuf()->sgetn( (char*)buff1, sizeof( buff1 ) );
	sz=cvRound(buff1[0])*cvRound(buff1[1]);
	M.create(cvRound(buff1[0]),cvRound(buff1[1]),CV_64FC1);
	ifs.rdbuf()->sgetn((char*)M.data, sz*sizeof(double));

	//Read size and data of centering correction for unbalanced training classes
	ifs.rdbuf()->sgetn( (char*)buff1, sizeof( buff1 ) );
	sz=cvRound(buff1[0])*cvRound(buff1[1]);
	Mlda.create(cvRound(buff1[0]),cvRound(buff1[1]),CV_64FC1);
	ifs.rdbuf()->sgetn((char*)Mlda.data, sz*sizeof(double));

//	fclose(fid);
	ifs.close();
	return 0;
}

inline int class2age(int classidx)
{
	return classidx>NUM_AGES?classidx-NUM_AGES:classidx;
}

inline int class2gender(int classidx)
{
	return classidx>NUM_AGES?2:1;
}

int faceAnalytics(const Mat& normImage, int *gender, double *genderConf, int *age, double *ageConf)
{
	double ages[] = {6, 16, 27, 50, 80};
	Mat vec, dist, idx;
	normImage.convertTo(vec,CV_64F);
	vec = vec.t();
	vec = vec.reshape(1, vec.rows*vec.cols);	
	vec=W*(vec-M)-Mlda;
	vec/=norm(vec,NORM_L2);
	vec=vec.t();
	dist=1-vec*Xpr;
	int numClasses=dist.cols;
	sortIdx(dist,idx,CV_SORT_ASCENDING);
	vector<int> classes, genderVotes, ageVotes;
	classes.resize(numClasses);
	genderVotes.resize(numClasses);
	ageVotes.resize(numClasses);
	vector<double> genderDecisions, ageDecisions;
	genderDecisions.resize(NUM_GENDERS);
	ageDecisions.resize(NUM_AGES);
	// Find gender
	for(int col=0; col<numClasses; col++)
	{
		int i=idx.at<int>(0,col);
		classes[col]=sub2class.at<int>(0, i);
		genderVotes[col]=class2gender(classes[col]);
		if (col<7)
			genderDecisions[genderVotes[col]-1]+=1/dist.at<double>(0, i);
	}
	double maxval;
	int maxidx[2];
	minMaxIdx(genderDecisions,NULL,&maxval,NULL,maxidx);
	*gender=maxidx[1];
	*genderConf=(maxval/sum(genderDecisions)[0]-1.0/NUM_GENDERS)*NUM_GENDERS/(NUM_GENDERS-1.0);
	// Find age
	for(int col=0; col<numClasses; col++)
	{
		ageVotes[col]=class2age(classes[col]);
		if (col<7)
		{
			int i=idx.at<int>(0,col);
			//Reduce weight for age decisions coming from opposite sex if gender confidence is high
			if (*genderConf>0.5 && class2gender(classes[col])!=*gender)
				ageDecisions[ageVotes[col]-1]+=0.5/dist.at<double>(0, i);
			else
				ageDecisions[ageVotes[col]-1]+=1/dist.at<double>(0, i);
		}
	}
	vector<double> sumAges;
	sumAges.resize(NUM_AGES-1);

	for (int i=0;i<NUM_AGES-1;i++)
		sumAges[i]=ageDecisions[i]+ageDecisions[i+1];
	minMaxIdx(sumAges,NULL,&maxval,NULL,maxidx);

	*age=cvRound((ages[maxidx[1]]*ageDecisions[maxidx[1]]+ages[maxidx[1]+1]*ageDecisions[maxidx[1]+1])/sumAges[maxidx[1]]);
	*ageConf=(maxval/sum(ageDecisions)[0]-1.0/NUM_AGES)*NUM_AGES/(NUM_AGES-1.0);
	return 0;
}

int testNormImages(vector<Mat>& normImages,vector<int>& genders,vector<double>& genderConfs,vector<int>& ages,vector<double>& ageConfs)
{
	size_t imsz=normImages.size();
	genders.resize(imsz);
	genderConfs.resize(imsz);
	ages.resize(imsz);
	ageConfs.resize(imsz);
	for (int img=0;img<(int)imsz;img++)
		faceAnalytics(normImages[img],&genders[img],&genderConfs[img],&ages[img],&ageConfs[img]);
	return 0;
}
