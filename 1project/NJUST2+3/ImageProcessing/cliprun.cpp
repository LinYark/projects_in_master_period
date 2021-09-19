#include "cliprun.h"
#include "Server/parameter.h"

#ifdef FULLY_BUILD
#include "Compute/mask_detector.h"
#include "Compute/weld_detector.h"
#include "Compute/character.h"
#endif

ClipRun::ClipRun(QObject *parent) : QObject(parent)
{
    thread = new QThread;
    this->moveToThread(thread);
    thread->start();

    connect(this, &ClipRun::add_one, this, &ClipRun::run);

    QTimer::singleShot(0, this, SLOT(init()));
}

void ClipRun::add_one_pic(int index,
                           cv::Mat img,
                           std::vector<cv::Mat> _MaskOut,
                           std::vector<std::vector<QPointF> > _centerpointout,
                           std::vector<std::vector<std::string> > _alltextout,
                           std::vector<cv::Mat> _WeldOut)
{

    emit add_one(index, img, _MaskOut, _centerpointout, _alltextout, _WeldOut);
}

void ClipRun::init()
{
    mutex.lock();

#ifdef FULLY_BUILD
    //插入初始化代码
    WP = new Weld_Processing;
    Md = new Mask_Detector;
    CCC = new character;
#endif

    mutex.unlock();
}

void ClipRun::run(int index,
                   cv::Mat img,
                   std::vector<cv::Mat> _MaskOut,
                   std::vector<std::vector<QPointF> > _centerpointout,
                   std::vector<std::vector<std::string> > _alltextout,
                   std::vector<cv::Mat> _WeldOut)
{
    QMutexLocker m_lock(&mutex);


    //替换成执行代码

    cv::Mat maskDeteOut_one;
    cv::Mat weldDeteOut_one;
    std::vector<std::string> alltextout;
    std::vector<cv::Point2f> centerpointout;

#ifdef FULLY_BUILD
    double End,Start;
    Start = (double)getTickCount();
    Mat maskDeteOut_one0=Md->mask_later(_MaskOut[index]);



//    for (int r = 0; r < maskDeteOut_one0.rows; r++)
//    {
//        for (int c = 0; c < maskDeteOut_one0.cols; c++)
//        {
//            int v = maskDeteOut_one0.at<uchar>(r, c);
//            if (v>0&&v!=255)
//            {
//                qDebug()<<v;
//                break;
//            }
//        }
//    }


    maskDeteOut_one = takeROI(index, maskDeteOut_one0);
    Mat maskDeteOut_oneAbove = Md->mask_laterplus(_MaskOut[index]);
    End = (double)getTickCount();
//    qDebug()<<index<<"mask_later all time:"<<(End - Start) * 1000 / (getTickFrequency())<<"ms\n";

    Start = (double)getTickCount();
    Mat weldDeteOut_one0=WP->weld_later(_WeldOut[index]);
//    imwrite("/home/jiangnan/图片/" + std::to_string(index) + "oo.jpg", weldDeteOut_one0);
    weldDeteOut_one = takeROI(index, weldDeteOut_one0);
//    imwrite("/home/jiangnan/图片/" +std::to_string(index) + ".jpg", weldDeteOut_one);
    End = (double)getTickCount();
//    qDebug()<<index<<"weld_later all time:"<<(End - Start) * 1000 / (getTickFrequency())<<"ms\n";

    alltextout=_alltextout[index];
    for(int i=0;i<_centerpointout[index].size();i++)
    {
        centerpointout.push_back(Point2f(_centerpointout[index][i].x(),_centerpointout[index][i].y()));
    }

//    qDebug()<<index<<"子线程结束！";

//    mutex.unlock();
#else
    QThread::sleep(2);
#endif

    emit run_finished(index, maskDeteOut_one, maskDeteOut_oneAbove, weldDeteOut_one, alltextout, centerpointout);


}

cv::Mat ClipRun::takeROI(int _index, cv::Mat imgI)
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
