#include "matchthread.h"
#include "matchrun.h"
//#include "Compute/Match.h"
MatchThread::MatchThread(QObject *parent) : QObject(parent)
{
    thread = new QThread;
    this->moveToThread(thread);
    thread->start();

    QTimer::singleShot(0, this, SLOT(init()));
}

void MatchThread::init()
{
    //插入匹配初始化代码

}

/*
 * @Brief:  添加一组工件
 * @Return: NULL
 */
void MatchThread::add_one_workpiece(int workpiece_index, std::vector<cv::Point3d> pcd, std::string name,std::vector<cv::Point3d> pcdmask)
{
//    QMutexLocker m_lock(&mutex);
    qDebug()<<"发送开始匹配信号";
    emit match_started(workpiece_index);        //发送开始匹配信号    

    MatchRun *matchRun = new MatchRun(workpiece_index, pcd, name);
    connect(matchRun, &MatchRun::send_weldData, this, &MatchThread::send_weldData);
    connect(matchRun, &MatchRun::match_finished, this, &MatchThread::match_finished);
    QThreadPool::globalInstance()->start(matchRun);
}

/*
 * @Brief:  重新匹配
 * @Return: NULL
 */
void MatchThread::send_to_rematch(int workpiece_index, std::vector<cv::Point3d> pcd, std::string name)
{
    qDebug()<<"收到重新匹配任务"<<workpiece_index << QString::fromStdString(name);
    std::vector<cv::Point3d> pcdmask;
    add_one_workpiece(workpiece_index, pcd, name, pcdmask );
}
