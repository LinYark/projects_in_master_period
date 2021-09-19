#include "serverctrl.h"
#include "serverdefs.h"
#include "Server/parameter.h"
#include "Compute/common.h"

ServerCtrl::ServerCtrl(QObject *parent) : QObject(parent)
{
    mSocket = new QUdpSocket();
    bSendWeldSeamdata = false;

    reset_server();
}

void ServerCtrl::reset_server()
{
    set_server(QHostAddress(PP->remote_address()), PP->remote_port());
}

void ServerCtrl::set_server(QHostAddress host, qint16 port)
{
    nRemoteAddress = host;
    nRemotePort = port;

    if(mSocket->state() == QAbstractSocket::BoundState){
        mSocket->disconnectFromHost();
    }

    if(mSocket->bind(QHostAddress(PP->self_address()),PP->self_port())){
        qDebug()<<QString("绑定%1成功！").arg(PP->self_address());
        connect(mSocket,SIGNAL(readyRead()),this,SLOT(read_data()));
        qDebug()<<"[1]. set remote helper process success,please send your requests...\n";
    }
    else{
        qDebug()<<QString("绑定%1失败！").arg(PP->self_address());
        qDebug()<<"[1]. set remote helper process failed,please retry...\n";
    }
}

void ServerCtrl::show_info(QString text)
{
    qDebug() << text;
    emit update_info(" [" + QTime::currentTime().toString() + "]" + text);
}

void ServerCtrl::read_data()
{
    QByteArray ba;
    while(mSocket->hasPendingDatagrams())
    {
        ba.resize(mSocket->pendingDatagramSize());
        mSocket->readDatagram(ba.data(), ba.size());
        MessageHeader* hdr = (MessageHeader*)ba.data();
        WeldSeamData *wsdata;
        ControlData *conData;

        switch(hdr->type)
        {
        case MSG_TYPE_LMPOS:            //龙门数据
        {
            LongmenData *value = (LongmenData*) ba.data();
            emit longmen_pos_changed(value->dLongmenPosX);
        }
            break;
        case MSG_TYPE_SPFJSON:
            break;
        case MSG_TYPE_WELDSEAM:         //焊缝数据
            wsdata = (WeldSeamData*)(char*)ba.data();
            if(wsdata->commandtype == COMMAND_TYPE_REQUEST_START)
            {
                bSendWeldSeamdata = true;
                //SendWeldseamData();
            }
            else if(wsdata->commandtype == COMMAND_TYPE_REQUEST_END)
            {
                bSendWeldSeamdata = false;
            }
            break;
        case MSG_TYPE_CONTROL:          //龙门到达开始位置，可以开始工作
            conData = (ControlData*)(char*)ba.data();
            if(conData->commandtype == COMMAND_TYPE_SYSTEM_START)
            {
                //龙门到达初始位置，开始初始化程序
//                qDebug()<<"收到龙门准备就绪信息!";
                show_info("收到龙门准备就绪信息!");
                emit longmen_ready();
//                SendControData_STARTOK();
            }
            break;
        default:
            break;
        }
    }
}

void ServerCtrl::SendControData_STARTOK()
{
    ControlData conData;

    //header
    conData.header.sig    = MSG_SIGNATURE;
    conData.header.type   = MSG_TYPE_CONTROL;
    conData.header.length = sizeof(qint16);

    //data
    conData.commandtype =  COMMAND_TYPE_SYSTEM_START_OK;

    mSocket->writeDatagram((char*)&conData.header.sig, sizeof(conData.header)+ conData.header.length,nRemoteAddress,nRemotePort);

    qDebug()<<"[3]. void CNetControl::SendControData_STARTOK...\n";
}

void ServerCtrl::AskGetLongmenPosX()
{
    LongmenData lmData;

    //header
    lmData.header.sig    = MSG_SIGNATURE;
    lmData.header.type   = MSG_TYPE_LMPOS;
    lmData.header.length = sizeof(qint16) + sizeof(double);

    //data
    lmData.commandtype =  COMMAND_TYPE_REQUEST_START;
    lmData.dLongmenPosX = 0.0;


    mSocket->writeDatagram((char*)&lmData.header.sig, sizeof(lmData.header)+ lmData.header.length,nRemoteAddress,nRemotePort);

    std::cout<<"[3]. void CNetControl::AskGetLongmenPosX()...\n";
}

void ServerCtrl::EndGetLongmenPosX()
{
    LongmenData lmData;

    //header
    lmData.header.sig    = MSG_SIGNATURE;
    lmData.header.type   = MSG_TYPE_LMPOS;
    lmData.header.length = sizeof(qint16) + sizeof(double);

    //data
    lmData.commandtype =  COMMAND_TYPE_REQUEST_END;
    lmData.dLongmenPosX = 0.0;


    mSocket->writeDatagram((char*)&lmData.header.sig,
                           sizeof(lmData.header)+ lmData.header.length,
                           nRemoteAddress,
                           nRemotePort);

    std::cout<<"[3]. void CNetControl::EndGetLongmenPosX()...\n";
}

void ServerCtrl::SendShutdown_OK()
{
    ShutdownData shutData;
    shutData.header.sig    = MSG_SIGNATURE;
    shutData.header.type   = MSG_TYPE_CONTROL;
    shutData.header.length = sizeof(qint16);

    //data
    shutData.commandtype =  COMMAND_TYPE_SYSTEM_SHUTDOWN_OK;

    mSocket->writeDatagram((char*)&shutData.header.sig, sizeof(shutData.header)+ shutData.header.length,nRemoteAddress,nRemotePort);

    std::cout<<"[3]. void CNetControl::SendShutdown_OK...\n";
}

void ServerCtrl::SendWeldSeamData(weld_same_data *mData)
{
    WeldSeamData wsData;

    //header
    wsData.header.sig    = MSG_SIGNATURE;
    wsData.header.type   = MSG_TYPE_WELDSEAM;
    wsData.header.length = sizeof(wsData.commandtype) + sizeof(char)*100 + 2*sizeof(qint16) + (mData->pointQuantity)*sizeof(qint64);

    //data
    wsData.commandtype =  COMMAND_TYPE_DATA;

    qint32 packetlen = sizeof(wsData.header) + wsData.header.length;

    char*buffer = new char[packetlen];

    memset(buffer,0,packetlen);

    memcpy(buffer,
           (char*)(&wsData.header.sig),sizeof(wsData.header));//包头

    memcpy(buffer + sizeof(wsData.header),(char*)&wsData.commandtype,sizeof(wsData.commandtype));

    //weldseam data
    //part ID
    memcpy(buffer + sizeof(wsData.header) + sizeof(wsData.commandtype),
           (char*)mData->partId,sizeof(char)*50);

    //seam quantity
    memcpy(buffer + sizeof(wsData.header) + sizeof(wsData.commandtype) + sizeof(char)*50,
           (char*)&mData->seamQuantity,sizeof(qint16));

    //seam ID
    memcpy(buffer + sizeof(wsData.header) + sizeof(wsData.commandtype) + sizeof(char)*50 + sizeof(qint16),
           mData->seamId,sizeof(char)*50);


    memcpy(buffer + sizeof(wsData.header) + sizeof(wsData.commandtype) + sizeof(char)*50 + sizeof(qint16) + sizeof(char)*50,
           (char*)&(mData->pointQuantity),sizeof(qint16));


    memcpy(buffer + sizeof(wsData.header) + sizeof(wsData.commandtype) + sizeof(char)*50 + sizeof(qint16) +  sizeof(char)*50 + sizeof(qint16),
           (char*)mData->points,mData->pointQuantity*sizeof(qint64));

    mSocket->writeDatagram(buffer,packetlen,nRemoteAddress,nRemotePort);

}

void ServerCtrl::send_weldData(std::vector<std::vector<cv::Point3d> > weldData, std::string strPart)
{

    int send_num = 0;
//    int mm=send_ID;
//    cout << "发送焊缝数据中-----------" << endl;
    show_info("发送焊缝数据中-----------");
    for (int i = 0; i < weldData.size(); i++)
    {
        weld_same_data data;
//        std::string strPart = Mask_String[mm] + "_" + num2str1(mm);
        memset(data.partId, 0, 50);
        strcpy(data.partId, strPart.c_str());
        std::string strWeldseam = Common::num2str1(i);
        memset(data.seamId, 0, 50);
        strcpy(data.seamId, strWeldseam.c_str());
        data.pointQuantity = 2;
        data.points = new qint64[data.pointQuantity];
        memset(data.points, 0, data.pointQuantity * sizeof(qint64));
        for (int j = 0; j < 2; j++)
        {
            qint32 x1 = weldData[i][j].x * 100;
            qint32 y1 = weldData[i][j].y * 100;
            memcpy((char*)data.points + sizeof(qint32) * 2 * j, &x1, sizeof(qint32));
            memcpy((char*)data.points + sizeof(qint32)*(2 * j + 1), &y1, sizeof(qint32));
        }
        SendWeldSeamData(&data);
        send_num++;
        delete[]data.points;

    }

    show_info("发送成功-----------");
    show_info(QString("该工件发送焊缝数量: %1").arg(send_num) );


}
