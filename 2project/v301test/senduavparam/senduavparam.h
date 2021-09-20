#ifndef SENDUAVPARAM_H
#define SENDUAVPARAM_H

#include <QObject>
#include"common/commonheaders.h"
#include"common/commonopencvheaders.h"
class senduavparam : public QObject
{
    Q_OBJECT
public:
    explicit senduavparam(QObject *parent = nullptr);
    void myChangeThread(){
        thread= new QThread;
        this->moveToThread(thread);
        thread->start();
    }

    void sendFirstFix(){ qDebug()<<"#飞控第一次发送调整型号#";}

    //contributes
    bool myInitFlg = false;
private:
     QThread *thread= new QThread ;
signals:
     void sendDone();
public slots:
    void uavFix(){emit sendDone();
                 qDebug()<<"#sendDone-init#";}
    void myinit(){
        qDebug()<<"#senduavparam-myinit#";
        this->myChangeThread();
        myInitFlg = true;
        Sleep(1);
    }
};

#endif // SENDUAVPARAM_H
