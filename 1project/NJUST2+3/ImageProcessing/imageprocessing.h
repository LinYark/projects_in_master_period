#ifndef IMAGEPROCESSING_H
#define IMAGEPROCESSING_H

#include <QThread>

class Compute;

class ImageProcessing : public QThread
{
    Q_OBJECT
public:
    explicit ImageProcessing(QObject *parent = nullptr);

    void run();

    void start_test();
    void end_test();
    std::vector<cv::Mat> show_pics(int num);

signals:
    void compute_progress(double);   //发送计算进度（百分比）
    void update_info(QString);      //更新文本信息

//    void SendWeldseamData();
    void show_mesh_data(QVector<QVector4D>,QStringList,QVector<int>);       //第一次字符校验的结果
    void show_match_result(QStringList,QVector<int>);                       //第二次焊缝检验的结果

    void start_match(int);      //开始匹配信号
    void match_success(int);    //匹配成功信号
    void match_failed(int);     //匹配失败信号
    void match_pending(int);     //匹配待定信号
    void send_weldData(std::vector<std::vector<cv::Point3d>>, std::string);

public slots:
    void add_one_group(std::vector<cv::Mat> DecImg);
    void examine_char_complete(QStringList c_list, QVector<int> r_list);    //检验字符完成
    void P_weld_send();         //P焊缝发送

private:
    Compute *compute;

    bool start_test_flag;                 //开始测试标志
    bool end_test_flag;                 //结束测试标志
//    bool hebing_flag;
    bool match_flag;
    QList<std::vector<cv::Mat>> DecImgList;      //数据缓冲区
    int compute_group_times;            //完成计算相片组的次数
    QMutex mutex;
    void thread_init();
    void show_info(QString text);

};

#endif // IMAGEPROCESSING_H
