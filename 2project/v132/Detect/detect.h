#ifndef DETECT_H
#define DETECT_H
#include "common.h"
#include "Detect/Yolo3Detection.h"
#include"pipeline/pipelinedata.h"

class detect
{
public:
    detect();
    static std::string net ;
    void myinit(pipelineData* plDin);
    static void pipelineDetect(pipelineData *plDin);
    static bool initFlg;
};

#endif // DETECT_H
