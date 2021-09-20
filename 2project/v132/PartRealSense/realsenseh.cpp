#include "realsenseh.h"

//ConcreteObserver Class
void ConcreteObserver::Update_rgb(Mat value) {
    value.copyTo(rgb_obser);
    rgb_isnew=true;
}
void ConcreteObserver::Update_dep(Mat value) {
    value.copyTo(dep_obser);
    dep_isnew=true;
}
void ConcreteObserver::Update_pose(realsense_pose value) {
    pose_obser=value;
    pose_isnew=true;
}
void ConcreteObserver::Update_rs2 (rs2::pipeline_profile value) {
    selection_obser=value;
}
void ConcreteObserver::Update_threshold (int value) {
    threshold_obser=value;
}
Mat ConcreteObserver::Get_rgb() {
    rgb_isnew=false;
    return rgb_obser;
}
Mat ConcreteObserver::Get_dep() {
    dep_isnew=false;
    return dep_obser;
}
realsense_pose ConcreteObserver::Get_pose() {
    pose_isnew=false;
    return pose_obser;
}
rs2::pipeline_profile ConcreteObserver::Get_rs2() {
    return selection_obser;
}
int ConcreteObserver::Get_threshold() {
    return threshold_obser;
}

//ConcreteSubject Class
void ConcreteSubject::Attach(Observer *pObserver) {
    m_ObserverList.push_back(pObserver);
}
void ConcreteSubject::Detach(Observer *pObserver) {
    m_ObserverList.remove(pObserver);
}
void ConcreteSubject::NotifyRGB(Mat rgb_new) {
    SetRGB(rgb_new);
    std::list<Observer *>::iterator it = m_ObserverList.begin();
    for (;it != m_ObserverList.end();it++) {
        (*it)->Update_rgb(rgb_iState);
    }
}
void ConcreteSubject::NotifyDEP (Mat dep_new) {
    SetDEP(dep_new);
    std::list<Observer *>::iterator it = m_ObserverList.begin();
    for (;it != m_ObserverList.end();it++) {
        (*it)->Update_dep(dep_iState);
    }
}
void ConcreteSubject::NotifyPOSE(realsense_pose pose_new) {
    SetPOSE(pose_new);
    std::list<Observer *>::iterator it = m_ObserverList.begin();
    for (;it != m_ObserverList.end();it++) {
        (*it)->Update_pose(pose_iState);
    }
}
void ConcreteSubject::NotifyRS2 (rs2::pipeline_profile selection_new) {
    SetRS2(selection_new);
    std::list<Observer *>::iterator it = m_ObserverList.begin();
    for (;it != m_ObserverList.end();it++) {
        (*it)->Update_rs2(selection_iState);
    }
}
void ConcreteSubject::NotifyTHRESHOLD (int threshold_new) {
    SetTHRESHOLD(threshold_new);
    std::list<Observer *>::iterator it = m_ObserverList.begin();
    for (;it != m_ObserverList.end();it++) {
        (*it)->Update_threshold(threshold_iState);
    }
}
void ConcreteSubject::SetRGB(Mat state) {
    state.copyTo(rgb_iState);
}
void ConcreteSubject::SetDEP(Mat state) {
    state.copyTo(dep_iState);
}
void ConcreteSubject::SetPOSE(realsense_pose state) {
    pose_iState =state;
}
void ConcreteSubject::SetRS2(rs2::pipeline_profile state) {
    selection_iState =state;
}
void ConcreteSubject::SetTHRESHOLD(int state) {
    threshold_iState =state;
}
