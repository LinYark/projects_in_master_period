#ifndef GONGJIAN_HEBING_H
#define GONGJIAN_HEBING_H

#pragma once
#include<math.h>
#include <opencv2/opencv.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <iostream>
#include <fstream>
//#include <io.h>
#include <stdio.h>
#include <cstdlib>
#include <time.h>
#include<vector>
#include "Server/parameter.h"

#include"LunKuoPiPeiHY.h"
#include "common.h"
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>
#include <pcl/visualization/pcl_visualizer.h>
#include <pcl/io/pcd_io.h>
using pcl::visualization::PointCloudColorHandlerGenericField;
using pcl::visualization::PointCloudColorHandlerCustom;
typedef pcl::PointXYZ PointT;
typedef pcl::PointCloud<PointT> PointCloud;
typedef pcl::PointNormal PointNormalT;
typedef pcl::PointCloud<PointNormalT> PointCloudWithNormals;



using namespace cv;
using namespace std;

struct Points_1
{
    vector<vector<vector<double>>> Xr00, Yr00, Zr00;    // 图--焊缝--坐标
};

struct HanFeng_1
{
    vector<vector<double>> HFx, HFy, HFz;        // 焊缝--坐标
};

struct ShuChuHF
{
    HanFeng_1 PointsHF;        // 和struct HanFeng_1一样
    vector<int> GongJianHF;    // 每条焊缝对应底板工件的编号
};
struct GongJian
{
    HanFeng_1 GongJianPoints;
    vector<int> GongJianNumber;
};


//void save_pcl_to_img(PointCloud::Ptr target00);
//void save_pcl_to_img(PointCloud::Ptr target00, PointCloud::Ptr cloud_standerd_temp);
class gongjian_hebing
{

    public:

    Mat MaskStack;
    Mat Numbered_Mask_Stack;

    Mat hanfengStack;
    Mat Numbered_hanfeng_Stack;

    static void save_pcl_to_img(PointCloud::Ptr target00);
    static void save_pcl_to_img(PointCloud::Ptr target00, PointCloud::Ptr cloud_standerd_temp);

public:
    gongjian_hebing();
    void viewPair1(PointCloud::Ptr cloud1);
    void viewPair2(PointCloud::Ptr cloud1, PointCloud::Ptr cloud2);                    //三维点云显示

    //=========================================================================================
    //weld, mask, Strings, Points依次为检测得到的焊缝、mask、字符串、坐标输入
    //numberedWeld：带编号的焊缝图，其中编号为合并后的新编号， numberedMask：带编号的mask图，其中编号为合并前的编号
    //getGongJianNewnumber为原始工件合并后依次对应的新编号，MaxNumber为合并后最大的工件编号
    ////newString为不同ID的工件依次对应的字符串（供UI调用）
    /// vector<point4d>& point4d_out为输出的四维点云（供UI调用）
    //=================================================================================
    void numberAndMerge(vector<Mat>weld,vector<Mat>mask,vector<vector<string>> Strings,
                        vector<vector<Point2f>> Points, vector<Mat>& numberedWeld, vector<Mat> & numberedMask,
                       vector<string>& newString, vector<double>&  Longmen, double L_shift0, vector<int>& getGongJianNewnumber,
                       int MaxCamNum, int MaxCamCrack,  int &MaxNumber, vector<int> cammap,vector<point4d>& point4d_out);
    vector<int> Gongjianhebin(vector<Mat> GongjianTUall, double L_shift, vector<double> location, vector<int> cammap,vector<point4d>& point4d_in);

    void check(vector<Mat> GongjianTUall, double L1, vector<double> location, vector<int> zifu, vector<int> cammap);

    vector<Point3d> handEYEcalibration(vector<vector<Point>> TUpoint, vector<double> location,double L1, vector<int> cammap);

    string num2str_h(int i);
    Mat convertTo1Channels(const Mat binImg);
    Mat convertTo3Channels(const Mat binImg);
    Mat WaterMark();
    vector<Mat> ROI(vector<Mat> imgIn);



    void RemoveSmallRegion(Mat &Src, Mat &Dst, int AreaLimit, int CheckMode, int NeihborMode);
private:
    string path_source;
    string txt_path;
    char path_txt[300];


//    string path_SAVE_source0;
//    string path_SAVE_source_hfc;
//    string Tmp_Path;


    string num2str(int i);
    vector<string> String_Mask_Matching(int numbermask, vector<Mat> MaskIn,
                                              vector<vector<string>> StringIn,vector<vector<Point2f>> PointsIn, vector<int> newNumber);
};


#endif // GONGJIAN_HEBING_H
