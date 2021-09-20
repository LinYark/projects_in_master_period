#include "maincontrol.h"

maincontrol::maincontrol(QObject *parent) : QObject(parent)
{

}

void  maincontrol::init()
{

    this->myinit();

    uint initTimeLimit = 100;
    while (1) {
        bool initFinishCondition = RsvParam->myInitFlg && RealSense->myInitFlg &&Detection->myInitFlg &&
                Track->myInitFlg && Location->myInitFlg && sendParam->myInitFlg ;
        if( initFinishCondition ){
            break;
        }
        Sleep(100);

        initTimeLimit--;
        if(initTimeLimit==0){
            qDebug()<<"##########out of initLimitTime,init error##########";
            exit(0);
        }
    }
    qDebug()<<"主控函数ThreadID:"<<QThread::currentThreadId();
    qDebug()<<"#主控函数及所有子函数初始化结束#";
    check(initCheck);
}

void maincontrol::myconnect()
{
    connect(this,&maincontrol::initAll,RsvParam,&reserveuavparam::myinit);
    connect(this,&maincontrol::initAll,RealSense,&realsense::myinit);
    connect(this,&maincontrol::initAll,Detection,&detection::myinit);
    connect(this,&maincontrol::initAll,Track,&track::myinit);
    connect(this,&maincontrol::initAll,Location,&location::myinit);
    connect(this,&maincontrol::initAll,sendParam,&senduavparam::myinit);


    connect(this,&maincontrol::begin,RealSense,&realsense::getCurrentRSP);
    connect(this,&maincontrol::begin,RsvParam,&reserveuavparam::getCurrentUAVP);

    connect(RealSense,&realsense::RSPReady,Detection,&detection::getRSPImform);
    connect(RsvParam,&reserveuavparam::UAVPReady,Detection,&detection::getUAVImform);

    connect(Detection,&detection::targetComfirm,Track,&track::newtrack);
    connect(Detection,&detection::targetKeep,Track,&track::continueTrack);

    connect(Track,&track::doneTrack,Detection,&detection::chooseRect);

    connect(Detection,&detection::rectReady,Location,&location::compute);

    connect(Location,&location::computeReady,sendParam,&senduavparam::uavFix);

    connect(sendParam,&senduavparam::sendDone,this,&maincontrol::end);

}

void maincontrol::uavTakeControl()
{
    bool takeControlFlg = false;
    while(1){

        takeControlFlg = RsvParam->myUavTakeControl() ;
        if(takeControlFlg ==true ){
            break;
        }
        Sleep(1);
    }
    Sleep(1);
    sendParam->sendFirstFix();

    check(uavControlCheck);
}


void  maincontrol::imQuit()
{
    while(1){
        if(oneTurnDoneFlg){
            oneTurnDoneFlg=false;
            emit begin();
        }
        Sleep(2000);
        static int turnhfc = 0;

        qDebug()<<"第"<<turnhfc++<<"轮";
    }

}














void maincontrol::check(uint checkModel)
{
    switch (checkModel) {
    case initCheck:


        break;
    case uavControlCheck:


        break;

    default:

        break;

    }
}






