#pragma once

#include"common.h"
#include"LocTrack/track/kcftracker.h"


#define GPSGuide 1

#define ONLY_TRACK		1
#define TRACK_AND_GPS	2 
#define ONLY_GPSGuide		3
#define ONLY_IMG_LOC		4
#define ONLY_IMG_LOC_BY_TRACK		5
#define ONLY_GPSPostion		6


class LocTrack : public KCFTracker //KCFTracker是父类，LocTrack 公有继承他的爸爸
{
public:
	LocTrack(int trackModel, bool hog = true, bool fixed_window = true, bool multiscale = true, bool lab = true);
	void convertModel(int Model);

	void myinit(UAVParam uavparam, camParam camparam);
    void myupdate_1(UAVParam uavparam, camParam camparam);  //track(maybe with loc)
    void myupdate_2(UAVParam uavparam, camParam camparam);      //loc


    Mat getPos();
    Point2d getPosBack();
    Rect getroi();

private:


	void myloc();
	void myloc_back();

	void point2d_to_roi();

	UAVParam myUAVParam;
	camParam myCamParam;

	int trackModel;
	Mat myposition;
	Point2d mypositionback;
	Rect myroi;




private:	//utils
	double sind(double corner);
	double cosd(double corner);
	double atand(double y, double x);

//hfc-end

};
