    #include "virtualserver.h"
#include "parameter.h"

#define SHOT_INTERVAL 2     //虚拟拍照间隔

VirtualServer::VirtualServer(QObject *parent) : QObject(parent)
{
    timer = new QTimer;
    timer->setInterval(SHOT_INTERVAL * 1000);
    connect(timer, &QTimer::timeout, this, &VirtualServer::deal_timeout);
}

void VirtualServer::start()
{
    init();
    deal_timeout();
    timer->start();
}

void VirtualServer::stop()
{
    timer->stop();
}

void VirtualServer::deal_timeout()
{
    int n = PP->shot_longmen_pos_record().count();
    if(n < shot_longmen_pos.count() && n >= 0){
        emit take_a_shot(shot_longmen_pos.at(n));
    }
}

void VirtualServer::init()
{
    shot_longmen_pos.clear();

    QString path = PP->offline_path() + "/LongmenPos.txt";

//    qDebug()<<path;

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug()<<"read LongmenPos.txt failed";
        return;
    }

    while (!file.atEnd()) {
        QByteArray line = file.readLine();
        if(line.contains(',')){
            QList<QByteArray> list = line.split(',');
            if(list.count() < 3){
                continue;
            }
            else{
                QVector3D v(list.at(0).toDouble(), list.at(1).toDouble(), list.at(2).toDouble());
                PP->shot_longmen_pos_modify_append(v);
            }
            qDebug()<<"load LongmenPos.txt"<<PP->shot_longmen_pos_modify();
        }
        else{
            shot_longmen_pos.append(line.toDouble());
        }
    }
//    qDebug()<<"shot_longmen_pos"<<shot_longmen_pos;
}



