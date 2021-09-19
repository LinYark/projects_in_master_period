#ifndef MATCHTHREAD_H
#define MATCHTHREAD_H

#include <QObject>
class PointCloud_match;

/********************************************************
 * @ClassName:  MatchThread
 * @Brief:      匹配线程,将HebingThread或者CheckDialog发来的
 *              数据进行匹配计算,后将计算结果发送至ui和ServerCtrl线程
 *              线程内可以生成MatchRun临时线程,完成多工件同时计算
 * @date        2020/08/31
 ********************************************************/
class MatchThread : public QObject
{
    Q_OBJECT
public:
    explicit MatchThread(QObject *parent = nullptr);

signals:
    void send_weldData(std::vector<std::vector<cv::Point3d>>, std::string);     //发送焊缝数据
    void match_started(int workpiece_index);                                    //开始匹配信号
    void match_finished(int workpiece_index, std::string name, bool flag);      //结束匹配信号

public slots:
    void init();
    void add_one_workpiece(int workpiece_index,                             /*工件编号*/
                           std::vector<cv::Point3d> pcd,                    /*该工件点云数据信息pointCloud*/
                           std::string name ,                               /*工件对应的字符*/
                           std::vector<cv::Point3d> pcdmask
                           );

    void send_to_rematch(int workpiece_index,           /*工件编号*/
                         std::vector<cv::Point3d> pcd,      /*该工件点云数据信息pointCloud*/
                         std::string name               /*工件对应的字符*/
                         );

private:
    QThread *thread;
    QMutex mutex;

    PointCloud_match      *PM;

};

#endif // MATCHTHREAD_H
