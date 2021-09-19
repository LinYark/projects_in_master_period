#ifndef CAMERATHREAD_H
#define CAMERATHREAD_H

#include <QObject>
#include "basler_camera.h"

/********************************************************
 * @ClassName:  CameraThread
 * @Brief:      摄像头线程控制类,完成所有摄像头的管理,包括初始化,
 *              拍照和数据传送
 * @date        2020/08/31
 ********************************************************/
class CameraThread : public QObject
{
    Q_OBJECT
public:
    explicit CameraThread(QObject *parent = nullptr);
    ~CameraThread();

signals:
    void camera_init_finished(QStringList d_list);  //初始化完成,返回sn列表
    void shot_finished(QImage,int);                 //将照片和相机序号发送至主线程
    void one_group_ready(std::vector<cv::Mat>);     //一组数据齐全,发送至计算线程
    void update_info(QString);                      //更新文本信息

    void CameraInit(const CDeviceInfo &Di);
    void CloseCamera();
    void TakeAShot();
    void SetExposureTime(int t);

public slots:
    void camera_init();                             //摄像头初始化
    void camera_reset();                            //摄像头重置
    void take_a_shot(double pos);                   //执行一次拍照
    void set_camera_exposure_time();                //设置曝光时间

private slots:
    void deal_shot_photo(cv::Mat cv_image, QString d);     //处理拍照结果

private:
    DeviceInfoList_t devices;

    QImage cvMat_to_QImage(const cv::Mat& mtx);     //图片格式转换
//    cv::Mat QImage_to_cvMat(QImage image);

    QMap<int, cv::Mat> pic_map;     //记录相机单批次拍摄内容
    double current_pos;             //记录当前龙门位置
    void write_longmen_pos_file();

    bool camera_init_flag;
    QStringList d_list;

//    QMutex mutex;
    QThread *thread;
    QList<BaslerCameraGigI *> camera_list;
};

#endif // CAMERATHREAD_H
