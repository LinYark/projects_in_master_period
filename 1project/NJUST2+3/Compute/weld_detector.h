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
using namespace caffe;  // NOLINT(build/namespaces)

//焊缝合并------------
#define BLACK 0
#define WHITE 255
#define GRAY 128
#define pi0 3.14159265

//函数声明----------------------------------------------------------------
class Weld_Processing
{
public:
    Weld_Processing();

private:
    //path_model
    string path_source;
    string model_file ;        //训练模型地址
    string trained_file ;       //网络地址
   //切割参数初始化--------------------------------------------
    int height;              //图片高
    int width;               //图片宽

    int Dst_img_height      = 512;      //切片高
    int Dst_img_width       = 512;       //切片宽

    int Dst_img_height_comp = 24;
    int Dst_img_width_comp  = 24;

    int cut_width;             //行方向数量
    int cut_height;            //列方向数量

    int ceil_down_height;      //最右侧宽度
    int ceil_down_width;       //最下端高度


private:
    std::shared_ptr<Net<float> > net_;      //网络
    cv::Size input_geometry_;               //网络输入图片的大小cv::Size（height，width）
    int num_channels_;                      //网络输入图片的通道数

public:

    Mat     Img_detector(Mat origiImage);
    string  num2str(int i);
    void    getFiles(const char* path, vector<string>& files);

    //细化提取
    void RemoveSmallRegion(Mat &Src, Mat &Dst, int AreaLimit, int CheckMode, int NeihborMode);// 去除较小连通域
    void Max_Min(const Mat Mask16Lite,  ushort  &Min,  ushort  &Max);//求非0最大值与最小值
    void ImgThin(cv::Mat& src, cv::Mat& dst, int intera);//图像细化,取单像素线宽
    Mat convertTo3Channels(const Mat binImg);
    Mat WaterMark();
    void    Net4();
    Mat weld_later(Mat imageDete);
private:
    //RCF网络------------------------

    Mat     Predict(const cv::Mat& img);
    void    WrapInputLayer(std::vector<cv::Mat>* input_channels);
    void    Preprocess(const cv::Mat& img, std::vector<cv::Mat>* input_channels);
    //图片切割拼接--------------------
    vector<Mat> Image_Cut(const Mat  image);
    Mat     Image_Stitching(vector<Mat> Imgs_in);

    int		r;			//nms最大半径
    int		s;			//最大边界半径
    float	m;			//最大监督系数
    int		nThreads;	//用于计算的线程数

public:
    Mat		NmsFaction(const Mat In);

private:

    void	gradient2D(Mat &Ix, Mat &Iy, Mat input);
    Mat		Equation_1(const Mat Oyy, const Mat Oxy, const Mat Oxx);
    void	Equation_2(int h, int w, float *E0, float *O, float* E);
    float	Equation_3(float *I, int h, int w, float x, float y);

};

