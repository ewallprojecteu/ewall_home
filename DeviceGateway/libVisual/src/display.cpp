#include "display.h"
#include "tracker.h"
#include "genderReco.hpp"

using namespace std;
using namespace cv;

bool DISPLAY_FACES = false; //Toggle showing of normalised faces at bottom

double display_fps=10;
int dispFaceWidth=20;

int genderDisplayDecisions = 5; //Min number of decisions before showing gender info


void setDisplayParams(double _capture_fps, int _faceWidth)
{
	display_fps=_capture_fps;
	dispFaceWidth=_faceWidth;
}

cv::Size writeText(cv::Mat& image, const std::string& text, cv::Point position, double fontScale, cv::Scalar color)
{
	static int fontFace = cv::FONT_HERSHEY_DUPLEX;
	static int thickness = 1;
	cv::Size textSize = cv::getTextSize(text,fontFace,fontScale,thickness+2,NULL);
	cv::Point textHeight = cv::Point(0,textSize.height);
	if (image.empty())
		return textSize;
	// Write text with outline effect
	cv::putText(image,text,position+textHeight,fontFace,fontScale,cv::Scalar(0),thickness+2,CV_AA);
	cv::putText(image,text,position+textHeight,fontFace,fontScale,color,thickness,CV_AA);
	return textSize;
}

void drawTarget(Mat& Irgb, vector<targetStruct>& targets, unsigned int targetIdx, 
	int64 frameTime /*for drawing target distance*/, int dec /*decimation of the targets*/)
{
	static int fontFace = FONT_HERSHEY_DUPLEX;
	static double fontScale = 0.5*Irgb.rows/480;
	static double fontScaleSmall = 0.6*fontScale;
	static int thickness = cvRound(Irgb.rows/1000.0);
	static int shadowThickness = cvRound(2*thickness)+2;
	static int border=20; //num of pixels between face images

	static Point lineOffset;
	static Point lineOffsetSmall;
	static Point textOffsetY(0,thickness/2+3);

	if (lineOffset.y==0)
	{
		int baseline;
		Size tmp = getTextSize("N/A",fontFace,fontScale,shadowThickness,&baseline);
		lineOffset=Point(0,tmp.height+baseline);
		tmp = getTextSize("N/A",fontFace,fontScaleSmall,shadowThickness,&baseline);
		lineOffsetSmall=Point(0,tmp.height+baseline);
	}

	if(targetIdx>targets.size())
		return;
	targetStruct curTarget=targets[targetIdx];
	RotatedRect curFace=curTarget.face;
	curFace.center*=dec;
	curFace.size.width*=dec;
	curFace.size.height*=dec;
	ellipse(Irgb, curFace, CV_GREEN, 1, CV_AA );
	string targetID=format("ID: %ld", curTarget.target_id);
	Size textSize = getTextSize(targetID,fontFace,fontScale,thickness,NULL);
	Rect BB = curFace.boundingRect();
	Point textorigin(cvRound(curFace.center.x-textSize.width/2.0), BB.y);
	writeText(Irgb,targetID,textorigin,fontScale,CV_GREEN);
	//Get the gender information
	string genderStr;
	//Show the target gender if >= 5 decisions
	int numdecisions=cvRound(sum(curTarget.genderCounts)[0]);
	if (numdecisions>=genderDisplayDecisions)
	{
		//Draw male/female bar above target
		Rect maleRect=Rect(textorigin.x,textorigin.y-cvRound(1.3*lineOffset.y),
			cvRound(textSize.width*(1-curTarget.gender)),cvRound(0.3*lineOffset.y));
		Rect femaleRect=Rect(maleRect.x+maleRect.width,maleRect.y,
			textSize.width-maleRect.width,maleRect.height);
		rectangle(Irgb,maleRect,CV_CYAN,CV_FILLED);
		rectangle(Irgb,femaleRect,CV_PINK,CV_FILLED);

		//if (curTarget.gender<0.5)
		//	genderStr="M ";
		//else
		//	genderStr="F ";
		//genderStr+=format("%.2f",curTarget.gender);
		genderStr=decisions2string(curTarget.gender) + format(" (%d)",numdecisions);
	}
	else
		genderStr="N/A"+format("%d",numdecisions);
//		Size tmp = getTextSize(genderStr,fontFace,fontScale,shadowThickness,NULL);
		


	//Write details below target
	Point beloworigin=textorigin;
	string targetData;
	int quantizer=Irgb.rows/12;
	beloworigin.y+= BB.height/2+lineOffsetSmall.y;
	beloworigin.x=beloworigin.x/quantizer*quantizer;
	beloworigin.y=beloworigin.y/quantizer*quantizer;
	targetData="gender: " + genderStr;
	writeText(Irgb,targetData,beloworigin,fontScaleSmall,CV_GREEN);
	beloworigin.y+= lineOffsetSmall.y;
	targetData=format("age: %d (%.2f)", curTarget.ageEst, curTarget.ageEstConf);
	writeText(Irgb,targetData,beloworigin,fontScaleSmall,CV_GREEN);
	beloworigin.y+= lineOffsetSmall.y;
	targetData=format("active: %5.1f sec", (frameTime-curTarget.created_time)/1000.0);
	writeText(Irgb,targetData,beloworigin,fontScaleSmall,CV_GREEN);
	beloworigin.y+= lineOffsetSmall.y;
	targetData=format("facing: %5.1f sec", curTarget.facing_duration/1000.0);
	writeText(Irgb,targetData,beloworigin,fontScaleSmall,CV_GREEN);
	beloworigin.y+= lineOffsetSmall.y;
	targetData="emotion: " + curTarget.emotion;
	writeText(Irgb,targetData,beloworigin,fontScaleSmall,CV_GREEN);
	//Draw distance for 1 second after most recent facing time
	if (frameTime-curTarget.facing_time<1000)
	{
		beloworigin.y+= lineOffsetSmall.y;
		targetData="dist: ";
		if (curTarget.facing_dist==0)
			targetData+="far";
		else if (curTarget.facing_dist==1)
			targetData+="medium";
		else
			targetData+="near";
//		targetAge+= format(" %d",cvRound(curTarget.xo.size));
		writeText(Irgb,targetData,beloworigin,fontScaleSmall,CV_GREEN);
	}

	//curTarget.
	//Limit the BB rectangle if too narrow
	//			if (BB.height>1.2*BB.width)
	//				BB.height = cvRound(1.2*BB.width);
	//			cv::rectangle(Irgb,BB,CV_RED);
	//			fprintf(fid,"%d %05.3g %05.3f %04d %04d %04d\n",frame,targets[0].Lmax,targets[0].spread,(int)targets[0].xo.x,(int)targets[0].xo.y,(int)targets[0].xo.size);
#ifdef USE_PF_TRACKER
//	drawParticles(Irgb,curTarget.particles);
	drawState(Irgb,curTarget, dec);
//	beloworigin.y-= 6*lineOffset.y;
//	writeText(Irgb,format("F:%5.3f",curTarget.LcompMax[0]),beloworigin,fontScale,CV_GREEN);
//	beloworigin.y+= lineOffset.y;
//	writeText(Irgb,format("C:%5.3f",curTarget.LcompMax[1]),beloworigin,fontScale,CV_GREEN);
//	beloworigin.y+= lineOffset.y;
//	writeText(Irgb,format("M:%5.3f",curTarget.LcompMax[2]),beloworigin,fontScale,CV_GREEN);
#endif /*USE_PF_TRACKER*/

	//Draw target faces at the bottom of the screen if they fit
	if (DISPLAY_FACES && (int)targetIdx*(dispFaceWidth+border)+dispFaceWidth<Irgb.cols && sum(curTarget.genderCounts)[0]>0)
	{
		//Start of target at the bottom of the scren
		Point targetOrigin(targetIdx*(dispFaceWidth+border),Irgb.rows-dispFaceWidth);

		Rect borderRect(targetOrigin.x,targetOrigin.y,dispFaceWidth+border,dispFaceWidth);
		Rect faceRect(targetOrigin.x+border/2,targetOrigin.y,dispFaceWidth,dispFaceWidth);

		//Normally should check that the two rects are inside Irgb, not needed as they are designed to fit

		//Create the background border
		Irgb(borderRect).setTo(Scalar(100,255,255));

		//Convert & copy the latest face image to the displayed region
		cvtColor(curTarget.latestFace, Irgb(faceRect),CV_GRAY2BGR);

		targetOrigin.x+=shadowThickness; //Move text a bit to the right to fit
		writeText(Irgb,targetID,targetOrigin-textOffsetY,fontScaleSmall,CV_GREEN);


		//Write the label
		writeText(Irgb,genderStr,targetOrigin-textOffsetY-lineOffsetSmall,fontScaleSmall,CV_GREEN);
	}

	//if (curTarget.detected_frame==framenum)
	//{
	//	cv::circle(Irgb,curTarget.eyes[0],3,CV_BLUE,thickness,CV_AA);
	//	cv::circle(Irgb,curTarget.eyes[1],3,CV_GREEN,thickness,CV_AA);
	//	imshow("Detected",Irgb);
	//	waitKey(0);
	//}
	//imwrite("test.png",Irgb);
	//			rectangle(Irgb,curTarget.trackWindow,CV_BLUE);
}

//plotType = 0, bar
//plotType = 1, line

Size drawPlot(Mat &Irgb, Rect drawRegion, vector<int> data, const char* dataFormat, 
	string title, vector<string> labels, int plotType, Scalar plotColor, Scalar textColor)
{
	if (plotType<0 || plotType>1)
		return Size(-1,-1);
	double fontScale = drawRegion.height/232.0;
	double maxData;
	minMaxLoc(data,NULL,&maxData);
    Mat tmp = Mat();
    Size sz= writeText(tmp,title,Point(),fontScale);
	if (sz.width > drawRegion.width)
	{
		fontScale *= 0.99*drawRegion.width/sz.width;
	}
    Size dataSize = writeText(tmp,format(dataFormat,maxData),Point(),fontScale);
	Size labelSize = Size(0,0);
	vector<int> labelWidths(labels.size());
	for (int i=labels.size()-1;i>=0;i--)
	{
        sz= writeText(tmp,labels[i],Point(),fontScale);
		labelWidths[i]=sz.width;
		if (sz.height>labelSize.height)
			labelSize.height=sz.height;
		if (sz.width>labelSize.width)
			labelSize.width=sz.width;
	}
	//The horizontal distance is at least as large as the text/labels, increase it even more if enough width
	int xDist=std::max(cvRound(0.99*drawRegion.width/labels.size()),
		std::max(labelSize.width,dataSize.width));

	int maxHeight = cvRound(0.96*drawRegion.height-4*dataSize.height);
	int plotHeight[2];
	Point startPoint = Point(drawRegion.x+labelSize.width/2,
		drawRegion.y+drawRegion.height-3*dataSize.height/2);

	int datalen=data.size();
	for (int i=0;i<datalen;i++)
	{
		if (maxData<=0 || data[i]<=0)
			plotHeight[i%2]=1;
		else
			plotHeight[i%2]=cvRound(maxHeight*data[i]/maxData);
		writeText(Irgb, format(dataFormat, data[i]),
			Point(startPoint.x+xDist/2-dataSize.width/2,startPoint.y-plotHeight[i%2]-3*dataSize.height/2), fontScale, plotColor);
		writeText(Irgb, labels[i], Point(startPoint.x-labelWidths[i]/2,startPoint.y), fontScale, textColor);
		rectangle(Irgb, Rect(startPoint.x-1,startPoint.y-labelSize.height,3,labelSize.height),textColor,-1);
		if (plotType==0)
			rectangle(Irgb,Rect(startPoint.x+xDist/4, startPoint.y-plotHeight[i%2], xDist/2, plotHeight[i%2]),plotColor,-1);
		else if (plotType==1 && i>0) //Draw lines after the first data point
		{
			line(Irgb,Point(startPoint.x+xDist/2,startPoint.y-plotHeight[i%2]),
				Point(startPoint.x-xDist/2,startPoint.y-plotHeight[(i-1)%2]),plotColor,labelSize.height>>3,CV_AA);
			circle(Irgb,Point(startPoint.x-xDist/2, startPoint.y-plotHeight[(i-1)%2]),labelSize.height>>2,textColor,-1);
			if (i==datalen-1) //Add the last circle
				circle(Irgb,Point(startPoint.x+xDist/2, startPoint.y-plotHeight[i%2]),labelSize.height>>2,textColor,-1);
		}
		startPoint += Point(xDist,0); //Prepare next point 
	}
	writeText(Irgb, labels[datalen], Point(startPoint.x-labelWidths[datalen]/2,startPoint.y), fontScale, textColor);
	rectangle(Irgb, Rect(startPoint.x,startPoint.y-labelSize.height,1,labelSize.height),textColor,-1);

	Size plotsz=Size(startPoint.x+labelWidths[datalen]/2-drawRegion.x,drawRegion.height);
	//Write the title centered
    sz=writeText(tmp,title,Point(),fontScale);
	writeText(Irgb, title, Point(drawRegion.x+plotsz.width/2-sz.width/2,
		drawRegion.y), fontScale, textColor);

	return plotsz;
}
