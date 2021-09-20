#ifndef COMMON_H
#define COMMON_H
//qt
#include <QThread>
#include<QDebug>

//c++ std
#include<stdio.h>  /*标准输入输出定义*/
#include<stdlib.h>     /*标准函数库定义*/
#include<unistd.h>     /*Unix 标准函数定义*/
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>      /*文件控制定义*/
#include<termios.h>    /*PPSIX 终端控制定义*/
#include<errno.h>      /*错误号定义*/
#include<string.h>
#include <time.h>
#include <stdio.h>
#include<iostream>
#include <ctime>
#include <fstream>
#include <stdlib.h>
#include <signal.h>
#include <mutex>
#include <iostream>
#include <stdio.h>

#include <string.h>
#include <signal.h>
#include <stdlib.h>
#include <typeinfo>
#include <sys/stat.h>
#include <dirent.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdarg.h>
#include <math.h>
#include <time.h>
#include <cstdlib>
#include <stdint.h>
#include <vector>
#include <math.h>
#include <fstream>

#include<iomanip>
#include<thread>


using namespace std;

#include <opencv.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/core/core.hpp>


using namespace cv;

struct ImageData
{
    Mat* vis;
    bool visNew;
    string visPath;
    bool codeFlg;
};

struct Bbox
{
    int bx0;
    int by0;
    int bx1;
    int by1;
    int bcl;
    int bw;
    int bh;
    float prob;
};

struct ioubox
{
    int m_left;
    int m_top;
    int m_width;
    int m_height;
    ioubox(int left, int top, int width, int height)
    {
        m_left = left;
        m_top = top;
        m_width = width;
        m_height = height;
    }
};

struct UAVParam
{
    double vx = 0;
    double vy = 0;
    double vz = 0;
    double up = 0;
    double ur = 0;
    double uy = 0;
    double gp = 0;
    double gr = 0;
    double gy = 0;
    double h = 0;
    double fh = 0;
    double l = 0;
    double m = 0;
    double hLidar = 0;
};

struct camParam
{
    Rect rect; //Rect是一个类，我觉得和Mat比较相似，rect对象用来存储一个矩形框的左上角坐标、宽度和高度，即描述矩形的宽度、高度、和原点。
    cv::Mat image;

    int xsize = 1920;	double xrisze;
    int ysize = 1200;  double yrisze;
    double f;
    double d;
};

class common
{
public:
    common();
    static bool b1;
    static mutex m1;

};


#endif // COMMON_H
