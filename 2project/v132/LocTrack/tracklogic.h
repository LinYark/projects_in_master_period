#ifndef TRACKLOGIC_H
#define TRACKLOGIC_H
#include "common.h"
#include"LocTrack/LocTrack.h"
#include"pipeline/pipelinedata.h"
#include"PartRealSense/realsensectl.h"

class tracklogic
{
public:
    tracklogic();

    //loctrack
    UAVParam myUavParam;
    camParam myCamParam;
    Rect myRoi;
    Mat myGPS;
    double resizescale = 1;
    LocTrack *myLocTracker = new LocTrack(ONLY_IMG_LOC);
    void myLocTrackInit(pipelineData* plDin);
    void myLocTrackUpdate_1(pipelineData* plDin);
    void myLocTrackUpdate_2();
    Rect getRoi();
    Mat getGPS();
    //loctrack-end

    Bbox b;
    Mat imageShow;
    Mat depImage;
    Bbox detB;
    float IOU(const ioubox& b1, const ioubox& b2);
    rs2::pipeline_profile rs2;
};

class tracklogicctl
{
public:
    void myinit(pipelineData* plDin) {
        thread t1(runTrack,plDin);
        t1.detach();
    } //在这里开的跟踪的线程
    static bool initFlg;
    static void runTrack(pipelineData* plDin);
};


#endif // TRACKLOGIC_H
