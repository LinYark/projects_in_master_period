#include "detect.h"

detect::detect(){}
bool  detect::initFlg=false;
string detect::net = "/home/nvidia/QT/Tx2DetraSendrec/yolo4_fp32.rt";

void detect::myinit(pipelineData* plDin) {
    thread t1(pipelineDetect,plDin);
    t1.detach();
}

void detect::pipelineDetect(pipelineData* plDin) {
    pipelineData* plD = plDin;
    tk::dnn::DetectionNN *detNN;
    std::vector<cv::Mat> batch_frame;
    std::vector<cv::Mat> batch_dnn_input;
    int n_classes = 4;
    int n_batch = 1;
    if(n_batch < 1 || n_batch > 64)
        FatalError("Batch dim not supported");
    tk::dnn::Yolo3Detection yolo;
    detNN = &yolo;
    detNN->init(net, n_classes, n_batch);
    initFlg =true;
    while(plD->SDetResult.gRun) {
        if(plD->DetectStart == true && plD->TrackStrart == false && plD->matReload==false) {
            std::vector<Bbox> detVector;
            batch_dnn_input.clear();
            batch_frame.clear();

            for(int bi=0; bi< n_batch; ++bi){
                if(!plD->DetectFrame.data) {
                    continue;
                }
                batch_frame.push_back(plD->DetectFrame.clone());

                // this will be resized to the net format
                batch_dnn_input.push_back(plD->DetectFrame.clone());
            }
            if(!plD->DetectFrame.data) {
                printf("Pic false!!!\n");
                plD->ofs_a<<"Pic false!!!\n";
                plD->ofs_a.flush();
                plD->matReload = true;
                continue;
            }

            //inference
            detNN->update(batch_dnn_input, n_batch);
            detVector = detNN->draw(batch_frame,plD);
            if(!detVector.empty()){
                for(int i=0; i<detVector.size(); i++) {
                    std::cout<<detVector[i].bx0<<"\t"<<detVector[i].by0<<"\t"<<detVector[i].bx1<<"\t"<<detVector[i].by1<<"\t"<<detVector[i].bcl<<"\t"<<detVector[i].prob<<"\n";
                }
            }
            plD->DetectStart = false;
            plD->TrackStrart = true;
        }
    }
}
