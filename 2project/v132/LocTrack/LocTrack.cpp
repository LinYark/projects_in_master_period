#include "LocTrack.h"

LocTrack::LocTrack(int trackModel, bool hog, bool fixed_window, bool multiscale, bool lab)
    :KCFTracker(hog, fixed_window, multiscale, lab)
{
    this->trackModel = trackModel;
}

void LocTrack::convertModel(int Model)
{
    this->trackModel = Model;
}

void LocTrack::myinit(UAVParam uavparam, camParam camparam)
{
    this->myUAVParam = uavparam;
    this->myCamParam = camparam;
    double ux,uy;
    switch (this->trackModel)
    {
    case ONLY_TRACK:
        KCFTracker::init(this->myCamParam.rect, this->myCamParam.image);	//即使只定位也会有目标框和图像，这是基本的属性
        this->myroi = this->myCamParam.rect;
        this->myloc();	//loc获取GPS
        break;
    case TRACK_AND_GPS:
        KCFTracker::init(this->myCamParam.rect, this->myCamParam.image);
        this->myloc();	//loc获取GPS
        this->myroi = this->myCamParam.rect;
        break;
    case ONLY_GPSGuide:
        KCFTracker::init(this->myCamParam.rect, this->myCamParam.image);
        this->myloc();
        this->myroi = this->myCamParam.rect;
        break;
    case ONLY_IMG_LOC:

        ux = this->myCamParam.rect.x + this->myCamParam.rect.width / 2;uy = this->myCamParam.rect.y + this->myCamParam.rect.height / 2;
        qDebug()<<"init :   ux:"<<ux<<"      uy:"<<uy;
        KCFTracker::init(this->myCamParam.rect, this->myCamParam.image);
        this->myloc();
        this->myroi = this->myCamParam.rect;
        break;

    case ONLY_IMG_LOC_BY_TRACK:
        KCFTracker::init(this->myCamParam.rect, this->myCamParam.image);
        this->myloc();
        this->myroi = this->myCamParam.rect;
        break;
    case ONLY_GPSPostion:
        KCFTracker::init(this->myCamParam.rect, this->myCamParam.image);
        this->myloc();
        this->myroi = this->myCamParam.rect;
        break;

    default:
        cout << "HHHHHHHHHHHHHHHHHHHHHH-error : model out of rang-HHHHHHHHHHHHHHHHHHHHHHHHHHHHHH" << endl;
    }

}


void LocTrack::myupdate_1(UAVParam uavparam, camParam camparam)
{
    this->myUAVParam = uavparam;
    this->myCamParam = camparam;

    switch (this->trackModel)
    {
    case ONLY_TRACK: {
        Rect lastroi = this->myroi;
        this->myroi = KCFTracker::update(this->myCamParam.image, lastroi);
        break;
    }

    case TRACK_AND_GPS: {
        bool condition1 = (this->myroi.x - 2) <0;
        bool condition2 = (this->myroi.y - 2)<0;
        bool condition3 = (this->myroi.x + this->myroi.width - (this->myCamParam.xsize - 2))>0;
        bool condition4 = (this->myroi.y + this->myroi.height - (this->myCamParam.ysize - 2))>0;
        if (condition1 || condition2 || condition3 || condition4) {	//出
            this->myloc_back();	//刷新point2d
            this->point2d_to_roi();//刷新roi,推测出现位置
        }
        else {
            Rect lastroi = this->myroi;
            this->myroi = KCFTracker::update(this->myCamParam.image, lastroi);//在上一次roi附近搜寻
            this->myCamParam.rect = this->myroi;	//myloc函数使用
            this->myloc();
        }
        break;
    }
    case ONLY_GPSGuide: {
        this->myloc_back();	//刷新point2d
        this->point2d_to_roi();//刷新myroi，使用gps的point2d
        this->myroi = KCFTracker::update(this->myCamParam.image, this->myroi);
        this->myCamParam.rect = this->myroi;
        this->myloc();
        break;
    }

    case ONLY_IMG_LOC: {

        this->myroi = KCFTracker::update(this->myCamParam.image, this->myroi);
        //        this->myCamParam.rect = this->myroi;
        break;
    }
    case ONLY_IMG_LOC_BY_TRACK: {
        this->myroi = KCFTracker::update(this->myCamParam.image, this->myroi);
        this->myCamParam.rect = this->myroi;
        this->myloc();
        break;
    }
    case ONLY_GPSPostion:
        this->myloc_back();	//刷新point2d
        this->point2d_to_roi();//刷新myroi，使用gps的point2d
        //this->myloc();//会加入视场夹角，会飘逸
        break;
    default: {
        cout << "HHHHHHHHHHHHHHHHHHHHHH-error : model out of rang-HHHHHHHHHHHHHHHHHHHHHHHHHHHHHH" << endl;
    }
    }
}

void LocTrack::myupdate_2(UAVParam uavparam, camParam camparam)
{
    this->myUAVParam = uavparam;
    this->myCamParam = camparam;

    switch (this->trackModel)
    {
    case ONLY_TRACK: {
        break;
    }

    case TRACK_AND_GPS: {
        break;
    }
    case ONLY_GPSGuide: {
        break;
    }

    case ONLY_IMG_LOC: {
        double ux = this->myCamParam.rect.x + this->myCamParam.rect.width / 2, uy = this->myCamParam.rect.y + this->myCamParam.rect.height / 2;
        qDebug()<<"update :   ux:"<<ux<<"      uy:"<<uy;
        this->myloc();
        break;
    }
    case ONLY_IMG_LOC_BY_TRACK: {
        break;
    }
    case ONLY_GPSPostion:
        break;
    default: {
        cout << "HHHHHHHHHHHHHHHHHHHHHH-error : model out of rang-HHHHHHHHHHHHHHHHHHHHHHHHHHHHHH" << endl;
    }
    }
}




void LocTrack::myloc()
{
    double roll = this->myUAVParam.gr;
    double pitch = this->myUAVParam.gp;
    double yaw = this->myUAVParam.gy + this->myUAVParam.uy;
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
    double d_high = (this->myUAVParam.h) - (this->myUAVParam.fh);
    double d = this->myCamParam.d, f = this->myCamParam.f;
    double xsize = this->myCamParam.xsize, ysize = this->myCamParam.ysize;
    double ux = this->myCamParam.rect.x + this->myCamParam.rect.width / 2, uy = this->myCamParam.rect.y + this->myCamParam.rect.height / 2;

    double xrisze = this->myCamParam.xrisze, yrisze = this->myCamParam.yrisze;

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

    if (this->trackModel == ONLY_IMG_LOC) {
        this->myposition = p6;
        return;
    }

    //gps部分
    Mat g1,	//无人机旋转为地心空间直角坐标系轴向点
            g2,	//地心空间直角坐标系点
            g3;//GPS坐标
    //坐标转换关系
    double l = this->myUAVParam.l, m = this->myUAVParam.m;
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
    double h = this->myUAVParam.h;
    Mat Ds = (Mat_<double>(3, 1) <<
              (N + h)*cosd(m)*cosd(l),
              (N + h)*cosd(m)*sind(l),
              (N*(1 - e2) + h)*sind(m)
              );
    g2 = g1 + Ds;
    double X = g2.at<double>(0, 0), Y = g2.at<double>(1, 0), Z = g2.at<double>(2, 0);
    double Li, Mi, Hi, Ni;
    Li = atand(Y, X);
    //if ((X < 0) && (Y>0)) {
    //	Li = 180 + Li;
    //}
    //else if ((X < 0) && (Y<0)){
    //	Li = -180 + Li;
    //}

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

    this->myposition = g3;
}

void LocTrack::myloc_back()
{
    Mat g1,//Gps坐标
            g2,//空间直角坐标系
            g3,//无人机再空间直角坐标系坐标
            g4,//目标点在无人机的相对距离（基于空间直角坐标系）
            g5;//无人机地理坐标系下的目标点
    g1 = this->myposition;
    double L = g1.at<double>(0, 0), M = g1.at<double>(1, 0), H = g1.at<double>(2, 0);
    double a = 6378137; double b = 6356752.3142;
    double e = sqrt(a*a - b*b) / a;   double  e2 = e*e;
    double N = a / sqrt(1 - e2*sind(M)*sind(M));
    g2 = (Mat_<double>(3, 1) <<
          (N + H)*cosd(M)*cosd(L),
          (N + H)*cosd(M)*sind(L),
          (N*(1 - e2) + H)*sind(M)
          );

    //
    double l = this->myUAVParam.l, m = this->myUAVParam.m, h = this->myUAVParam.h;
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
    double roll = this->myUAVParam.gr;
    double pitch = this->myUAVParam.gp;
    double yaw = this->myUAVParam.gy + this->myUAVParam.uy;
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
    double f = this->myCamParam.f;
    p3_x = f / p2.at<double>(2, 0)*p2.at<double>(0, 0);
    p3_y = f / p2.at<double>(2, 0)*p2.at<double>(1, 0);

    p3 = (Mat_<double>(3, 1) <<
          p3_x,
          p3_y,
          f
          );
    double d = this->myCamParam.d;
    double xsize = this->myCamParam.xsize;
    double ysize = this->myCamParam.ysize;
    double xrisze = this->myCamParam.xrisze;
    double yrisze = this->myCamParam.yrisze;
    double p4_x = p3_x / d + xsize / xrisze / 2;
    double p4_y = p3_y / d + ysize / yrisze / 2;
    p4 = (Mat_<double>(2, 1) <<
          p4_x,
          p4_y
          );
    this->mypositionback.x = p4_x*xrisze;
    this->mypositionback.y = p4_y*yrisze;

}

Mat LocTrack::getPos()
{
    return this->myposition;
}

Point2d LocTrack::getPosBack()
{
    return this->mypositionback;
}

void LocTrack::point2d_to_roi()
{
    this->myroi.x = mypositionback.x - this->myroi.width / 2;
    this->myroi.y = mypositionback.y - this->myroi.height / 2;
}

Rect LocTrack::getroi()
{
    return this->myroi;
}



double LocTrack::sind(double corner)
{
    double pi = 3.1415926535898;
    return sin(corner / 180 * pi);
}

double LocTrack::cosd(double corner)
{
    double pi = 3.1415926535898;
    return cos(corner / 180 * pi);
}

double LocTrack::atand(double y, double x)
{
    double pi = 3.1415926535898;
    return  atan2(y, x) / pi * 180;
}

