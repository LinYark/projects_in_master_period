#pragma once
#include "Server/parameter.h"
#include "LunKuoPiPeiHY.h"
#include "json.h"
#include "Compute/common.h"
#include <opencv2/opencv.hpp>
#include <opencv2/ml.hpp>
//#include <opencv2/flann2/miniflann.hpp>
//#include "opencv.hpp"
#include "vtkKdTree.h"
//#include "opencv2/flann2/kdtree_index.h"

struct direct_turn_matrix
{
    Eigen::Matrix4f transformation_matrix;
    Eigen::Matrix4f transformation_matrix2;
};

class PointCloud_match
{
public:
    PointCloud_match();

private:
    string path_source;
    string Path_gongjianmodel;
    int pipei_result;

public:
    string  strToPath(string tmp);
    string reverseString(string s);


    vector<Point3d> Match(int workpiece_index, PointCloud::Ptr target, char* Path);
    PointCloud::Ptr p3dToPCD(vector<cv::Point3d> p3d);
    void save_pcl_to_img(PointCloud::Ptr target00);
    void save_pcl_to_img(PointCloud::Ptr target00, PointCloud::Ptr cloud_standerd_temp,int workpiece_index,double pipeidu000);
    void save_pcl_to_img1(PointCloud::Ptr target00, PointCloud::Ptr cloud_standerd_temp);
    direct_turn_matrix direct_turn(PointCloud::Ptr source00);
    Eigen::Matrix4f getMinRect(PointCloud::Ptr target00);
    float func_2(vector<Point3d> twoPoint, PointCloud::Ptr sourcelk, int pointNums,int  jumpPoint);
    vector<Point3d> Shi_Duan(vector<Point3d> fourPoint, PointCloud::Ptr sourcelk,
    int pointNums,//??????????
    double distance,
    int jumpPoint
    );
};
