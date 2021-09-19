#include "clipthread.h"
#include "cliprun.h"
#include "Server/parameter.h"
#include "clipneta.h"
#include "clipnetb.h"

using namespace  std;
using namespace  cv;

ClipThread::ClipThread(QObject *parent) : QObject(parent)
{
    thread = new QThread;
    this->moveToThread(thread);
    thread->start();

    //创建两个常备线程
    netA_load_flag = false;
    netB_load_flag = false;
    clipNetA = new ClipNetA;
    connect(this, &ClipThread::start_net_compute, clipNetA, &ClipNetA::add_one_group);
    connect(clipNetA, &ClipNetA::netA_ready, this, &ClipThread::netA_ready);
    connect(clipNetA, &ClipNetA::netA_load_ready, this, &ClipThread::netA_load_ready);
    clipNetB = new ClipNetB;
    connect(this, &ClipThread::start_net_compute, clipNetB, &ClipNetB::add_one_group);
    connect(clipNetB, &ClipNetB::netB_ready, this, &ClipThread::netB_ready);
    connect(clipNetB, &ClipNetB::netB_load_ready, this, &ClipThread::netB_load_ready);

    QTimer::singleShot(0, this, SLOT(init()));
}

void ClipThread::reload()
{
    QMutexLocker m_lock(&mutex);
    group_index = 0;

    qDebug() << "ClipThread::reload()" << PP->compute_path();
    qDeleteAll(clipRuns);
    clipRuns.clear();
    for (uint i = 0; i < PP->cameraMap().size(); ++i) {
        ClipRun *clipRun = new ClipRun;
        connect(clipRun, &ClipRun::run_finished, this, &ClipThread::one_pic_completed, Qt::QueuedConnection);
        clipRuns.append(clipRun);
    }
    number_need_to_add = 0;
    qDebug()<<"创建"<<PP->cameraMap().size()<<"个ClipRun子线程";
}

void ClipThread::init()
{
    //    if(PP->check_training_model()){

    //    }
}

void ClipThread::add_one_group(std::vector<cv::Mat> DecImg)
{
    //    qDebug()<<"add_one_group"<<buffer.size();
    buffer.append(DecImg);
    if(buffer.size() == 1){
        deal_one_group();
    }
}

void ClipThread::one_pic_completed(int index, cv::Mat maskDeteOut_one, cv::Mat maskDeteOut_oneAbove, cv::Mat weldDeteOut_one, std::vector<std::string> alltextout, std::vector<cv::Point2f> centerpointout)
{
    //    QMutexLocker m_lock(&mutex);

//    imwrite("/home/jiangnan/图片/" + std::to_string(index) + "kk.jpg", weldDeteOut_one);

    if(index >= flags.count()){
        qDebug()<<"ClipThread::one_pic_completed() error!";
        return;
    }

    //    for (int r = 0; r <maskDeteOut_one.rows; r++)
    //    {
    //        for (int c = 0; c < maskDeteOut_one.cols; c++)
    //        {
    //            int v = maskDeteOut_one.at<uchar>(r, c);
    //            if (v>0&&v!=255)
    //            {
    //                qDebug()<<"mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm!";
    //                break;
    //            }
    //        }
    //    }


    flags[index] = true;
    maskDeteOut[index] = maskDeteOut_one;
    maskDeteAbove[index] = maskDeteOut_oneAbove;
    weldDeteOut[index] = weldDeteOut_one;
    Strings[index] = alltextout;
    centerpointall[index] = centerpointout;

    bool result = true;
    foreach (bool flag, flags) {
        if(flag == false){
            result = false;
            break;
        }
    }

    //    qDebug()<<"group_index"<<group_index<<"\tflags"<<flags;
    if(result){
        togetnumber(NUMTOTALL,maskDeteOut, weldDeteOut);
        //        qDebug()<<"NUMTOTALL                         !"<<NUMTOTALL[0];
        //        qDebug()<<"NUMTOTALL                         !"<<NUMTOTALL[1];
        //        qDebug()<<"NUMTOTALL                         !"<<NUMTOTALL[2];
        stringmasknum = matchStrMask(maskDeteOut, Strings, centerpointall);
        qDebug()<<"发送one_group_ready信号"<<group_index;
        emit one_group_ready(group_index, maskDeteOut, weldDeteOut, Strings, stringmasknum, NUMTOTALL);

        buffer.removeFirst();


        if(!buffer.isEmpty()){
            deal_one_group();
        }
        //        qDebug()<<"one_pic_completed执行完毕"<<group_index;
    }


}

void ClipThread::netA_ready(std::vector<cv::Mat> MaskOut, std::vector<std::vector<QPointF> > centerpointout,
                            std::vector<std::vector<std::string> > alltextout)
{
    QMutexLocker m_lock(&mutex);
    netA_flag = true;
    _MaskOut = MaskOut;
    _centerpointout = centerpointout;
    _alltextout = alltextout;

    if(netB_flag){
        check_start_cliprun();
    }

}

void ClipThread::netB_ready(std::vector<cv::Mat> WeldOut)
{
    QMutexLocker m_lock(&mutex);
    netB_flag = true;
    _WeldOut = WeldOut;

    if(netA_flag){
        check_start_cliprun();
    }
}

void ClipThread::netA_load_ready()
{
    netA_load_flag = true;
    if(netB_load_flag){
        emit net_load_ready();
    }
}

void ClipThread::netB_load_ready()
{
    netB_load_flag = true;
    if(netA_load_flag){
        emit net_load_ready();
    }
}

void ClipThread::deal_one_group()
{
    if(!buffer.isEmpty()){
        //        deal_one_group(buffer.first());
        netA_flag = false;
        netB_flag = false;
        emit start_net_compute(buffer.first());
    }
}

void ClipThread::check_start_cliprun()
{
    qDebug()<<" [" + QTime::currentTime().toString() + "]"+"netA & netB completed!!" << group_index;
    group_index++;

    int n = buffer.first().size();
    flags.clear();
    flags.resize(n);
    for (int i = 0; i < n; ++i) {
        flags[i] = false;
    }
    maskDeteOut.clear();
    maskDeteOut.resize(n);
    maskDeteAbove.clear();
    maskDeteAbove.resize(n);
    weldDeteOut.clear();
    weldDeteOut.resize(n);
    Strings.clear();
    Strings.resize(n);
    centerpointall.clear();
    centerpointall.resize(n);
    stringmasknum.clear();
    NUMTOTALL.clear();
    NUMTOTALL.resize(n);

    for (int i = 0; i < n; ++i) {
        clipRuns.at(i)->add_one_pic(i, buffer.first().at(i), _MaskOut, _centerpointout, _alltextout, _WeldOut);
    }
}

void ClipThread::togetnumber(std::vector<int>& NUMTOTALL, vector<Mat>& gmask, vector<Mat> & gweld)
{
    Q_ASSERT(gmask.size() == gweld.size());

    for (int j = 0; j < gmask.size(); j++) //循环次数 = 一组图片数
    {

        Mat maskDeteAbove0 = maskDeteAbove[j];
        Mat MASKAbove_16;
        maskDeteAbove0.convertTo(MASKAbove_16, CV_16UC1);
        //        Mat xdfnxfyj;
        //        xdfnxfyj = maskDeteAbove0 * 255;
        //        imwrite("/home/jiangnan/图片/0612/0.bmp", xdfnxfyj);

        //    qDebug() << "MASKAboveStack_16.type():           "<<MASKAboveStack_16.type();


        Mat Mask = gmask[j];
        //        for (int r = 0; r < Mask.rows; r++)
        //        {
        //            for (int c = 0; c < Mask.cols; c++)
        //            {
        //                int v = Mask.at<uchar>(r, c);
        //                if (v>0&&v!=255)
        //                {
        //                    qDebug()<<"mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm!";
        //                    break;
        //                }
        //            }
        //        }
        //Mask = Mask * 255;
        //RemoveSmallRegion(Mask, Mask, 10000, 1, 1);
//        std::vector<std::vector<cv::Point>> contours1;
//        cv::findContours(Mask, contours1, cv::RETR_LIST, cv::CHAIN_APPROX_NONE);
//        contours1.erase(std::remove_if(contours1.begin(), contours1.end(), [](const std::vector<cv::Point>& c){return cv::contourArea(c) < 10000; }), contours1.end());
//        Mask.setTo(0);
//        cv::drawContours(Mask, contours1, -1, cv::Scalar(255), cv::FILLED);



        Mat Numbered_Mask; //用于存储编号后的Mask检测图
        int numbermask = connectedComponents(Mask, Numbered_Mask, 8, CV_16UC1) - 1;

        //        for (int r = 0; r < Numbered_Mask.rows; r++)
        //        {
        //            for (int c = 0; c < Numbered_Mask.cols; c++)
        //            {
        //                int v = Numbered_Mask.at<ushort>(r, c);
        //                if (v>10)
        //                {
        //                    qDebug()<<"mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm!";
        //                    break;
        //                }
        //            }
        //        }

        Mat convert = Mat::zeros(1200, 1920, CV_16UC1);
        for (int r = 0; r < convert.rows; r++)
        {
            for (int c = 0; c < convert.cols; c++)
            {
                unsigned short v = Numbered_Mask.at<unsigned short>(r, c);
                if (v > 0)
                    convert.at<unsigned short>(r, c) = 1;
            }
        }
        //        qDebug()<<"number_need_to_add                         !"<<number_need_to_add;
        convert = convert * number_need_to_add;
        Numbered_Mask += convert; //每张图片加上前面的最大编号，每张图片上的Mask才能得到在整个所有图片中的正确的编号

        //        for (int r = 0; r < Numbered_Mask.rows; r++)
        //        {
        //            for (int c = 0; c < Numbered_Mask.cols; c++)
        //            {
        //                int v = Numbered_Mask.at<ushort>(r, c);
        //                if (v>255)
        //                {
        //                    qDebug()<<"mmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmmm!";
        //                    break;
        //                }
        //            }
        //        }

        gmask[j] = Numbered_Mask;

        QString str = PP->save_path() + "/" + PP->str_time() + "/MASK/camera" + QString::number(j+1) + "-" + QString::number(group_index) + ".jpg";


        Mat aa;
        Numbered_Mask.convertTo(aa, CV_8UC1);
        vector<int> compression_params;
        compression_params.push_back(IMWRITE_JPEG_QUALITY);  //选择jpeg
        compression_params.push_back(75); //在这个填入你要的图片质量
        cv::imwrite(str.toStdString(), aa*255,compression_params);                //保存数据



        number_need_to_add += numbermask; //更新最大编号

        NUMTOTALL[j]=number_need_to_add;
        //        qDebug()<<"NUMTOTALL                         !"<<number_need_to_add;

        Mat hanfeng0 = gweld[j];
//         imwrite("/home/jiangnan/图片/" + std::to_string(j) + "pp.jpg", hanfeng0);
        Mat hanfeng = hanfeng0.mul(Mask / 255);
        Mat hanfeng_16; //从8位转化为16位，与16位的mask图适配，进行点乘操作
        Mat Numbered_hanfeng; //用于存储编号后的焊缝检测图
        //RemoveSmallRegion(hanfeng, hanfeng, 200, 1, 1);
//        std::vector<std::vector<cv::Point>> contours2;
//        cv::findContours(hanfeng, contours2, cv::RETR_LIST, cv::CHAIN_APPROX_NONE);
//        contours2.erase(std::remove_if(contours2.begin(), contours2.end(), [](const std::vector<cv::Point>& c){return cv::contourArea(c) < 5; }), contours2.end());
//        hanfeng.setTo(0);
//        cv::drawContours(hanfeng, contours2, -1, cv::Scalar(255), cv::FILLED);

//        imwrite("/home/jiangnan/图片/" + std::to_string(j) + "xx.jpg", hanfeng*255);


        Mat kkk = Mat::zeros(1200, 1920, CV_8UC1);
        for (int r = 0; r < hanfeng.rows; r++)
        {
            for (int c = 0; c < hanfeng.cols; c++)
            {
                int v = hanfeng.at<uchar>(r, c);
                if (v > 0)
                {
                    kkk.at<uchar>(r, c) = 1;
                }
            }
        }
        kkk.convertTo(hanfeng_16, CV_16UC1); //8位图转16位


        Numbered_hanfeng = Numbered_Mask.mul(hanfeng_16); //点乘得到编号信息
        Mat bb;
        Numbered_hanfeng.convertTo(bb, CV_8UC1);
//        imwrite("/home/jiangnan/图片/0612/2.bmp", bb*255);
        QString str1 = PP->save_path() + "/" + PP->str_time() + "/WELD/camera" + QString::number(j+1) + "-" + QString::number(group_index) + ".jpg";
        cv::imwrite(str1.toStdString(), bb*255,compression_params);                //保存数据
        Numbered_hanfeng = Numbered_hanfeng.mul(MASKAbove_16);
        Mat cc;
        Numbered_hanfeng.convertTo(cc, CV_8UC1);
        QString str2 = PP->save_path() + "/" + PP->str_time() + "/WELD_MINUS/camera" + QString::number(j+1) + "-" + QString::number(group_index) + ".jpg";
        cv::imwrite(str2.toStdString(), cc*255,compression_params);                //保存数据
        //        imwrite("/home/jiangnan/图片/0612/3.bmp", cc*255);
        gweld[j] = Numbered_hanfeng;
//        Mat lop = gweld[j];
//        Mat pop = gmask[j];

//        qDebug()<<"gweld[j].               "<<lop.type();
//        qDebug()<<"gmask[j] .            "<<pop.type();

    }

}

vector<vector<int>> ClipThread::matchStrMask(vector<Mat> numberedMask, vector<vector<string>> StringIn, vector<vector<Point2f>> PointsIn)
{
    vector<vector<int>> StrToMask;
    StrToMask.resize(StringIn.size());
    for (int i = 0; i < StringIn.size(); i++)
    {
        for (int j = 0; j < StringIn[i].size(); j++)
        {
            StrToMask[i].resize(StringIn[i].size());
            int pixel = numberedMask[i].at<ushort>(PointsIn[i][j]); //字符所在的位置，读图像中坐标的像素值
            StrToMask[i][j] = pixel;
        }
    }
    return StrToMask;
}
