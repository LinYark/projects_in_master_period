#include "common.h"
#include "PartRealSense/realsensectl.h"
#include "encode/encodectl.h"
#include "pipeline/pipelinectl.h"
#include "Detect/detect.h"
#include "LocTrack/tracklogic.h"

#define encodeFlg 1



int main()
{
    //init txt
    pipelineData *pld = new pipelineData();
    string configPath = "/home/nvidia/jiqun/conf.txt";
    pld->myinit(configPath);


    realsensectl *rsc=new realsensectl();
    rsc->myinit();

#if encodeFlg
    encodectl *ecc = new encodectl();
    ecc->imgData =new ImageData{(new Mat(1080,1920,CV_8UC3)),false," ",true};
    ecc->myinit();
#endif

    pipelineCtl *plc = new  pipelineCtl();
    plc->myinit(pld);


    detect *det = new detect();
    det->myinit(pld);

    tracklogicctl *tra = new tracklogicctl();
    tra->myinit(pld);
    while(1){
        if(rsc->initFlg && ecc->initFlg &&det->initFlg && tra->initFlg){
            break;
        }
        QThread::msleep(10);
    }

    while(!pld->shutdown) {
        vector<Mat> frames   = rsc->oneShot();
        plc->send(pld,rsc);

#if encodeFlg
        common::m1.lock();
        *(ecc->imgData->vis) = frames[0];
        ecc->imgData->visNew = true;
        common::m1.unlock();
#endif
    }
    return 0;
}
