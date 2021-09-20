#ifndef LOCTION_H
#define LOCTION_H

#include <QObject>
#include"common/commonheaders.h"
#include"common/commonopencvheaders.h"
class location : public QObject
{
    Q_OBJECT
public:
    explicit location(QObject *parent = nullptr);
    ~location(){    thread->terminate();
                     thread->deleteLater();
                qDebug()<<"del location CameraThread";}
    void myChangeThread(){
        thread= new QThread;
        this->moveToThread(thread);
        thread->start();
    }


    //contributes
    bool myInitFlg = false;
private:
    QThread *thread= new QThread ;

signals:
    void computeReady();
public slots:
    void compute(){qDebug()<<"#compute-end#";
                   emit computeReady();
                   qDebug()<<"location-ThreadID:"<<QThread::currentThreadId();
                  }
    void myinit(){
        qDebug()<<"#location-myinit#";
        this->myChangeThread();
        myInitFlg = true;
        Sleep(1);
        //qDebug()<<"location-init-ThreadID:"<<QThread::currentThreadId();
    }


};



#endif // LOCTION_H
