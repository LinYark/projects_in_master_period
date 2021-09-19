#include "clipnetb.h"
#include "Server/parameter.h"

#ifdef FULLY_BUILD
#include "Compute/mask_detector.h"
#include "Compute/weld_detector.h"
#include "Compute/character.h"
#endif

ClipNetB::ClipNetB(QObject *parent) : QObject(parent)
{
    thread = new QThread;
    this->moveToThread(thread);
    thread->start();

    QTimer::singleShot(0, this, SLOT(init()));
}

void ClipNetB::init()
{
    if(PP->check_training_model()){

#ifdef FULLY_BUILD
        //插入网络B初始化代码
        WP = new Weld_Processing;
        WP->Net4();
        qDebug() << "WP->Net4()";
#endif
        emit netB_load_ready();
    }    
}

void ClipNetB::add_one_group(std::vector<cv::Mat> DecImg)
{
    QMutexLocker m_lock(&mutex);
             //替换成网络B执行代码
    std::vector<cv::Mat> WeldOut;

#ifdef FULLY_BUILD
    double End,Start;
    Start = (double)getTickCount();

    for(int i=0;i<DecImg.size();i++)
    {
        double tMASKSTART,tMASKEND;
        tMASKSTART = (double)getTickCount();
        Mat weldDeteOut_one0 = WP->Img_detector(DecImg[i]);
        tMASKEND = (double)getTickCount();
//        qDebug()<<"ClipNetB "<<i<<"     weldDeteOut_one0 time:"<<(tMASKEND - tMASKSTART) * 1000 / (getTickFrequency())<<"ms\n";
        WeldOut.push_back(weldDeteOut_one0);
    }

    End = (double)getTickCount();
//    qDebug()<<"WWWWWWWWWWWWWWWWWWWWWWWWeld all time:"<<(End - Start) * 1000 / (getTickFrequency())<<"ms\n";

#else
    QThread::sleep(2);
#endif

    emit netB_ready(WeldOut);
}
