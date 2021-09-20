#ifndef DETECTION_H
#define DETECTION_H

#include <QObject>
#include"common/commonheaders.h"
#include"common/commonopencvheaders.h"
class detection: public QObject
{
    Q_OBJECT
public:
    explicit detection(QObject *parent = nullptr);
    ~detection(){    thread->terminate();
                     thread->deleteLater();
                qDebug()<<"del detection CameraThread";}
    void myChangeThread(){
        thread= new QThread;
        this->moveToThread(thread);
        thread->start();
    }


    //contributes
    bool myInitFlg = false;
private:
     QThread *thread= new QThread ;
     QMutex mutex;
    bool RSPGet = false;
    bool UAVGet = false ;
signals:
    void targetComfirm();
    void targetKeep();
    void rectReady();

public slots:
    void myinit(){
        qDebug()<<"#detection-myinit#";
        this->myChangeThread();
        myInitFlg = true;
        Sleep(1);
    }
    void mydetection(){}
    void getRSPImform(){ RSPGet = true;
                         if(RSPGet&& UAVGet){
                             mutex.lock();
                             emit targetComfirm();
                             RSPGet=false;
                             UAVGet = false;
                             qDebug()<<"#getRSPImform & getUAVImform end#";
                             cout<<"#getRSPImform & getUAVImform end#"<<endl;
                             mutex.unlock();
                         }

                       }
    void getUAVImform(){UAVGet = true;
                        if(RSPGet&& UAVGet){
                            mutex.lock();
                            emit targetComfirm();
                            RSPGet=false;
                            UAVGet = false;
                            qDebug()<<"#getRSPImform & getUAVImform end#";
                            mutex.unlock();
                        }
                       }
    void chooseRect(){qDebug()<<"#chooseRect-end#";
                     emit rectReady();
                      //qDebug()<<"detection-ThreadID:"<<QThread::currentThreadId();
                     }
};


#endif // DETECTION_H
