//===================================================================================================================================
//作者：南京理工大学 电子信息与光电工程学院 光学工程 智能光电传感实验室 
//工程名称：
//文件名：
//文件功能描述：
//===================================================================================================================================
#pragma once

#include <QObject>
#include <pylon/PylonIncludes.h>
#ifdef PYLON_WIN_BUILD
#    include <pylon/PylonGUI.h>
#endif

#include <iostream>

using namespace std;
using namespace Pylon;
using namespace GenApi;

/********************************************************
 * @ClassName:  BaslerCameraGigI
 * @Brief:      单个相机驱动类,完成相机的一系列控制
 * @date        2020/08/31
 ********************************************************/
class BaslerCameraGigI : public QObject,
                         public CImageEventHandler
{
    Q_OBJECT
public:
    BaslerCameraGigI(const CDeviceInfo& Di);
    ~BaslerCameraGigI(void);

    void OnImageGrabbed( CInstantCamera& camera, const CGrabResultPtr& ptrGrabResult);

    enum CameraMode { FREERUN, SOFTTRIGGER };

    bool OpenCamera(CameraMode mode = FREERUN);

    void StartCamera();

    void StopCamera();

    bool GetFloatPara(const char * nameNode, double& para);

    bool GetIntPara(const char * nameNode, int64_t&  para);

    bool GetStringPara(const char * nameNode, string& para);

    bool SetFloatPara(const char * nameNode, double para);

    bool SetIntPara(const char * nameNode, int64_t para);

    bool SetStringPara(const char * nameNode, String_t para);

    bool SetBooleanPara(const char * nameNode, bool para);

    bool SetCmd(const char *name);

    bool SetSoftTrigger();

    CameraMode GetCamareMode()
    {
        return m_Mode;
    }

signals:
    void shot_done(cv::Mat,QString);     //拍照完成
    void update_info(QString);      //更新文本信息

public slots:
    void CameraInit(const CDeviceInfo &Di);
    void CloseCamera();
    void TakeAShot();
    void SetExposureTime(int t);

private:
    int64_t Adjust(int64_t val, int64_t minimum, int64_t maximum, int64_t inc);

    CInstantCamera mCamera;

    GENAPI_NAMESPACE::INodeMap* nodemap;

    bool CameraRuning;
    CameraMode m_Mode;
    CPylonImage targetImage;

    QMutex mutex;

    QVector<cv::Mat> _CameraImg;
    QTimer *timer;
    QString _sn;

};
