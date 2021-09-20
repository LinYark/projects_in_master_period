#ifndef MYREALSENSE_H
#define MYREALSENSE_H
#endif // MYREALSENSE_H
#include <QObject>

#include"common/commonheaders.h"



class realsense : public QObject
{
    Q_OBJECT
public:
    explicit realsense(QObject *parent = nullptr);
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
    void RSPReady();
public slots:
    void getCurrentRSP(){qDebug()<<"getCurrentRSP-end!";
                        emit RSPReady();
                        }

    void myinit(){
        qDebug()<<"#realsense-myinit#";
        this->myChangeThread();
        myInitFlg = true;
        Sleep(1);
    }

};



