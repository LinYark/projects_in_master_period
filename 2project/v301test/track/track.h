#ifndef TRACK_H
#define TRACK_H

#include <QObject>
#include"common/commonheaders.h"
#include"common/commonopencvheaders.h"
class track : public QObject
{
    Q_OBJECT
public:
    explicit track(QObject *parent = nullptr);
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
    void doneTrack();
public slots:
    void myinit(){
        qDebug()<<"#track-myinit#";
        this->myChangeThread();
        myInitFlg = true;
        Sleep(1);
    }
    void newtrack(){qDebug()<<"#newtrack-end#";
                   emit doneTrack();
                   }
    void continueTrack(){}
};

#endif // TRACK_H
