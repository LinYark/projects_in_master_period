#pragma once

#include"common.h"
#include"LocTrack/track/kcftracker.h"

class location{
public:
    Mat compute(UAVParam uavparam, camParam camparam);
    Mat computeGps(UAVParam uavparam, camParam camparam);
    Mat computeBack(Mat g1_in,UAVParam uavparam) ;
    vector<Point> computeBackPoints(UAVParam curUAVInf){
        int GPSResize = 10000;
        double l = curUAVInf.l * GPSResize;   double m =  curUAVInf.m* GPSResize;   double h = curUAVInf.h;
        double l_int = ceil(l);    double m_int = ceil(m);
        vector<double> ls_int = {(l_int+2)/GPSResize,(l_int)/GPSResize,(l_int-2)/GPSResize,(l_int-4)/GPSResize};
        vector<double> ms_int = {(m_int+2)/GPSResize,(m_int)/GPSResize,(m_int-2)/GPSResize,(m_int-4)/GPSResize};
        Mat p1,p2,p3,p4,p5,p6,p7,p8;
        p1 =  (Mat_<double>(3, 1) << ls_int[0],ms_int[0],h );
        p2 =  (Mat_<double>(3, 1) << ls_int[0],ms_int[3],h );
        p3 =  (Mat_<double>(3, 1) << ls_int[3],ms_int[0],h );
        p4 =  (Mat_<double>(3, 1) << ls_int[3],ms_int[3],h );
        p5 =  (Mat_<double>(3, 1) << ls_int[1],ms_int[1],h );
        p6 =  (Mat_<double>(3, 1) << ls_int[1],ms_int[2],h );
        p7 =  (Mat_<double>(3, 1) << ls_int[2],ms_int[1],h );
        p8 =  (Mat_<double>(3, 1) << ls_int[2],ms_int[2],h );
        vector<Mat> GPSPoints = {p1,p2,p3,p4,p5,p6,p7,p8};
        vector<Point> imgPoints;
        for(int i=0;i<GPSPoints.size();i++){
            Mat pointMat = computeBack(GPSPoints[i],curUAVInf);
            Point point = Point(pointMat.at<double>(0,0), pointMat.at<double>(1,0));
            imgPoints.push_back(point);
        }

        return imgPoints;
    }

private:	//utils
    double sind(double corner);
    double cosd(double corner);
    double atand(double y, double x);

};



class trackClass: public KCFTracker
{
public:
    trackClass(bool hog = true, bool fixed_window = true, bool multiscale = true, bool lab = true);
    void init(Rect roi,Mat image ){KCFTracker::init(roi, image);}
    Rect updateWithRoiShift(Rect roi,Mat image ){
        Rect rect  =  KCFTracker::update(image, roi);
        lastroi = rect;
        return rect;
    }
    Rect updateWithOutRoiShift(Mat image ){
        Rect rect  =KCFTracker::update(image, this->lastroi);
        lastroi = rect;
        return rect;
    }
private:
    Rect lastroi;
};


class targetidmap{
public:
    void setFlashPower(int power) {this->newPower=power;}

    map<int,Point2d> getMap(){  return myID; }
    vector<int> compare(vector<Point2d> points_in){
        //prepare
        vector<Point2d> left = points_in;
        vector<Point2d> right = map2kenel(this->myID);

        //match
        vector<int> newID = matchID(left,right);

        //flash Map
        for(int i=0;i<newID.size();i++){
            int key = newID[i];
            if(this->myID.count(key)){
                 this->myID[key]=left[i]*newPower + this->myID[key]*(1-newPower);
            }
            else{
                 this->myID[key]=left[i];
            }

        }

        return newID;
    }

private:
    vector<int> matchID(vector<Point2d> left,vector<Point2d> right,double disThr_in=3){
        float disThr = disThr_in;
        vector<Point2d> LTemp = left;
        vector<Point2d> RTemp = right;
        int LSize = LTemp.size();
        int RSize = RTemp.size();

        vector<int> numL;
        for (int i = 0; i < LSize; i++) {
            numL.push_back(i+ RSize+LSize);
        }

        Mat dis = Mat(LTemp.size(), RTemp.size(), CV_32FC1, Scalar(0));
        for (int i = 0; i < LSize; i++) {
            for (int j = 0; j < RSize; j++) {
                dis.at<float>(i, j) = this->getdis(LTemp[i], RTemp[j]);
            }
        }

        double minv;	Point minl;
        for (int i = 0; i < RSize; i++) {
            minMaxLoc(dis, &minv, 0, &minl, 0);
            if (minv >= disThr) {
                break;
            }
            else
            {
                int iIndex = minl.y;
                int jIndex = minl.x;
                for (int j = 0; j < RSize; j++) {
                    dis.at<float>(iIndex, j) = disThr;
                }
                for (int k = 0; k < LSize; k++) {
                    dis.at<float>(k, jIndex) = disThr;
                }
                numL[iIndex] = jIndex;
            }
        }

        int currentMapCount = RSize;
        for(int i =0; i < LSize; i++) {
            if(numL[i]>=RSize+LSize){
                numL[i] = currentMapCount;
                currentMapCount++;
            }
        }
        return numL;
    }

    vector<Point2d> map2kenel(map<int,Point2d> tempID_in){
        map<int,Point2d> tempID = tempID_in;
        vector<Point2d> right ;
        for(auto it=tempID.begin(); it != tempID.end();it++ ){
            right.push_back(it->second);
        }
        return right;
    }

    double getdis(Point2d left,Point2d right){
        double lx = multiTimes(left.x);
        double ly = multiTimes(left.y);
        double rx = multiTimes(right.x);
        double ry = multiTimes(right.y);
        return sqrt(((lx-rx)*(lx-rx))+((ly-ry)*(ly-ry)));
    }
    double multiTimes(double x,double times=100000){
        return x*times;
    }
private:
    map<int,Point2d> myID;
    int newPower=0.2;

};




