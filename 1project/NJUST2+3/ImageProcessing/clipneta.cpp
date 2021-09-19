#include "clipneta.h"
#include "Server/parameter.h"

#ifdef FULLY_BUILD
#include "Compute/mask_detector.h"
#include "Compute/weld_detector.h"
#include "Compute/character.h"
#endif

ClipNetA::ClipNetA(QObject *parent) : QObject(parent)
{
    thread = new QThread;
    this->moveToThread(thread);
    thread->start();

    QTimer::singleShot(0, this, SLOT(init()));
}

void ClipNetA::init()
{
    if(PP->check_training_model()){

#ifdef FULLY_BUILD
        //插入网络A初始化代码
        Md = new Mask_Detector;
        CCC = new character;
        Md->Net3();
        qDebug() << "Md->Net3()";
        CCC->load_east_net();
        qDebug() << "CCC->load_east_net();";
        CCC->load_ssd();
        qDebug() << "CCC->load_ssd()";
#endif

        emit netA_load_ready();
    }
}

void ClipNetA::add_one_group(std::vector<cv::Mat> DecImg)
{
    QMutexLocker m_lock(&mutex);
          //替换成网络A执行代码

    std::vector<cv::Mat> MaskOut;
    std::vector<std::vector<QPointF>> centerpointout;
    std::vector<std::vector<std::string>> alltextout;

#ifdef FULLY_BUILD
    double tMaskProcEnd,tWPProcStart;
    //    qDebug()<<"ClipRun2::run()___index:     "<<index<<endl;
    double End,Start;
    Start = (double)getTickCount();
    for(int i=0;i<DecImg.size();i++)
    {
        tWPProcStart = (double)getTickCount();
        Mat maskDeteOut_one0 = Md->Img_detector(DecImg[i]);
        tMaskProcEnd = (double)getTickCount();
//        qDebug()<<"ClipNetA"<<i<<"     maskDeteOut_one0 time:"<<(tMaskProcEnd - tWPProcStart) * 1000 / (getTickFrequency())<<"ms\n";
        MaskOut.push_back(maskDeteOut_one0);


        //字符串检测----------------------------------------------
        vector<string> alltextout0;
        vector<Point2f> centerpointout0;
        vector<QPointF> centerpointout1;


        Mat origiRoi=takeROI(i, DecImg[i]);
        tWPProcStart = (double)getTickCount();
        CCC->Chara_Detec(origiRoi, alltextout0, centerpointout0);
        tMaskProcEnd = (double)getTickCount();
//        qDebug()<<"ClipNetA"<<i<<"     Chara_Detec time:"<<(tMaskProcEnd - tWPProcStart) * 1000 / (getTickFrequency())<<"ms\n";
        for(int j=0;j<centerpointout0.size();j++)
        {
            centerpointout1.push_back(QPointF(centerpointout0[j].x,centerpointout0[j].y));
        }
        centerpointout.push_back(centerpointout1);
        alltextout.push_back(alltextout0);

//        for(int kk = 0; kk < alltextout0.size(); kk++)
//        {
//            String a = alltextout0[kk];
//            qDebug()<<"alltextout0"<<QString::fromStdString(a);

//        }


    }

    End = (double)getTickCount();
//    qDebug()<<"CCCCCCCCCCCCCCCCCCCCCCCC all time:"<<(End - Start) * 1000 / (getTickFrequency())<<"ms\n";
#else
    QThread::sleep(2);
#endif

    emit netA_ready(MaskOut, centerpointout, alltextout);
}

cv::Mat ClipNetA::takeROI(int _index, cv::Mat imgI)
{
    vector<int> cam_map= PP->cameraMap();
    int usedCanNum = PP->cameraMap().size();
    cv::Mat Roi = cv::Mat::zeros(imgI.rows, imgI.cols, imgI.type());

    if (cam_map[_index] == 1)
    {
        //            10,10,1900,1180---------------------new
        cv::Rect rect(10,10,1900,1180);
        cv::Mat Tmp = imgI(rect);
        cv::Mat src = cv::Mat::zeros(imgI.rows, imgI.cols, imgI.type());
        cv::Mat ROI = src(rect);
        Tmp.copyTo(ROI);
        Roi = src.clone();
    }

    else if (cam_map[_index] == 2)
    {

        cv::Rect rect(10,10,1900,1180);
        cv::Mat Tmp = imgI(rect);
        cv::Mat src = cv::Mat::zeros(imgI.rows, imgI.cols, imgI.type());
        cv::Mat ROI = src(rect);
        Tmp.copyTo(ROI);
        Roi = src.clone();
    }
    else if (cam_map[_index] == 3)
    {

        cv::Rect rect(610,10,1300,1180);
        cv::Mat Tmp = imgI(rect);
        cv::Mat src = cv::Mat::zeros(imgI.rows, imgI.cols, imgI.type());
        cv::Mat ROI = src(rect);
        Tmp.copyTo(ROI);
        Roi = src.clone();
    }
    else if (cam_map[_index] == 4)
    {

        cv::Rect rect(10,10,1220,1180);
        cv::Mat Tmp = imgI(rect);
        cv::Mat src = cv::Mat::zeros(imgI.rows, imgI.cols, imgI.type());
        cv::Mat ROI = src(rect);
        Tmp.copyTo(ROI);
        Roi = src.clone();
    }
    else if (cam_map[_index] == 5)
    {

        cv::Rect rect(10,10,1900,1180);
        cv::Mat Tmp = imgI(rect);
        cv::Mat src = cv::Mat::zeros(imgI.rows, imgI.cols, imgI.type());
        cv::Mat ROI = src(rect);
        Tmp.copyTo(ROI);
        Roi = src.clone();
    }
    else if (cam_map[_index] == 6)
    {

        cv::Rect rect(220,10,1610,1180);
        cv::Mat Tmp = imgI(rect);
        cv::Mat src = cv::Mat::zeros(imgI.rows, imgI.cols, imgI.type());
        cv::Mat ROI = src(rect);
        Tmp.copyTo(ROI);
        Roi = src.clone();
    }
    else
    {
        //            10,10,1900,1180--------------------new
        cv::Rect rect(PP->ROI_last().x(), PP->ROI_last().y(), PP->ROI_last().width(), PP->ROI_last().height());
        cv::Mat Tmp = imgI(rect);
        cv::Mat src = cv::Mat::zeros(imgI.rows, imgI.cols, imgI.type());
        cv::Mat ROI = src(rect);
        Tmp.copyTo(ROI);
        Roi = src.clone();
    }

    return Roi;
}
