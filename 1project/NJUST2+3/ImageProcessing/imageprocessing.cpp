#include "imageprocessing.h"
#include "Compute/compute.h"
#include "Server/parameter.h"

ImageProcessing::ImageProcessing(QObject *parent) : QThread(parent)
{
    start_test_flag = false;
    end_test_flag = false;
    match_flag = false;
    compute_group_times = 0;

    this->start();
}

void ImageProcessing::thread_init()
{
    compute = new Compute;
    connect(compute, &Compute::send_weldData, this, &ImageProcessing::send_weldData);
    connect(compute, &Compute::start_match, this, &ImageProcessing::start_match);
    connect(compute, &Compute::match_success, this, &ImageProcessing::match_success);
    connect(compute, &Compute::match_failed, this, &ImageProcessing::match_failed);
    connect(compute, &Compute::match_pending, this, &ImageProcessing::match_pending);

    compute->init();
    start_test_flag = false;
    end_test_flag = false;
    compute_group_times = 0;
    DecImgList.clear();
}

void ImageProcessing::run()
{
    thread_init();

//    qDebug()<<"ImageProcessing:"<<QThread::currentThreadId();

    while (true) {
        //开始测试
        if(start_test_flag){
            start_test_flag = false;
            end_test_flag = false;
            match_flag = false;
            compute_group_times = 0;
            mutex.lock();
            DecImgList.clear();
            mutex.unlock();
//            show_info(tr("开启一次新的计算！"));
            compute->reload();
        }

        //计算单组图片数据

        while (!DecImgList.isEmpty()) {
            //计算次数累计
            compute_group_times++;
            show_info(tr("开始第%1次计算").arg(compute_group_times));

            mutex.lock();
            std::vector<cv::Mat> DecImg = DecImgList.takeFirst();
            mutex.unlock();

            //计算
            compute->deal_one_group(DecImg);
//            QThread::sleep(2);

            //进度及文本更新
            emit compute_progress(80.0 * compute_group_times / PP->camera_shot_times() );
            show_info(tr("第%1次计算完成").arg(compute_group_times));
            qDebug()<<tr("第%1次计算完成").arg(compute_group_times);
        }


        //计算整体（一定要在所有单体计算完成之后）
//        qDebug()<<"end_test_flag"<<end_test_flag<<"DecImgList"<<DecImgList.count()<<compute_group_times<<PP->camera_shot_times();
        if(end_test_flag && DecImgList.isEmpty() && compute_group_times == PP->camera_shot_times()){
            end_test_flag = false;
            compute_group_times = 0;    //计算次数清零

            show_info(tr("开始合并工件"));
            compute->deal_hebing();
            show_info(tr("合并工件完成"));

            //发送数据至图形显示
            emit compute_progress(90 );
            emit show_mesh_data(compute->point_cloud(),
                                compute->character_list(),
                                compute->result_list());
        }

        if(match_flag){
            match_flag = false;
            show_info(tr("开始匹配工件"));
            compute->deal_Match();
            emit compute_progress(100);
            show_info(tr("匹配工件完成"));
        }
    }
}

void ImageProcessing::add_one_group(std::vector<Mat> DecImg)
{
    QMutexLocker locker(&mutex);
    qDebug()<<"计算线程数据队列长度"<<DecImgList.count();
    DecImgList.append(DecImg);
}

void ImageProcessing::examine_char_complete(QStringList c_list, QVector<int> r_list)
{
    compute->set_character_list(c_list);
    compute->set_result_list(r_list);
    match_flag = true;
}

void ImageProcessing::P_weld_send()
{
    compute->P_weld_send();
}

void ImageProcessing::start_test()
{
    QMutexLocker locker(&mutex);
//    qDebug()<<"ImageDectot::start_test()";
    start_test_flag = true;
}

void ImageProcessing::end_test()
{
    QMutexLocker locker(&mutex);
//    qDebug()<<"ImageDectot::end_test()";
    end_test_flag = true;
}

std::vector<Mat> ImageProcessing::show_pics(int num)
{
    return compute->show_pics(num-1);
}

void ImageProcessing::show_info(QString text)
{
    qDebug() << text;
    emit update_info(" [" + QTime::currentTime().toString() + "]" + text);
}
