
#pragma once
#include <pcl/registration/ia_ransac.h>
#include <pcl/point_types.h>
#include <pcl/point_cloud.h>
#include <pcl/features/normal_3d.h>
#include <pcl/features/fpfh.h>
#include <pcl/search/kdtree.h>
#include <pcl/io/pcd_io.h>
#include <pcl/filters/voxel_grid.h>
#include <pcl/filters/filter.h>
#include <pcl/registration/icp.h>
#include <pcl/visualization/pcl_visualizer.h>
#include <time.h>
#include<vector>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;
using namespace std;
using pcl::NormalEstimation;
using pcl::search::KdTree;
typedef pcl::PointXYZ PointT;
typedef pcl::PointCloud<PointT> PointCloud;


//#include <pcl/registration/ia_ransac.h>
//#include <pcl/point_types.h>
//#include <pcl/point_cloud.h>
//#include <pcl/features/normal_3d.h>
//#include <pcl/features/fpfh.h>
//#include <pcl/search/kdtree.h>
//#include <pcl/io/pcd_io.h>
//#include <pcl/filters/voxel_grid.h>
//#include <pcl/filters/filter.h>
//#include <pcl/registration/icp.h>
//#include <pcl/visualization/pcl_visualizer.h>
//#include <time.h>

//using pcl::NormalEstimation;
//using pcl::search::KdTree;
//typedef pcl::PointXYZ PointT;
//typedef pcl::PointCloud<PointT> PointCloud;

struct PiPei
{
    //PointCloud::Ptr result00;
    PointCloud::Ptr result00;
    Eigen::Matrix4f init_transform_ni00;
    Eigen::Matrix4f transformation_matrix0_ni00;
    double PiPeiDu00;
    int flag;
};

//ÀàÉùÃ÷----------------------------------------------------------------
class LunKuoPiPei
{
//public:
//     float VOXEL_GRID_SIZE = 2;    //3
//     double radius_normal = 65;    //20
//     double radius_feature = 115;    //50
//     int max_sacis_iteration = 3000;    //1000
//     float min_correspondence_dist = 0.01;    //0.01
//     double max_correspondence_dist = 3000;    //1000  //µãÔÆÆ¥ÅäËã·šµÄÒ»Ð©²ÎÊý

public:
    //LunKuoPiPei();
    PointCloud::Ptr LunKuoPiPeiHY_11(PointCloud::Ptr cloud1);
    //PointCloud::Ptr LunKuoPiPeiHY_12(cv::Mat& img2);
    PiPei LunKuoPiPeiHY_2(PointCloud::Ptr cloud1, PointCloud::Ptr cloud2);
};

void viewPair(PointCloud::Ptr pcd_src,PointCloud::Ptr pcd_tgt);




