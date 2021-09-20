#ifndef DETECT_H
#define DETECT_H
#include "common.h"
#include "Detect/Yolo3Detection.h"

class detect
{
public:
    void myinit(string netPath);
    static void pipelineDetect(string netPath);

    static bool initFlg;
    DetectData getCurrentDetectInf(){
        return *detect::myDetectData;
    }
    void setNewData(Mat frame,bool flashFlg){
        myDetectData->DetectFrame=frame;
        myDetectData->flashFlg = flashFlg;
    }
private:
    static DetectData* myDetectData;
};

#endif // DETECT_H
