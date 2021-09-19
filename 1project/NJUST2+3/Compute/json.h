
#pragma once
#ifndef JSON_H
#define JSON_H


#include <QCoreApplication>
#include <stdio.h>
#include "cjson.h"
#include <math.h>
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

#include"LunKuoPiPeiHY.h"
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


struct POINT
{
    double X;
    double Y;
    double Z;
};

struct SEGMENT
{
    struct POINT amplitude;
    struct POINT end_point;
};

struct CONTOUR
{
    struct POINT start_point;
    struct SEGMENT lis_segments[1000];
    int num_segment;
};

struct WHOLE
{
    char type[100];
    char name[100];
    struct CONTOUR contour;
};


struct line_fl
{
    vector<double> xx;
    vector<double> yy;
    vector<double> zz;
};

void viewPair1(PointCloud::Ptr cloud1);
void viewPair2(PointCloud::Ptr cloud1);
void json_print(int len_whole , struct WHOLE whole[]);
vector<vector<vector<double>>> all_points(char* type , int len_whole ,  WHOLE whole[]);
void print_all_points(vector<vector<vector<double>>> all_weld_endpoints);
struct line_fl line_compute(double xx1 , double yy1 , double zz1 , double xx2 , double yy2 , double zz2);
vector<vector<line_fl>> all_lines(vector<vector<vector<double>>> all_weld_endpoints);
void all_lines_print(vector<vector<line_fl>> all_weld_lines);
vector<Point3d> ago2cloud(vector<vector<line_fl>> lines1,vector<vector<line_fl>> lines2,vector<vector<line_fl>> lines3);
vector<vector<Point3d>> weld2cloud(vector<vector<line_fl>> lines1);
vector<vector<Point3d>> json_all(char* Path);
//vector<Point3d> json_all(char* Path);
vector<vector<Point3d>> json_hf(char* Path);

#endif // JSON_H
