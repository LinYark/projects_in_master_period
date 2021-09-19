//===================================================================================================================================
//作者：南京理工大学 电子信息与光电工程学院 光电技术系 江苏省光谱成像与智能感知重点实验室 A521/A523/B515 
//工程名称：
//文件名：
//文件功能描述：
//===================================================================================================================================
#include "basler_camera.h"
#include "Server/parameter.h"
#include <QVector3D>

//===================================================================================================================================
//函数名称：BaslerCameraGigI::BaslerCameraGigI(const CDeviceInfo& Di)
//函数功能：
//===================================================================================================================================
BaslerCameraGigI::BaslerCameraGigI(const CDeviceInfo& Di)
{
//    QThread *thread = new QThread;
//    this->moveToThread(thread);
//    thread->start();

    QTimer::singleShot(0, this, [this, Di](){
        CameraInit(Di);
    });
}

//===================================================================================================================================
//函数名称：BaslerCameraGigI::~BaslerCameraGigI(void)
//函数功能：
//===================================================================================================================================
BaslerCameraGigI::~BaslerCameraGigI(void)
{
    mCamera.Attach( nullptr );
    nodemap = nullptr;
}

//===================================================================================================================================
//函数名称：void BaslerCameraGigI::OnImageGrabbed( CInstantCamera& camera, const CGrabResultPtr& ptrGrabResult)
//函数功能：
//===================================================================================================================================
void BaslerCameraGigI::OnImageGrabbed( CInstantCamera& camera, const CGrabResultPtr& ptrGrabResult)
{
    QMutexLocker locker(&mutex);

    //cout<<"sn:"<<camera.GetDeviceInfo().GetSerialNumber()<<endl;
    String_t sn = camera.GetDeviceInfo().GetSerialNumber();
    _sn = QString(sn);
    if( ptrGrabResult->GrabSucceeded() )
    {
        /* add processing code */
        try
        {
            unsigned short* ptr = (unsigned short *)ptrGrabResult->GetBuffer();

            //cout << "vaule = " << ((*ptr)>>4) << endl;
            //cout <<  ptrGrabResult->GetImageSize()/ptrGrabResult->GetWidth()/ptrGrabResult->GetHeight() << endl;

            //存图到内存
            CImageFormatConverter converter;
            converter.OutputPixelFormat = PixelType_BGR8packed;
            CPylonImage targetImage;
            converter.Convert(targetImage, ptrGrabResult);

            cv::Mat Image = cv::Mat(ptrGrabResult->GetHeight(), ptrGrabResult->GetWidth(),CV_8UC3, (uint8_t*)targetImage.GetBuffer());

            qDebug().noquote()<<_sn<<"拍摄完成!!!"<<QTime::currentTime().toString("hh:mm:ss--zzz")<<ptrGrabResult->GetHeight()<<ptrGrabResult->GetWidth();

            _CameraImg.append(Image.clone());
//            emit shot_done(Image.clone(), QString(sn));     //拍摄完成------------------------------------------------------------------------------------------

        }
        catch (...)
        {
            std::cout << ("image format conersion faild!")<<std::endl;
        }
    }
    else{
//        emit update_info("抓图失败" + QString(sn) + "尝试补拍！" + QTime::currentTime().toString("hh:mm:ss--zzz"));
//        emit update_info(QString(ptrGrabResult->GetErrorDescription()));

//        qDebug()<<QString(ptrGrabResult->GetErrorDescription())<<QString(sn);
        SetSoftTrigger();
        QVector3D p;
        p.setX(PP->sn_to_index(_sn) + 1);
        p.setY(PP->shot_longmen_pos_record().count());
        p.setZ(PP->dx);
        PP->shot_longmen_pos_modify_append(p);
        qDebug()<<"记录龙门偏移量："<<p;
    }
}

//===================================================================================================================================
//函数名称：bool BaslerCameraGigI::OpenCamera(CameraMode mode)
//函数功能：
//===================================================================================================================================
bool BaslerCameraGigI::OpenCamera(CameraMode mode)
{
    cout << "OpenCamera" << endl;
    // Before using any pylon methods, the pylon runtime must be initialized.
    try
    {
        nodemap = &mCamera.GetNodeMap();

        m_Mode = mode;

        if(mode == SOFTTRIGGER)
        {
            mCamera.RegisterConfiguration( new CSoftwareTriggerConfiguration,
                                               RegistrationMode_ReplaceAll,
                                               Cleanup_Delete);
        }

        // For demonstration purposes only, register another image event handler.
        mCamera.RegisterImageEventHandler( this, RegistrationMode_Append, Cleanup_None);

        // Open the camera device.
        mCamera.Open();

        /*mCamera.TriggerMode.SetValue(1);*/

//        CIntegerParameter heartbeat(mCamera.GetTLNodeMap(), "HeartbeatTimeout");
//        heartbeat.TrySetValue(50000000, IntegerValueCorrection_Nearest);  // set to 1000 ms timeout if writable

        std::cout << CStringPtr( nodemap->GetNode( "DeviceVendorName") )->GetValue() << endl;
    }
    catch (const GenericException &e)
    {
        // Error handling.
        cerr << "An exception occurred." << endl
            << e.GetDescription() << endl;

        // Remove left over characters from input buffer.
        cin.ignore(cin.rdbuf()->in_avail());

        return false;
    }

    return true;
}

//===================================================================================================================================
//函数名称：void BaslerCameraGigI::StartCamera()
//函数功能：
//===================================================================================================================================
void BaslerCameraGigI::StartCamera()
{
    if ( !CameraRuning && mCamera.CanWaitForFrameTriggerReady())
    {
        // Start the grabbing using the grab loop thread, by setting the grabLoopType parameter
        // to GrabLoop_ProvidedByInstantCamera. The grab results are delivered to the image event handlers.
        // The GrabStrategy_OneByOne default grab strategy is used.
        mCamera.StartGrabbing( GrabStrategy_OneByOne, GrabLoop_ProvidedByInstantCamera);
        CameraRuning = true;
        cout << "StartCamera" << endl;
    }
}

//===================================================================================================================================
//函数名称：void BaslerCameraGigI::StopCamera()
//函数功能：
//===================================================================================================================================
void BaslerCameraGigI::StopCamera()
{
    mCamera.StopGrabbing();
    CameraRuning = false;
}

//===================================================================================================================================
//函数名称：void BaslerCameraGigI::CloseCamera()
//函数功能：
//===================================================================================================================================
void BaslerCameraGigI::CloseCamera()
{
    PylonTerminate();
}

void BaslerCameraGigI::TakeAShot()
{
    SetSoftTrigger();

    qDebug().noquote()<<_sn<<"开始拍摄!"<<QTime::currentTime().toString("hh:mm:ss--zzz");
    timer->start();
}

void BaslerCameraGigI::SetExposureTime(int t)
{
    SetFloatPara("ExposureTimeAbs", t);
}

//===================================================================================================================================
//函数名称：
//函数功能：
//func : Get an Float parameter
//nameNode : [in] the Name Node
//para : [out] the vaule of parameter
 //===================================================================================================================================
bool BaslerCameraGigI::GetFloatPara(const char* nameNode, double& para)
{
    try{
        if( nodemap ) {
            CFloatPtr tmp( nodemap->GetNode( nameNode ));

            para = tmp->GetMin();

            return true;
        }
    } catch (const GenericException &e) {
        std::cout << e.GetDescription()<<std::endl;
    }

    return false;
}

//===================================================================================================================================
//函数名称：
//函数功能：
//* func : Get an Integer parameter
//* nameNode : [in] the Name Node
//* para : [out] the vaule of parameter
//===================================================================================================================================
bool BaslerCameraGigI::GetIntPara(const char* nameNode, int64_t& para)
{
    try {
        if( nodemap ) {
            CIntegerPtr tmp( nodemap->GetNode( nameNode));
            para = tmp->GetValue();
            return true;
        }
    } catch (const GenericException &e) {
         std::cout << e.GetDescription()<<std::endl;
    }
    return false;
}

//===================================================================================================================================
//函数名称：
//函数功能：
//* func : Get an enumeration parameter
//* nameNode : [in] the Name Node
//* para : [out] the vaule of parameter
//===================================================================================================================================
bool BaslerCameraGigI::GetStringPara(const char* nameNode, string& para)
{
    try {
        if( nodemap ) {
            CEnumerationPtr tmp( nodemap->GetNode( nameNode));
            para = tmp->ToString();

            return true;
        }
    } catch (const GenericException &e) {
        std::cout << e.GetDescription()<<std::endl;
   }
    return false;
}

//===================================================================================================================================
//函数名称：
//函数功能：
//* func : Set an Float parameter
//* nameNode : [in] the Name Node
//* para : [in] the vaule of parameter
//===================================================================================================================================
bool BaslerCameraGigI::SetFloatPara(const char* nameNode, double para)
{
    try {
        if( nodemap ) {
            CFloatPtr tmp( nodemap->GetNode( nameNode));

            if ( IsWritable(tmp) ) {
                tmp->SetValue(para);
                return true;
            } else {
                return false;
            }
        }
    } catch (const GenericException &e) {
        std::cout << e.GetDescription() << std::endl;
   }
    return false;
}

//===================================================================================================================================
//函数名称：
//函数功能：
//* func : Set an Integer parameter
//* nameNode : [in] the Name Node
//* para : [in] the vaule of parameter
//===================================================================================================================================
bool BaslerCameraGigI::SetIntPara(const char* nameNode, int64_t para)
{
    try {
        if( nodemap ) {
            CIntegerPtr tmp( nodemap->GetNode( nameNode ));
            if ( IsWritable(tmp) ) {
                para = Adjust(para, tmp->GetMin(), tmp->GetMax(), tmp->GetInc());       //将para做窗口化修正
                tmp->SetValue(para);
                return true;
            } else {
                return false;
            }
        }
    } catch (const GenericException &e) {
        std::cout << e.GetDescription()<<std::endl;
    }
    return false;
}

//===================================================================================================================================
//函数名称：
//函数功能：
//* func : Set an enumeration parameter
//* nameNode : [in] the Name Node
//* para : [in] the vaule of parameter
//===================================================================================================================================
bool BaslerCameraGigI::SetStringPara(const char* nameNode, String_t para)
{
    try {
        if( nodemap ) {
            CEnumerationPtr tmp( nodemap->GetNode( nameNode ));

            if ( IsWritable(tmp) && IsAvailable( tmp->GetEntryByName( para) ) ) {

                tmp->FromString(para);

                return true;
            } else {
                return false;
            }
        }
    } catch (const GenericException &e) {
        std::cout << e.GetDescription()<<std::endl;
   }
    return false;
}

//===================================================================================================================================
//函数名称：
//函数功能：
//===================================================================================================================================
bool BaslerCameraGigI::SetBooleanPara(const char *nameNode, bool para)
{
    try {
        if( nodemap ) {
            CBooleanPtr tmp( nodemap->GetNode( nameNode ));

            if ( IsWritable(tmp) ) {

                tmp->SetValue(para);

                return true;
            } else {
                return false;
            }
        }
    } catch (const GenericException &e) {
        std::cout << e.GetDescription()<<std::endl;
   }
    return false;
}

//===================================================================================================================================
//函数名称：
//函数功能：
//* func : Excuting an ICommand
//* nameNode : [in] the Node Name
//===================================================================================================================================
bool BaslerCameraGigI::SetCmd(const char* nameNode)
{
    try {
        if( nodemap ) {
            CCommandPtr tmp( nodemap->GetNode( nameNode ));
            if( IsWritable(tmp)) {
                tmp->Execute();
            }

        }
    } catch (const GenericException &e) {
        std::cout << e.GetDescription()<<std::endl;
   }
    return false;
}

//===================================================================================================================================
//函数名称：
//函数功能：
//===================================================================================================================================
bool BaslerCameraGigI::SetSoftTrigger()
{

    try {
        if(mCamera.CanWaitForFrameTriggerReady()) {
            SetCmd("TriggerSoftware");
            return true;
        }
    } catch (const GenericException &e) {
        std::cout << e.GetDescription()<<std::endl;
        return false;
    }

    return false;
}

void BaslerCameraGigI::CameraInit(const CDeviceInfo &Di)
{
//    qDebug()<<"BaslerCameraGigI:"<<QThread::currentThreadId();
    CTlFactory& tlFactory = CTlFactory::GetInstance();

    mCamera.Attach( tlFactory.CreateDevice( Di ) );

    nodemap = &mCamera.GetNodeMap();
    m_Mode = FREERUN;
    CameraRuning = false;

    OpenCamera();      //打开相机
    SetStringPara("TriggerMode", "On");
    SetStringPara("TriggerSource", "Software");
//    SetFloatPara("ExposureTimeAbs", EXPOSURE_TIME);
    StartCamera();

    timer = new QTimer;
    timer->setSingleShot(true);
    timer->setInterval(100);
    connect(timer, &QTimer::timeout, [this](){
        if(_CameraImg.count() > 0){
            qDebug().noquote()<<_sn<<"发送图片 !"<<QTime::currentTime().toString("hh:mm:ss--zzz");
            emit shot_done(_CameraImg.first(), _sn);
            _CameraImg.removeFirst();
        }
        else{
            timer->start();
//            qDebug()<<"未收到图片，等待再收 !";
        }
    });
}

//===================================================================================================================================
//函数名称：Adjust
//函数功能：修正val，使其在minimum和maximum之间，且在定义好的窗口内被四舍五入
//* Adjust value to make it comply with range and increment passed.
//* The parameter's minimum and maximum are always considered as valid values.
//* If the increment is larger than one, the returned value will be : min + (n * inc).
//* If the value doesn't meet these criteria, it will be rounded down to ensure compliance.
//===================================================================================================================================
int64_t BaslerCameraGigI::Adjust(int64_t val, int64_t minimum, int64_t maximum, int64_t inc)
{
    /* Check the input parameters. */
    if (inc <= 0) {
        /*Negative increments are invalid.*/
        throw LOGICAL_ERROR_EXCEPTION("Unexpected increment %d", inc);
    }

    if (minimum > maximum) {
        /* Minimum must not be bigger than or equal to the maximum.*/
        throw LOGICAL_ERROR_EXCEPTION("minimum bigger than maximum.");
    }

     /* Check the lower bound. */
    if (val < minimum) {
        return minimum;
    }

    /*Check the upper bound.*/
    if (val > maximum) {
        return maximum;
    }

    /*Check the increment.*/
    if (inc == 1) {
        /*Special case: all values are valid.*/
        return val;
    } else {
        /*The value must be min + (n * inc).*/
        /*Due to the integer division, the value will be rounded down.*/
        return minimum + ( ((val - minimum) / inc) * inc );
    }
}
