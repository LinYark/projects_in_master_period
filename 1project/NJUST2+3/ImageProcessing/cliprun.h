#ifndef CLIPRUN_H
#define CLIPRUN_H

#include <QObject>
class Weld_Processing;
class Mask_Detector;
class character;

/********************************************************
 * @ClassName:  ClipRun
 * @Brief:      切割子线程类,完成切割计算
 *              ClipNetA,ClipNetB两个线程计算过后的数据,输入
 *              ClipRun类,做后续计算
 * @date        2020/08/31
 ********************************************************/
class ClipRun : public QObject
{
    Q_OBJECT
public:
    explicit ClipRun(QObject *parent = nullptr);

    void add_one_pic(int index,
                     cv::Mat img,
                     std::vector<cv::Mat> _MaskOut,
                     std::vector<std::vector<QPointF> > _centerpointout,
                     std::vector<std::vector<std::string> > _alltextout,
                     std::vector<cv::Mat> _WeldOut);

signals:
    void add_one(int index,
                 cv::Mat img,
                 std::vector<cv::Mat> _MaskOut,
                 std::vector<std::vector<QPointF> > _centerpointout,
                 std::vector<std::vector<std::string> > _alltextout,
                 std::vector<cv::Mat> _WeldOut
                 );
    void run_finished(int index, cv::Mat maskDeteOut_one, cv::Mat maskDeteOut_oneAbove, cv::Mat weldDeteOut_one, std::vector<std::string> alltextout, std::vector<cv::Point2f> centerpointout);

public slots:
    void init();
    void run(int index, cv::Mat img,
             std::vector<cv::Mat> _MaskOut,
             std::vector<std::vector<QPointF> > _centerpointout,
             std::vector<std::vector<std::string> > _alltextout,
             std::vector<cv::Mat> _WeldOut
             );

private:
    QThread *thread;
    QMutex mutex;

    Weld_Processing       *WP;
    Mask_Detector         *Md;
    character             *CCC;
    cv::Mat takeROI(int _index, cv::Mat imgI);
};

#endif // CLIPRUN_H
