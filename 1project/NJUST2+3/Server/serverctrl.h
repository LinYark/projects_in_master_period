#ifndef SERVERCTRL_H
#define SERVERCTRL_H

#include <QObject>
#include <QHostAddress>
#include <QUdpSocket>

struct weld_same_data;

/********************************************************
 * @ClassName:  ServerCtrl
 * @Brief:      服务器通信控制类,负责和服务器之间的UDP报文收发,
 *              和内部计算线程间的通信,是对外交互的门户
 * @date        2020/08/31
 ********************************************************/
class ServerCtrl : public QObject
{
    Q_OBJECT
public:
    explicit ServerCtrl(QObject *parent = nullptr);

    void reset_server();        //重置和服务的连接

    //向服务器发送数据
    void SendControData_STARTOK();  //发送程序初始化完成标志
    void AskGetLongmenPosX();       //开始请求龙门位置(废弃)
    void EndGetLongmenPosX();       //结束请求龙门位置(废弃)
    void SendShutdown_OK();         //发送拍摄完成
    void SendWeldSeamData(weld_same_data* mData);   //发送焊缝数据

signals:
    void longmen_ready();           //龙门准备完毕
    void longmen_pos_changed(double pos);           //龙门位置变化
    void update_info(QString);      //更新文本信息

public slots:
    void send_weldData(std::vector<std::vector<cv::Point3d>> weldData, std::string strPart);        //发送焊缝信息

private slots:
    void read_data();

private:
    QUdpSocket* mSocket;
    QHostAddress nRemoteAddress;
    quint16 nRemotePort;
    bool bSendWeldSeamdata;

    QTimer *timer;
    void set_server(QHostAddress host,qint16 port);     //设置服务器地址,端口
    void show_info(QString text);
};

#endif // SERVERCTRL_H
