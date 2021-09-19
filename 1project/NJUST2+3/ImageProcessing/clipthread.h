#ifndef CLIPTHREAD_H
#define CLIPTHREAD_H

#include <QObject>
class ClipRun;
class ClipNetA;
class ClipNetB;

/********************************************************
 * @ClassName:  ClipThread
 * @Brief:      切割线程控制类,完成ClipNetA,ClipNetB,ClipRun
 *              三个切割子线程的控制,维护和数据传送,流程是:
 *              拍摄数据导入后,先分别送入完成ClipNetA和完成ClipNetB
 *              之后再送入若干个临时线程ClipRun,计算完成后将所得数据
 *              传输至合并线程
 * @date        2020/08/31
 ********************************************************/
class ClipThread : public QObject
{
    Q_OBJECT
public:
    explicit ClipThread(QObject *parent = nullptr);
    void reload();

signals:
    void one_group_ready(int group_index,                               /*照片组编号*/
                         std::vector<cv::Mat> maskDeteOut,              /*当下组MASK图片，供调用*/
                         std::vector<cv::Mat> weldDeteOut,              /*当下组焊缝图片，供调用*/
                         std::vector<std::vector<std::string>> Strings, /*当下组字符信息，供调用*/
                         std::vector<std::vector<int>> stringmasknum ,   /*当下组编号信息，供调用*/
                         std::vector<int> NUMTOTALL
                         );

    void start_net_compute(std::vector<cv::Mat> DecImg);        //开始网络计算
    void net_load_ready();

public slots:
    void init();
    void add_one_group(std::vector<cv::Mat> DecImg);
    void one_pic_completed(int index, cv::Mat maskDeteOut_one, cv::Mat maskDeteOut_oneAbove, cv::Mat weldDeteOut_one, std::vector<std::string> alltextout, std::vector<cv::Point2f> centerpointout);
    void netA_ready(std::vector<cv::Mat> MaskOut,
                    std::vector<std::vector<QPointF>> centerpointout,
                    std::vector<std::vector<std::string>> alltextout);
    void netB_ready(std::vector<cv::Mat> WeldOut);
    void netA_load_ready();
    void netB_load_ready();

public:


    void togetnumber(std::vector<int>& NUMTOTALL, std::vector<cv::Mat>& gmask, std::vector<cv::Mat> & gweld);
    std::vector<std::vector<int>> matchStrMask(std::vector<cv::Mat> numberedMask, std::vector<std::vector<std::string>> StringIn, std::vector<std::vector<cv::Point2f>> PointsIn);


private:
    QThread *thread;
    QMutex mutex;

    int number_need_to_add;      /* num of masks */
    int group_index;           /*照片组编号*/


//    std::vector<cv::Mat> asdasd;

    std::vector<std::vector<cv::Point2f>> centerpointall;

    std::vector<int> NUMTOTALL;

    QVector<ClipRun*> clipRuns;
    QVector<bool> flags;
    std::vector<cv::Mat> maskDeteOut;               /*当下组MASK图片，供调用*/
    std::vector<cv::Mat> weldDeteOut;               /*当下组焊缝图片，供调用*/
    std::vector<std::vector<std::string>> Strings;  /*当下组字符信息，供调用*/
    std::vector<std::vector<int>> stringmasknum;    /*当下组编号信息，供调用*/

    std::vector<cv::Mat> maskDeteAbove;

    void deal_one_group();
    QList<std::vector<cv::Mat> > buffer;

    ClipNetA *clipNetA;
    ClipNetB *clipNetB;
    bool netA_flag;
    bool netB_flag;
    bool netA_load_flag;
    bool netB_load_flag;

    std::vector<cv::Mat> _MaskOut;
    std::vector<std::vector<QPointF> > _centerpointout;
    std::vector<std::vector<std::string> > _alltextout;
    std::vector<cv::Mat> _WeldOut;
    void check_start_cliprun();

};

#endif // CLIPTHREAD_H
