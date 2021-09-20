#ifndef MAINCONTROL_H
#define MAINCONTROL_H

#include <QObject>
#include<QDebug>

#include"common/commonopencvheaders.h"

#include"reserveuavparam/reserveuavparam.h"

#include"realsence/realsense.h"
#include "detection/detection.h"
#include"track/track.h"
#include"location/location.h"

#include"senduavparam/senduavparam.h"


#define initCheck 1
#define uavControlCheck 2

class maincontrol : public QObject
{
    Q_OBJECT
public:
    explicit maincontrol(QObject *parent = nullptr);
    ~maincontrol(){    thread->terminate();
                     thread->deleteLater();
                qDebug()<<"del maincontrol CameraThread";}
    void imQuit();
    void myconnect();
    void uavTakeControl();
    void init();
private:
    //functions
    void check(uint checkModel);
    void myChangeThread(){
        thread= new QThread;
        this->moveToThread(thread);
        thread->start();
    }
    void myinit(){

        this->myChangeThread();
        Sleep(1);
        emit initAll();
    }

    //contributes
    reserveuavparam *RsvParam=new reserveuavparam;
    realsense *RealSense = new realsense;
    detection *Detection = new  detection;
    track *Track = new track;
    location *Location = new location;
    senduavparam *sendParam = new senduavparam;

    bool initFlg = 0;
    QThread *thread= new QThread ;

    bool oneTurnDoneFlg = true;
signals:
    void initAll();
    void begin();

public slots:
    void end(){qDebug()<<"#end-end,一轮结束#";
              oneTurnDoneFlg=true;
              }
};

#endif // MAINCONTROL_H
