#ifndef PIPELINECTL_H
#define PIPELINECTL_H
#include"common.h"
#include"pipeline/pipelinedata.h"
#include "PartRealSense/realsensectl.h"
class pipelineCtl
{
public:
    pipelineCtl();

    pipelineData* plD ;
    static bool initFlg;
    void flush();
    void myinit(pipelineData* plDin ){
        plD  = plDin;
        flush();
        thread t1(Rec,plD);
        t1.detach();
        initFlg=true;
    }

    int pipeCount;
    bool firstDetect;
    bool brkFLg;
    static void Rec(pipelineData* plD);

    void send(pipelineData* plDin,realsensectl *rsc);
};

#endif // PIPELINECTL_H
