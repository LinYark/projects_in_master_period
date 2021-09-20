#include "LocTrack.h"

trackClass::trackClass( bool hog, bool fixed_window, bool multiscale, bool lab)
    :KCFTracker(hog, fixed_window, multiscale, lab)
{
}

double location::sind(double corner)
{
    double pi = 3.1415926535898;
    return sin(corner / 180 * pi);
}

double location::cosd(double corner)
{
    double pi = 3.1415926535898;
    return cos(corner / 180 * pi);
}

double location::atand(double y, double x)
{
    double pi = 3.1415926535898;
    return  atan2(y, x) / pi * 180;
}

Mat location::compute(UAVParam uavparam_in, camParam camparam_in)
{
    UAVParam uavparam = uavparam_in;
    camParam camparam = camparam_in;

    double roll = uavparam.gr;
    double pitch = uavparam.gp;
    double yaw = uavparam.gy + uavparam.uy;
    //1.计算无人机图像系到地理系的旋转矩阵
    Mat Xc, Yc, Zc, Tc2u;		//tran cam to uav	(旋转角对应uav to cam，所以这里时逆)
    Xc = (cv::Mat_<double>(3, 3) <<
          1, 0, 0,
          0, cosd(-roll), sind(-roll),
          0, -sind(-roll), cosd(-roll)
          );
    Yc = (cv::Mat_<double>(3, 3) <<
          cosd(-pitch), 0, -sind(-pitch),
          0, 1, 0,
          sind(-pitch), 0, cosd(-pitch)
          );
    Zc = (cv::Mat_<double>(3, 3) <<
          cosd(-yaw), sind(-yaw), 0,
          -sind(-yaw), cosd(-yaw), 0,
          0, 0, 1
          );
    Tc2u = Xc*Zc*Yc;	//外旋
    //2.获取图像系点
    double d_high = (uavparam.h) - (uavparam.fh);
    double d = camparam.d, f = camparam.f;
    double xsize = camparam.xsize, ysize = camparam.ysize;
    double ux = camparam.rect.x + camparam.rect.width / 2, uy = camparam.rect.y + camparam.rect.height / 2;

    double xrisze = camparam.xrisze, yrisze = camparam.yrisze;

    Mat p1,		//无人机像素坐标系成像点
            p2,		//无人机像素坐标系（轴向按照地理坐标系排列)成像点
            p3,		//无人机地理坐标系成像点
            p4,		//无人机像素坐标系实际点
            p5,		//无人机像素坐标系（轴向按照地理坐标系排列)实际点
            p6;		//无人机地理坐标系实际点

    p1 = (Mat_<double>(3, 1) <<
          (ux - xsize / 2) / xrisze*d,
          (uy - ysize / 2) / yrisze*d,
          f
          );

    p2 = (Mat_<double>(3, 1) <<
          p1.at<double>(2, 0),
          p1.at<double>(0, 0),
          p1.at<double>(1, 0)
          );
    p3 = Tc2u*p2;
    //获取夹角
    Mat n = (Mat_<double>(3, 1) << 0, 0, 1);
    double cose = f / sqrt(p1.at<double>(0, 0)*p1.at<double>(0, 0) + p1.at<double>(1, 0)*p1.at<double>(1, 0) + p1.at<double>(2, 0)*p1.at<double>(2, 0));
    double cosP = (p3.at<double>(2, 0)) /
            (sqrt(p3.at<double>(0, 0)*p3.at<double>(0, 0) + p3.at<double>(1, 0)*p3.at<double>(1, 0) + p3.at<double>(2, 0)*p3.at<double>(2, 0)));

    double d_loc = d_high / cosP;	//拍摄点的z轴距离
    double d_center = d_loc*cose; //拍摄图中心的z轴距离

    double d_x = d_center / f *p1.at<double>(0, 0);
    double d_y = d_center / f *p1.at<double>(1, 0);

    p4 = (Mat_<double>(3, 1) <<
          d_x,
          d_y,
          d_center
          );
    p5 = (Mat_<double>(3, 1) <<
          p4.at<double>(2, 0),
          p4.at<double>(0, 0),
          p4.at<double>(1, 0)
          );
    p6 = Tc2u*p5;
    return p6;

}
Mat location::computeGps(UAVParam uavparam_in, camParam camparam_in)
{
    UAVParam uavparam = uavparam_in;
    camParam camparam = camparam_in;

    double roll = uavparam.gr;
    double pitch = uavparam.gp;
    double yaw = uavparam.gy + uavparam.uy;
    //1.计算无人机图像系到地理系的旋转矩阵
    Mat Xc, Yc, Zc, Tc2u;		//tran cam to uav	(旋转角对应uav to cam，所以这里时逆)
    Xc = (cv::Mat_<double>(3, 3) <<
          1, 0, 0,
          0, cosd(-roll), sind(-roll),
          0, -sind(-roll), cosd(-roll)
          );
    Yc = (cv::Mat_<double>(3, 3) <<
          cosd(-pitch), 0, -sind(-pitch),
          0, 1, 0,
          sind(-pitch), 0, cosd(-pitch)
          );
    Zc = (cv::Mat_<double>(3, 3) <<
          cosd(-yaw), sind(-yaw), 0,
          -sind(-yaw), cosd(-yaw), 0,
          0, 0, 1
          );
    Tc2u = Xc*Zc*Yc;	//外旋
    //2.获取图像系点
    double d_high = (uavparam.h) - (uavparam.fh);
    double d = camparam.d, f = camparam.f;
    double xsize = camparam.xsize, ysize = camparam.ysize;
    double ux = camparam.rect.x + camparam.rect.width / 2, uy = camparam.rect.y + camparam.rect.height / 2;

    double xrisze = camparam.xrisze, yrisze = camparam.yrisze;

    Mat p1,		//无人机像素坐标系成像点
            p2,		//无人机像素坐标系（轴向按照地理坐标系排列)成像点
            p3,		//无人机地理坐标系成像点
            p4,		//无人机像素坐标系实际点
            p5,		//无人机像素坐标系（轴向按照地理坐标系排列)实际点
            p6;		//无人机地理坐标系实际点

    p1 = (Mat_<double>(3, 1) <<
          (ux - xsize / 2) / xrisze*d,
          (uy - ysize / 2) / yrisze*d,
          f
          );

    p2 = (Mat_<double>(3, 1) <<
          p1.at<double>(2, 0),
          p1.at<double>(0, 0),
          p1.at<double>(1, 0)
          );
    p3 = Tc2u*p2;
    //获取夹角
    Mat n = (Mat_<double>(3, 1) << 0, 0, 1);
    double cose = f / sqrt(p1.at<double>(0, 0)*p1.at<double>(0, 0) + p1.at<double>(1, 0)*p1.at<double>(1, 0) + p1.at<double>(2, 0)*p1.at<double>(2, 0));
    double cosP = (p3.at<double>(2, 0)) /
            (sqrt(p3.at<double>(0, 0)*p3.at<double>(0, 0) + p3.at<double>(1, 0)*p3.at<double>(1, 0) + p3.at<double>(2, 0)*p3.at<double>(2, 0)));

    double d_loc = d_high / cosP;	//拍摄点的z轴距离
    double d_center = d_loc*cose; //拍摄图中心的z轴距离

    double d_x = d_center / f *p1.at<double>(0, 0);
    double d_y = d_center / f *p1.at<double>(1, 0);

    p4 = (Mat_<double>(3, 1) <<
          d_x,
          d_y,
          d_center
          );
    p5 = (Mat_<double>(3, 1) <<
          p4.at<double>(2, 0),
          p4.at<double>(0, 0),
          p4.at<double>(1, 0)
          );
    p6 = Tc2u*p5;

    //gps部分
    Mat g1,	//无人机旋转为地心空间直角坐标系轴向点
            g2,	//地心空间直角坐标系点
            g3;//GPS坐标
    //坐标转换关系
    double l = uavparam.l, m = uavparam.m;
    Mat Ld, Le, UAV2Earth;
    Ld = (Mat_<double>(3, 3) <<
          cosd(-l), sind(-l), 0,
          -sind(-l), cosd(-l), 0,
          0, 0, 1
          );
    Le = (Mat_<double>(3, 3) <<
          cosd(90 + m), 0, -sind(90 + m),
          0, 1, 0,
          sind(90 + m), 0, cosd(90 + m)
          );
    UAV2Earth = Ld *Le;
    g1 = UAV2Earth*p6;

    //加上偏置
    double a = 6378137; double b = 6356752.3142;
    double e = sqrt(a*a - b*b) / a;   double  e2 = e*e;
    double N = a / sqrt(1 - e2*sind(m)*sind(m));
    double h = uavparam.h;
    Mat Ds = (Mat_<double>(3, 1) <<
              (N + h)*cosd(m)*cosd(l),
              (N + h)*cosd(m)*sind(l),
              (N*(1 - e2) + h)*sind(m)
              );
    g2 = g1 + Ds;
    double X = g2.at<double>(0, 0), Y = g2.at<double>(1, 0), Z = g2.at<double>(2, 0);
    double Li, Mi, Hi, Ni;
    Li = atand(Y, X);

    //迭代法求
    Ni = a;
    Hi = sqrt(X*X + Y*Y + Z*Z) - sqrt(a*b);
    double Mi_temp = (Z / (sqrt(X*X + Y*Y))) / (1 - e2*Ni / (Ni + Hi));
    Mi = atand(Mi_temp, 1);

    for (int i = 0; i < 100; i++)
    {
        double lastMi = Mi, lastNi = Ni;
        Ni = a / sqrt(1 - e2*sind(lastMi)*sind(lastMi));
        Hi = sqrt(X*X + Y*Y) / cosd(lastMi) - lastNi;
        double Mi_temp = (Z / (sqrt(X*X + Y*Y))) / (1 - e2*Ni / (Ni + Hi));
        Mi = atand(Mi_temp, 1);
        if (abs(lastMi - Mi) < 0.0000000001) {
            break;
        }
    }
    g3 = (Mat_<double>(3, 1) <<
          Li,
          Mi,
          Hi
          );

    return g3;
}
Mat location::computeBack(Mat g1_in,UAVParam uavparam_in)  //target gps
{
    UAVParam uavparam = uavparam_in;
    camParam camparamDefault ;

    Mat     g1, //targetGPS
            g2,//空间直角坐标系
            g3,//无人机再空间直角坐标系坐标
            g4,//目标点在无人机的相对距离（基于空间直角坐标系）
            g5;//无人机地理坐标系下的目标点

    g1=g1_in;
    double L = g1.at<double>(0, 0), M = g1.at<double>(1, 0), H = g1.at<double>(2, 0);
    double a = 6378137; double b = 6356752.3142;
    double e = sqrt(a*a - b*b) / a;   double  e2 = e*e;
    double N = a / sqrt(1 - e2*sind(M)*sind(M));
    g2 = (Mat_<double>(3, 1) <<
          (N + H)*cosd(M)*cosd(L),
          (N + H)*cosd(M)*sind(L),
          (N*(1 - e2) + H)*sind(M)
          );


    double l = uavparam.l, m = uavparam.m, h = uavparam.h;
    double n = a / sqrt(1 - e2*sind(m)*sind(m));
    g3 = (Mat_<double>(3, 1) <<
          (n + h)*cosd(m)*cosd(l),
          (n + h)*cosd(m)*sind(l),
          (n*(1 - e2) + h)*sind(m)
          );
    g4 = g3 - g2;


    Mat Ld, Le, UAV2Earth, UAV2Earth_ni;
    Ld = (Mat_<double>(3, 3) <<
          cosd(-l), sind(-l), 0,
          -sind(-l), cosd(-l), 0,
          0, 0, 1
          );
    Le = (Mat_<double>(3, 3) <<
          cosd(90 + m), 0, -sind(90 + m),
          0, 1, 0,
          sind(90 + m), 0, cosd(90 + m)
          );
    UAV2Earth = Ld *Le;
    UAV2Earth_ni = UAV2Earth.inv();
    g5 = UAV2Earth_ni*g4;

    //
    Mat p1,//相机坐标系下的目标点，按无人机地理坐标系轴向排序
            p2,//相机坐标系下的目标点,，按自己的xyf排序
            p3,//相机坐标系下的成像点
            p4;//像素坐标系的点
    double roll = uavparam.gr;
    double pitch = uavparam.gp;
    double yaw = uavparam.gy + uavparam.uy;
    Mat Xc, Yc, Zc, Tc2u, Tc2u_ni;		//tran cam to uav	(旋转角对应uav to cam，所以这里时逆)
    Xc = (cv::Mat_<double>(3, 3) <<
          1, 0, 0,
          0, cosd(-roll), sind(-roll),
          0, -sind(-roll), cosd(-roll)
          );
    Yc = (cv::Mat_<double>(3, 3) <<
          cosd(-pitch), 0, -sind(-pitch),
          0, 1, 0,
          sind(-pitch), 0, cosd(-pitch)
          );
    Zc = (cv::Mat_<double>(3, 3) <<
          cosd(-yaw), sind(-yaw), 0,
          -sind(-yaw), cosd(-yaw), 0,
          0, 0, 1
          );
    Tc2u = Xc* Zc*Yc;	//外旋
    Tc2u_ni = Tc2u.inv();
    p1 = Tc2u_ni*g5;
    p2 = (Mat_<double>(3, 1) <<
          p1.at<double>(1, 0),
          p1.at<double>(2, 0),
          p1.at<double>(0, 0)
          );

    double p3_x, p3_y;
    double f = camparamDefault.f;
    p3_x = f / p2.at<double>(2, 0)*p2.at<double>(0, 0);
    p3_y = f / p2.at<double>(2, 0)*p2.at<double>(1, 0);

    p3 = (Mat_<double>(3, 1) <<
          p3_x,
          p3_y,
          f
          );
    double d = camparamDefault.d;
    double xsize = camparamDefault.xsize;
    double ysize = camparamDefault.ysize;
    double xrisze = camparamDefault.xrisze;
    double yrisze = camparamDefault.yrisze;
    double p4_x = p3_x / d + xsize / xrisze / 2;
    double p4_y = p3_y / d + ysize / yrisze / 2;
    p4 = (Mat_<double>(2, 1) <<
          p4_x,
          p4_y
          );

    Mat r1 = (Mat_<double>(2, 1) <<
              p4_x*xrisze,
              p4_y*yrisze
              );
    return r1;

}



