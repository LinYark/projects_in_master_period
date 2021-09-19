#include "camerathread.h"
#include <QThread>
#include <Compute/common.h>
#include "Server/parameter.h"

CameraThread::CameraThread(QObject *parent) : QObject(parent)
{
    camera_init_flag = false;

    thread = new QThread;
    this->moveToThread(thread);
    thread->start();
}

CameraThread::~CameraThread()
{
    emit CloseCamera();
    thread->terminate();
    thread->deleteLater();
    qDebug()<<"del thread CameraThread";
}

//相机初始化,返回一个设备列表
void CameraThread::camera_init()
{
//    qDebug()<<"CameraThread:"<<QThread::currentThreadId();
    if(!camera_init_flag){
        PylonInitialize();
        CTlFactory& tlFactory = CTlFactory::GetInstance();
        if ( tlFactory.EnumerateDevices(devices) > 0 ){
            for (uint i = 0; i < devices.size(); ++i) {
                d_list.append(QString(devices[i].GetSerialNumber()));
            }
        }
        emit camera_init_finished(d_list);
        if(!d_list.isEmpty())
            camera_init_flag = true;

        //继续做初始化的工作
        camera_list.clear();
        for (uint i = 0; i < devices.size(); ++i) {
            camera_list.append(new BaslerCameraGigI(devices[i]));
        }
    }
    else{
        emit camera_init_finished(d_list);
    }

    qDebug()<<"相机初始化完毕";
    emit update_info(tr("相机初始化完毕"));
}

//摄像头重置
void CameraThread::camera_reset()
{
    for (uint i = 0; i < devices.size(); ++i) {
        //断开连接
        disconnect(this, SIGNAL(CloseCamera()), camera_list.at(i), SLOT(CloseCamera()));
        disconnect(this, SIGNAL(TakeAShot()), camera_list.at(i), SLOT(TakeAShot()));
        disconnect(this, SIGNAL(SetExposureTime(int)), camera_list.at(i),SLOT(SetExposureTime(int)));
        disconnect(camera_list.at(i), SIGNAL(shot_done(cv::Mat,QString)), this, SLOT(deal_shot_photo(cv::Mat, QString)));
        disconnect(camera_list.at(i), SIGNAL(update_info(QString)), this, SIGNAL(update_info(QString)));

        //根据实际开启数打开连接
        QString s_n = QString(devices[i].GetSerialNumber());
        if(PP->active_sn_list().contains(s_n)){
            connect(this, SIGNAL(CloseCamera()), camera_list.at(i), SLOT(CloseCamera()));
            connect(this, SIGNAL(TakeAShot()), camera_list.at(i), SLOT(TakeAShot()));
            connect(this, SIGNAL(SetExposureTime(int)), camera_list.at(i),SLOT(SetExposureTime(int)));
            connect(camera_list.at(i), SIGNAL(shot_done(cv::Mat,QString)), this, SLOT(deal_shot_photo(cv::Mat, QString)));
            connect(camera_list.at(i), SIGNAL(update_info(QString)), this, SIGNAL(update_info(QString)));
        }
    }

    emit SetExposureTime(PP->camera_exposure_time());
}

//所有相机执行一次拍照
void CameraThread::take_a_shot(double pos)
{
    qDebug()<<"发送触发信号! 龙门位置:"<<pos;
    current_pos = pos;
    if(PP->test_mode() == OFFLINE){         //离线
        //读测试图
        vector<cv::Mat> v;
        for (int i = 0; i < PP->offline_camera_list().count(); ++i) {
            if(PP->offline_camera_list().at(i)){
                QString str = PP->offline_path() + "/camera" + QString::number(i+1) + "-"
                        + QString::number(PP->shot_longmen_pos_record().count()) + "----.bmp";
                cv::Mat PFA_img = cv::imread(str.toStdString());        //读取

                if(PFA_img.channels()==1)
                {cvtColor(PFA_img,PFA_img,COLOR_GRAY2BGR);}

                emit shot_finished(cvMat_to_QImage(PFA_img), i);        //发送
//                PP->_CameraImg_PFA.push_back(PFA_img);
                PP->cameraImg_PFA_append(PFA_img);
                v.push_back(PFA_img);                                   //存储至一组


                cvtColor(PFA_img,PFA_img,COLOR_BGR2GRAY);
                str = PP->save_path() + "/" + PP->str_time() + "/yuantu/camera" + QString::number(i+1) + "-" + QString::number(PP->shot_longmen_pos_record().count())
                        + "----.bmp";
                cv::imwrite(str.toStdString(), PFA_img);                //保存数据
                cvtColor(PFA_img,PFA_img,COLOR_GRAY2BGR);
            }
        }
//        qDebug()<<"发送一组！";
        emit one_group_ready(v);
        write_longmen_pos_file();
    }
    else{                                   //在线
//        QMutexLocker locker(&mutex);
//        for (uint i = 0; i < devices.size(); ++i) {
//            QString s_n = QString(devices[i].GetSerialNumber());

//            if(PP->active_sn_list().contains(s_n)){
//                emit update_info(s_n + tr("相机设置软触发[%1]成功").arg(QTime::currentTime().toString("hh:mm:ss--zzz")) );
//            }
//        }
        emit TakeAShot();
    }
}

//设置曝光时间
void CameraThread::set_camera_exposure_time()
{
    qDebug()<<"重设曝光时间为："<<PP->camera_exposure_time();

    emit SetExposureTime(PP->camera_exposure_time());
}

//处理拍照结果
void CameraThread::deal_shot_photo(cv::Mat cv_image, QString d)
{
//    qDebug()<<"deal_shot_photo:"<<d;

//    QMutexLocker locker(&mutex);

    int index = PP->sn_to_index(d);
    emit shot_finished(cvMat_to_QImage(cv_image), index);
    emit update_info(d + tr("相机拍照成功 编号:%1[%2]").arg(index).arg(QTime::currentTime().toString("hh:mm:ss--zzz")));

    //按顺序压入照片组中，如果一组已满，则发送至计算线程
    if(pic_map.contains(index)){
        pic_map.clear();
        qDebug()<<"Q_ASSERT(!pic_map.contains(index)) error!";
    }
    pic_map.insert(index, cv_image);
    if(pic_map.count() == PP->active_camera_num()){     //判断拍摄次数满
        vector<cv::Mat> v;
        foreach (cv::Mat mat, pic_map.values()) {
            v.push_back(mat);
            PP->cameraImg_PFA_append(mat);
        }
        qDebug()<<"发送一组" << d;
        emit one_group_ready(v);
        write_longmen_pos_file();

        pic_map.clear();
    }

    //保存数据 
//    QString str = PP->save_path() +"/camera" + QString::number(index +1) + "-" + QString::number(PP->shot_longmen_pos_record.count())
//            +"----.bmp";
    //new_save_path-----------------------------------------------------------------------------------------------------------------------------------------------
//    QString str = PP->save_path() +"/"+PP->str_time()+ "/yuantu/camera" + QString::number(index +1) + "-" + QString::number(PP->shot_longmen_pos_record.count())
//            + "-" + QString::number(current_pos) + "----.bmp";
    QString str = PP->save_path() +"/"+PP->str_time()+ "/yuantu/camera" + QString::number(index +1) + "-" + QString::number(PP->shot_longmen_pos_record().count())
            + "----.bmp";
    cvtColor(cv_image,cv_image,COLOR_RGB2GRAY);
    cv::imwrite(str.toStdString(), cv_image);                //保存数据
    cvtColor(cv_image,cv_image,COLOR_GRAY2RGB);

}

//图片格式转换
QImage CameraThread::cvMat_to_QImage(const cv::Mat &mtx)
{
    switch (mtx.type())
    {
    case CV_8UC1:
    {
        QImage img((const unsigned char *)(mtx.data), mtx.cols, mtx.rows, mtx.cols, QImage::Format_Grayscale8);
        return img;
    }
        break;
    case CV_8UC3:
    {
        QImage img((const unsigned char *)(mtx.data), mtx.cols, mtx.rows, mtx.cols * 3, QImage::Format_RGB888);
        return img.rgbSwapped();
    }
        break;
    case CV_8UC4:
    {
        QImage img((const unsigned char *)(mtx.data), mtx.cols, mtx.rows, mtx.cols * 4, QImage::Format_ARGB32);
        return img;
    }
        break;
    default:
    {
        QImage img;
        return img;
    }
        break;
    }
}

void CameraThread::write_longmen_pos_file()
{
    QString path = PP->save_path() + "/" + PP->str_time() + "/yuantu/LongmenPos.txt";
    QFile file(path);
    if(!file.open(QIODevice::ReadWrite|QIODevice::Text)){
        qDebug()<<"write LongmenPos.txt failed";
        return;
    }
    QTextStream in(&file);
    foreach (double pos, PP->shot_longmen_pos_record()) {
        in << pos << endl;
    }
}
















