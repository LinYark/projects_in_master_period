#include "realsensectl.h"

//Realsensectl class
realsensectl::realsensectl() {}
bool realsensectl::initFlg = false;
void realsensectl::attachObserverDeal() {
    pSubject->Attach(pObserver_deal);
}
void realsensectl::myinit(){
    attachObserverDeal();
    thread t1(realsense,pSubject); //realsense
    t1.detach();
} //开线程，分离线程
void realsensectl::realsense(ConcreteSubject *pSubject) {
    cout<<"load realsense ..."<<endl;
    rs2::pipeline pipe;
    rs2::config cfg;
    cfg.enable_stream(RS2_STREAM_COLOR,  1920, 1080, RS2_FORMAT_BGR8, 15);
    cfg.enable_stream(RS2_STREAM_DEPTH, 424, 240, RS2_FORMAT_Z16, 15);
    rs2::pipeline_profile selection = pipe.start(cfg);
    pSubject->NotifyRS2(selection);
    cout<<"realsense open!!!"<<endl;
    while (1)
    {
        rs2::frameset frames = pipe.wait_for_frames();
        //Get each frame
        auto color_frame = frames.get_color_frame();
        auto depth_frame = frames.get_depth_frame();
        //create cv::Mat from rs2::frame
        Mat color(Size(1920, 1080), CV_8UC3 , (void*)color_frame.get_data(), Mat::AUTO_STEP);
        Mat depth(Size(424 , 240 ), CV_16UC1, (void*)depth_frame.get_data(), Mat::AUTO_STEP);
        pSubject->NotifyRGB(color);
        pSubject->NotifyDEP(depth);
        if(!initFlg) {
            initFlg = true;
        }
    }
}
vector<Mat> realsensectl::oneShot() {

    Mat rgb_temp=pObserver_deal->Get_rgb().clone();
    Mat dep_temp=pObserver_deal->Get_dep().clone();
    vector<Mat> imgRet = {rgb_temp,dep_temp};
//    imshow("rgb",rgb_temp);
//    cv::waitKey(50);
    return imgRet;

//    return rgb_temp;
}

rs2::pipeline_profile realsensectl::rs2Shot() {
    rs2::pipeline_profile rs = pObserver_deal->Get_rs2();
    return rs;
}
