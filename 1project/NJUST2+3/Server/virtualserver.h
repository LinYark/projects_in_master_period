#ifndef VIRTUALSERVER_H
#define VIRTUALSERVER_H

#include <QObject>

/********************************************************
 * @ClassName:  VirtualServer
 * @Brief:      虚拟服务器类,能够模拟一个虚拟的服务器线程,
 *              给计算线程发送指令,完成本地读取计算流程
 * @date        2020/08/31
 ********************************************************/
class VirtualServer : public QObject
{
    Q_OBJECT
public:
    explicit VirtualServer(QObject *parent = nullptr);

    void start();       //开启虚拟服务器
    void stop();        //关闭虚拟服务器

signals:
    void take_a_shot(double);

private slots:
    void deal_timeout();

private:
    QTimer *timer;

    QVector<double> shot_longmen_pos;

    void init();        //线程初始化
};

#endif // VIRTUALSERVER_H
