#ifndef COMPUTE_H
#define COMPUTE_H

#define TEST 0

//程序包含----------------------------------------
#ifndef TEST
#include "Compute/gongjian_hebing.h"
#include "Compute/LunKuoPiPeiHY.h"
#include "Compute/mask_detector.h"
#include "Compute/weld_detector.h"
#include "Compute/points_detector.h"
#include "Compute/weld_only_detector.h"
#include "Compute/character.h"
#include "Compute/Match.h"
#include "testzbar.h"
#endif
//命名空间---------------------------------------------------------------
using namespace cv;
using namespace std;

class Weld_Processing;
class Mask_Detector;
class Points_Processing;
class Weld_Only_Processing;
class character;
class PointCloud_match;
struct point4d;

class Compute : public QObject
{
    Q_OBJECT
public:
    Compute();

    void init();            //计算初始化，程序执行期间只运行一次
    void reload();          //重新执行参数计算，每次计算都运行一次（12.22添加）
    QVector<QVector4D> point_cloud();   //返回计算的点云
    QStringList character_list();       //返回计算的字符串列表
    void set_character_list(QStringList list);          //设置字符串列表
    QVector<int> result_list();         //返回计算的识别结果列表
    void set_result_list(QVector<int> list);             //设置识别结果列表
    void deal_one_group(vector<Mat> DecImg);    //计算一次拍摄的一组图片
    void deal_hebing();       //做合并工作
    void deal_Match();       //做匹配工作
    vector<Mat> show_pics(int num);     //显示对应序号工件的相关图片
    void P_weld_send();         //P焊缝发送接口

signals:
    void start_match(int);      //开始匹配信号
    void match_success(int);    //匹配成功信号
    void match_failed(int);     //匹配失败信号
    void match_pending(int);    //匹配待定信号
//    void start_point_check(int);      //开始点检测
//    void point_check_success(int);    //点检测成功信号
//    void point_check_failed(int);     //点检测失败信号
    void send_weldData(std::vector<std::vector<cv::Point3d>>, std::string);

private:

        Weld_Processing       *WP;
        Mask_Detector         *Md;
        Points_Processing     *POP;
        Weld_Only_Processing  *WOP;
        character             *CCC;
        PointCloud_match      *PM;

        QList<vector<Mat>> DecImgList;
        QMutex mutex;

        vector<Mat> takeROI(vector<Mat> imgIn, vector<int> usedCamID);

        vector<string> Mask_String;
        vector<vector<string>> Dec_Strings;
        vector<vector<Point2f>> Dec_Points;
        vector<Mat> weldDeteOut;
        vector<Mat> maskDeteOut;
        vector<Mat> Numbered_hanfengs;
        vector<Mat> Numbered_Masks;
        vector<int> getGongJian;
        vector<vector<vector<Point>>> PointCloud_3d;
        vector<int> matchSucceedOrNot;

#ifndef TEST
        vector<point4d> point4dShow;     //为输出的四维点云（供UI调用）
        vector<point4d> camIndexPoint;
        vector<point4d> jsonIndexPoint;
#endif

        QString log_path;

        int deal_one_group_count;

        int kkkkk;


};

#endif // COMPUTE_H
