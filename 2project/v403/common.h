#ifndef COMMON_H
#define COMMON_H



#include <QCoreApplication>
#include<stdio.h>  /*标准输入输出定义*/
#include<stdlib.h>     /*标准函数库定义*/
#include<unistd.h>     /*Unix 标准函数定义*/
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>      /*文件控制定义*/
#include<termios.h>    /*PPSIX 终端控制定义*/
#include<errno.h>      /*错误号定义*/
#include<string.h>
#include <time.h>
#include <stdio.h>
#include<iostream>
#include <ctime>
#include <fstream>
#include <stdlib.h>
#include <signal.h>
#include <mutex>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <typeinfo>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include <math.h>
#include <time.h>
#include <cstdlib>
#include<thread>

#include <stdint.h>
#include <vector>
#include <math.h>
#include <fstream>
#include <QThread>

#include<iomanip>


#include <opencv.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/core/core.hpp>

using namespace std;
using namespace cv;

struct localGPSData{
    double num,time,l,m,h,r,p,y,lidar;
    // num,?,l,m,h,r,p,y,?
};

struct Bbox
{
    int bx0;
    int by0;
    int bx1;
    int by1;
    int bcl;
    int bw;
    int bh;
    float prob;
    float S;
};

struct TargetLib {
    map<int,Bbox> tagetLib;
    float power = 0.2;
};

struct portInformation
{
    //4+8+8+4+4+4+4+2+2+1+1 =42
    uint32_t aTime;
    double Lon; //l
    double Lat; //m
    float Height;
    float Roll;
    float Pitch;
    float Yaw;
    unsigned short hLidar;
    unsigned short oLidar;
    char status;
    char CRC;
};
struct portRecieveInf
{
    portInformation portInf;
    bool recieveFlg=false;
};


struct DetectData {
    Mat DetectFrame;
    vector<Bbox> detV;
    bool flashFlg=true;
    //    map<int,Bbox> detIDMap;
};

struct UAVParam
{
    double vx = 0;   double vy = 0; double vz = 0;  double up = 0;
    double ur = 0;  double uy = 0;  double gp = 0;   double gr = 0;
    double gy = 0;   double h = 0;    double fh = 0;     double l = 0;
    double m = 0;    double hLidar = 0;
};

struct camParam
{
    Rect rect;
    int xsize = 1920;	double xrisze=1;
    int ysize = 1080;  double yrisze=1;
    double f=2.81;   double d=0.003;
};

struct projectdefaultpath{
    string txtPath;
    string videoPath;
    string netPath;
};
class runsupportfunctions{
public:
    //init
    projectdefaultpath getDefaultPath(){
        return this->myPath;
    }
    void setPath(projectdefaultpath path){
        this->myPath=path;
    }

    //supportFunctions
    Mat drawVecs(Mat img,vector<Bbox> box){
        for(int i = 0;i<box.size();i++){
            rectangle(img,bbox2Rect(box[i]),Scalar(0,255,0),2);
        }
        return img;
    }
    Mat putText(Mat img,vector<Bbox> box,vector<int> id){
        for(int i = 0;i<id.size();i++){
            cv::putText(img,to_string(id[i]),Point(box[i].bx0,box[i].by0),FONT_HERSHEY_SIMPLEX,1,Scalar(0,255,0),2,8);
        }
        return img;
    }

    Mat drawLine(Mat img,vector<Point> points){
        line(img,points[0],points[1],Scalar(0,255,0));
        line(img,points[0],points[2],Scalar(0,255,0));
        line(img,points[1],points[3],Scalar(0,255,0));
        line(img,points[2],points[3],Scalar(0,255,0));
        line(img,points[0],points[4],Scalar(0,255,0));
        line(img,points[1],points[5],Scalar(0,255,0));
        line(img,points[2],points[6],Scalar(0,255,0));
        line(img,points[3],points[7],Scalar(0,255,0));

        line(img,points[4],points[5],Scalar(0,255,0));
        line(img,points[4],points[6],Scalar(0,255,0));
        line(img,points[5],points[7],Scalar(0,255,0));
        line(img,points[6],points[7],Scalar(0,255,0));
        line(img,points[4],points[7],Scalar(0,255,0));
        line(img,points[5],points[7],Scalar(0,255,0));
    }

    vector<camParam> Bbox2camParam(vector<Bbox> bbox_in){
        vector<camParam> camParams;
        for(int i=0;i<bbox_in.size();i++){
            camParam camTemp;
            camTemp.rect=bbox2Rect(bbox_in[i]);
            camParams.push_back(camTemp);
        }
        return camParams;
    }
    UAVParam localGPS2uavParam(localGPSData gps_in){
        UAVParam uavParam;
        uavParam.fh=100;
        uavParam.l=gps_in.l; uavParam.m = gps_in.m;uavParam.h=uavParam.fh+gps_in.h;
        uavParam.gr=gps_in.r; uavParam.gp=gps_in.p;uavParam.gy=gps_in.y;

        //trans
        uavParam.gp = uavParam.gp-90;
        uavParam.gy=gps_in.y;
        return uavParam;
    }
    UAVParam portInf2uavParam(portRecieveInf gps_in){
        portInformation  gps=gps_in.portInf;
        UAVParam uavParam;
        uavParam.fh=100;
        uavParam.l=gps.Lon;  uavParam.m = gps.Lat;uavParam.h=uavParam.fh+gps.Height;
        uavParam.gr=gps.Roll; uavParam.gp=gps.Pitch;uavParam.gy=gps.Yaw;

        //trans
        uavParam.gp = uavParam.gp-90;
        return uavParam;
    }
    vector<Point2d> mat2Point2d(vector<Mat> mat_in){
        vector<Point2d> vecPoints ;
        for(int i =0;i<mat_in.size();i++){
            vecPoints.push_back(Point2d(mat_in[i].at<double>(0,0),mat_in[i].at<double>(1,0)));
        }
        return  vecPoints;
    }

    Bbox updateBbox(Bbox box1, Bbox box2, double power) {
        Bbox box;
        box.bcl = box1.bcl;
        box.bx0 = std::ceil(box1.bx0 * power + box2.bx0 * (1 - power));
        box.bx1 = std::ceil(box1.bx1 * power + box2.bx1 * (1 - power));
        box.by0 = std::ceil(box1.by0 * power + box2.by0 * (1 - power));
        box.by1 = std::ceil(box1.by1 * power + box2.by1 * (1 - power));
        box.bw = box.bx1 - box.bx0;
        box.bh = box.by1 - box.by0;
        box.prob = box1.prob;
        box.S = box.bw * box.bh;
        return box;
    }

    TargetLib* updateTargetMap(TargetLib* targetAll, vector<int> vecID, vector<Bbox> bbox_in, double power = 0) {
        if(power!=0){
            targetAll->power=power;
        }
        for(int i = 0; i < vecID.size(); i++) {
            if(targetAll->tagetLib.count(vecID[i])) {
                targetAll->tagetLib[vecID[i]] =  this->updateBbox(targetAll->tagetLib[vecID[i]],bbox_in[i],targetAll->power);
            }
            else {
                targetAll->tagetLib[vecID[i]] = bbox_in[i];
            }
        }
        return targetAll;
    }


private:
    projectdefaultpath myPath {
        "/home/nvidia/VID/pkg-1662956288.txt",
        "/home/nvidia/VID/rgb-1662956288.mp4",
        "/home/nvidia/lib/yolo4_fp32.rt"
    };
    camParam myCamParam;
    inline Rect bbox2Rect(Bbox box){
        return Rect(box.bx0,box.by0,box.bw,box.bh);
    }
};



class initmodel2{
public:
private:
};


class initmodel3{
public:
private:
};


#endif // COMMON_H
