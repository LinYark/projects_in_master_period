#include <QCoreApplication>
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "zbar.h"
#include <opencv2/imgproc/types_c.h>


using namespace zbar;  //添加zbar名称空间
using namespace cv;
using namespace std;

void testzbar();
Point Center_cal(vector<vector<Point> > contours, int i);
