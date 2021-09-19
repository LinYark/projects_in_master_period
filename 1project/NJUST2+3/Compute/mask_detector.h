//===================================================================================================================================
#pragma once
#include <opencv2/opencv.hpp>
#include <caffe/caffe.hpp>
#include <string>
#include <vector>
#include <algorithm>
#include <stdio.h>
#include <sys/stat.h>
#include <dirent.h>
#include "Server/parameter.h"
//命名空间----------------------------------------------------------------
using namespace cv;
using namespace std;
using namespace caffe;
//函数声明----------------------------------------------------------------
class Mask_Detector
{

public:

    string path_source         ;
    string model_file          ;        //训练模型地址
    string trained_file        ;       //网络地址
    void Net3();

private:
    //切割参数初始化--------------------------------------------
    int height;              //图片高
    int width;               //图片宽

    int Dst_img_height      = 256;       //切片高
    int Dst_img_width       = 256;       //切片宽

    int Dst_img_height_comp = 4;         //重叠区
    int Dst_img_width_comp  = 4;         //重叠区

    int cut_width;             //行方向数量
    int cut_height;            //列方向数量

    int ceil_down_height;      //最右侧宽度
    int ceil_down_width;       //最下端高度

private:
    std::shared_ptr<Net<float> > net_;      //网络
    cv::Size input_geometry_;               //网络输入图片的大小cv::Size（height，width）
    int num_channels_;                      //网络输入图片的通道数

public:
    Mask_Detector();

    //----------------------------------------
    Mat Img_detector(Mat origiImage);
    Mat convertTo3Channels(const Mat binImg);
    Mat WaterMark();
    Mat mask_later(Mat image_Mask);
    Mat mask_laterplus(Mat image_Mask);
private:
    //----------------------------------------
    void Predict(const cv::Mat& img, Mat &maskMax);
    void WrapInputLayer(std::vector<cv::Mat>* input_channels);
    void Preprocess(const cv::Mat& img, std::vector<cv::Mat>* input_channels);
    //----------------------------------------
    vector<Mat> Image_Cut(const Mat image);
    Mat Image_Stitching(vector<Mat> Imgs_in);


    string num2str(int i);
    void getFiles(const char* path, vector<string>& files);
    Mat fillHole(const Mat srcBw);
    void RemoveSmallRegion(Mat &Src, Mat &Dst, int AreaLimit, int CheckMode, int NeihborMode);
};
