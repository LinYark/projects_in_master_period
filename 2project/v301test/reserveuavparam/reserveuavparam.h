#ifndef RESERVEUAVPARAM_H
#define RESERVEUAVPARAM_H

#include <QObject>
#include"common/commonheaders.h"
#include"common/commonopencvheaders.h"

class reserveuavparam : public QObject
{
    Q_OBJECT
public:
    explicit reserveuavparam(QObject *parent = nullptr);
    void myChangeThread(){
        thread= new QThread;
        this->moveToThread(thread);
        thread->start();
    }

    bool myUavTakeControl(){
        qDebug()<<"#uav刚拿到控制权#";
        return true;
    } //同时会调整姿态


    //contributes
    bool myInitFlg = false;
    bool myUavTakeControlFlg = false;
private:
     QThread *thread= new QThread ;


signals:
    void UAVPReady();
public slots:
    void getCurrentUAVP(){qDebug()<<"getCurrentUAVP-end";
                         emit UAVPReady();
                         }
    void myinit(){
        qDebug()<<"#reserveuavparam-myinit#";
        this->myChangeThread();
        myInitFlg = true;
        Sleep(1);
    }

};

#endif // RESERVEUAVPARAM_H
