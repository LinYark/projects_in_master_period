#include "detect.h"

bool  detect::initFlg=false;
DetectData* detect::myDetectData=new DetectData;
void detect::myinit(string netPath) {
    thread t1(pipelineDetect,netPath);
    t1.detach();
}

void detect::pipelineDetect(string netPath) {

    DetectData* dtD = myDetectData;
    tk::dnn::DetectionNN *detNN;
    std::vector<cv::Mat> batch_frame;
    std::vector<cv::Mat> batch_dnn_input;
    int n_classes = 2;
    int n_batch = 1;
    if(n_batch < 1 || n_batch > 64)
        FatalError("Batch dim not supported");
    tk::dnn::Yolo3Detection yolo;
    detNN = &yolo;
    detNN->init(netPath, n_classes, n_batch);
    initFlg =true;
    while(1) {
        if(dtD->flashFlg==true){
            QThread::msleep(10);
            continue;
        }
        batch_dnn_input.clear();
        batch_frame.clear();

        for(int bi=0; bi< n_batch; ++bi){
            if(!dtD->DetectFrame.data) {
                continue;
            }
            batch_frame.push_back(dtD->DetectFrame.clone());

            // this will be resized to the net format
            batch_dnn_input.push_back(dtD->DetectFrame.clone());
        }
        if(!dtD->DetectFrame.data) {
            printf("Pic false!!!\n");
            continue;
        }

        //inference
        detNN->update(batch_dnn_input, n_batch);

        dtD->detV = detNN->draw(batch_frame);
        if(!dtD->detV.empty()){
            for(int i=0; i<dtD->detV.size(); i++) {
//                std::cout<<dtD->detV[i].bx0<<"\t"<<dtD->detV[i].by0<<"\t"<<dtD->detV[i].bx1<<"\t"<<dtD->detV[i].by1<<"\t"<<dtD->detV[i].bcl<<"\t"<<dtD->detV[i].prob<<"\n";
            }
        }
        dtD->flashFlg=true;

    }

}
