#ifndef HEBINGTHREAD_H
#define HEBINGTHREAD_H

#include <QObject>

#include <opencv2/opencv.hpp>
#include "Server/parameter.h"
#include "Compute/common.h"

using namespace cv;
using namespace std;

class character;

struct PCDcombine
{
    vector<Point3d> pcd;
    int num = 0;		//0为初始状态,-1表示未更新，可以待匹配，1表示待继续更新(num最开始表示的是编号的意思，现在觉得不太妥）
    vector<int> source;
};
struct PCDeach
{
    vector<Point3d> pcd;
};
struct camParam
{
    double A;
    double B;
    double C;
    double D;
    double E;
    double F;
};

/********************************************************
 * @ClassName:  HebingThread
 * @Brief:      合并线程,接收ClipThread线程传入的数据
 *              做合并计算之后,识别出不同的工件,将工件数据
 *              传输至匹配线程和3D显示线程
 * @date        2020/08/31
 ********************************************************/
class HebingThread : public QObject
{
    Q_OBJECT
public:
    explicit HebingThread(QObject *parent = nullptr);

    void reload();

    vector<Mat> show_pics(int num);
    vector<Point3d> PcdGet(int num);

signals:
    void one_workpiece_ready(int workpiece_index,           /*工件编号*/
                             std::vector<cv::Point3d> pcd,      /*该工件点云数据信息pointCloud*/
                             std::string name,               /*工件对应的字符*/
                             std::vector<cv::Point3d> pcdmask
                             );
    void send_to_rematch(int workpiece_index,           /*工件编号*/
                         std::vector<cv::Point3d> pcd,      /*该工件点云数据信息pointCloud*/
                         std::string name               /*工件对应的字符*/
                         );

public slots:
    void init();
    void add_one_group(int group_index,                     /*照片组编号*/
                       std::vector<cv::Mat>  GongjianTU,	/*拍摄的mask图片*/
                       std::vector<cv::Mat>  GongjianTU_n,	/*拍摄的焊缝图片，n代表交集*/
                       std::vector<std::vector<std::string>> allstring,	/*以下两物来自字符检测的*/
                       std::vector<std::vector<int>> stringmasknum,
                       std::vector<int> NUMTOTALL
                       );
    void prepare_to_rematch(std::vector<int> index_list, std::vector<std::string> char_list);


private:
    QThread *thread;
    QMutex mutex;


    int shotTimes;

    int workpiece_index;           /*工件编号*/
    vector<PCDeach> eachPCD, eachPCD_n;
    vector<PCDcombine> comPCD,comPCDmask;

    vector<int> cam_map;
    vector<double> *tranMatShift ;
    vector<Mat> X;
    vector<camParam> mycamparam;
    void savCOMMaskImg();
    void savCOMImg();
    void savImg();
    string srcToStr(vector<int> PCDnums, vector<vector<string>> allstring, vector<vector<int>> stringmasknum);

    vector<vector<string>> StringALL;
    vector<vector<int>> NUMALL; // tupian-----zifu weizhi de bianhao


    vector<Mat> MaskPicALL;// mask jiance jieguotu
    vector<vector<int>> WorkPieceNums;// hfc hebing de bianhao
    vector<int> NUMTOTALLSAVE;

    character             *CCC;
    vector<Mat> CameraImg;  //pai zhao yuan tu

    vector<vector<Point3d>> PcdAll;
//    vector<Mat> Numbered_Masks;  //MASK tu
//    vector<vector<int>> NUMALL;
//    vector<vector<int>> WorkPiceNums;
    string path_source0;
    string txt_path0;


private:
    int huidu=0;
    int huiduMAX =0;
    int moreEach=0;
    int  moreLast=0;
     vector<int> allMerge;

     vector<int> comPCDMaskSaved;
     int comPCDMasksave = 1;
     vector<int> comPCDsaved;
     int comPCDsave = 1;
    int eachPCdsave = 1;
};

#endif // HEBINGTHREAD_H
