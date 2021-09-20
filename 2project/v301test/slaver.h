#ifndef SLAVER_H
#define SLAVER_H

#include <QObject>
#include"common/commonheaders.h"
class slaver : public QObject
{
    Q_OBJECT
public:
    explicit slaver(QObject *parent = nullptr);
    void myChangeThread(){
        thread= new QThread;
        this->moveToThread(thread);
        thread->start();
    }
    QThread *thread = new QThread;
signals:
    void back();

public slots:
    void giveBack(){
        Sleep(1000);
        qDebug()<<"slaver-end";
        emit back();
    }
};

#endif // SLAVER_H
