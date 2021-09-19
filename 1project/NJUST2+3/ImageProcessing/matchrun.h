#ifndef MATCHRUN_H
#define MATCHRUN_H

#include <QObject>
#include <QRunnable>
class PointCloud_match;

/********************************************************
 * @ClassName:  MatchRun
 * @Brief:      单个工件的匹配线程
 * @date        2020/08/31
 ********************************************************/
class MatchRun : public QObject, public QRunnable
{
    Q_OBJECT
public:
    explicit MatchRun(int workpiece_index, std::vector<cv::Point3d> pcd, std::string name, QObject *parent = nullptr);

    void run();

signals:
    void send_weldData(std::vector<std::vector<cv::Point3d>>, std::string);
    void match_finished(int workpiece_index, std::string name, bool flag);

public slots:

private:
    int _workpiece_index;
    std::vector<cv::Point3d> _pcd;
    std::string _name;
    PointCloud_match      *PM;
};

#endif // MATCHRUN_H
