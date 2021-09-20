#ifndef REALSENSEH_H
#define REALSENSEH_H
#include "common.h"
#include <rs.hpp>
#include <rsutil.h>
#include <hpp/rs_processing.hpp>
#include <hpp/rs_types.hpp>
#include <hpp/rs_sensor.hpp>


struct Configure
{
    double max=7000;
    double min=10;
};

struct realsense_pose
{
    rs2_vector accel_sample;
    rs2_vector gyro_sample;
};

class Observer {
public:
    //-------------------defined-----------------------------
    Mat rgb_obser =Mat::zeros(1080,1920,CV_8UC3 );
    Mat dep_obser =Mat::zeros(240 ,424 ,CV_16UC1);
    realsense_pose  pose_obser;
    Configure  conf;
    rs2::pipeline_profile selection_obser;
    int threshold_obser=1;
    char path_obser[100];
    //--------------------------------------------------------
    bool rgb_isnew =false;
    bool dep_isnew =false;
    bool pose_isnew=false;
    bool pkg_isnew =false;
    //-------------------Updat-------------------------------------
    virtual void Update_rgb (Mat ) = 0;
    virtual void Update_dep (Mat ) = 0;
    virtual void Update_pose(realsense_pose) = 0 ;
    virtual void Update_rs2 (rs2::pipeline_profile) = 0;
    virtual void Update_threshold (int) = 0;
    //---------------------Get-----------------------------------
    virtual Mat Get_rgb() = 0;
    virtual Mat Get_dep() = 0;
    virtual realsense_pose Get_pose() = 0;
    virtual rs2::pipeline_profile Get_rs2() = 0;
    virtual int Get_threshold() = 0;
};

class Subject {
public:
    virtual void Attach(Observer *) = 0;
    virtual void Detach(Observer *) = 0;
    //----------------------Notify----------------------------------
    virtual void NotifyRGB (Mat rgb_new) = 0;
    virtual void NotifyDEP (Mat dep_new) = 0;
    virtual void NotifyPOSE(realsense_pose pose_new) = 0;
    virtual void NotifyRS2 (rs2::pipeline_profile selection_new) = 0;
    virtual void NotifyTHRESHOLD (int threshold_new) = 0;
};

class ConcreteObserver : public Observer {
public:
    ConcreteObserver(Subject *pSubject) : m_pSubject(pSubject){}
    void Update_rgb(Mat value);
    void Update_dep(Mat value);
    void Update_pose(realsense_pose value);
    void Update_rs2 (rs2::pipeline_profile value);
    void Update_threshold (int value);
    Mat Get_rgb();
    Mat Get_dep();
    realsense_pose Get_pose();
    rs2::pipeline_profile Get_rs2();
    int Get_threshold();
private:
    Subject *m_pSubject;
};

class ConcreteSubject : public Subject {
public:
    void Attach(Observer *pObserver);
    void Detach(Observer *pObserver);
    void NotifyRGB(Mat rgb_new);
    void NotifyDEP (Mat dep_new);
    void NotifyPOSE(realsense_pose pose_new);
    void NotifyRS2 (rs2::pipeline_profile selection_new);
    void NotifyTHRESHOLD (int threshold_new);
    void SetRGB(Mat state);
    void SetDEP(Mat state);
    void SetPOSE(realsense_pose state);
    void SetRS2(rs2::pipeline_profile state);
    void SetTHRESHOLD(int state);
private:
    std::list<Observer *> m_ObserverList;
    Mat  rgb_iState=Mat::zeros(1080,1920,CV_8UC3 );
    Mat  dep_iState=Mat::zeros(240 ,424 ,CV_16UC1);
    realsense_pose pose_iState;
    rs2::pipeline_profile selection_iState;
    int threshold_iState=1;
};

#endif // REALSENSEH_H
