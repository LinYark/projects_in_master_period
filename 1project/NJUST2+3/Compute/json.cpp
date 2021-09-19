#include "json.h"

#define interval 1

void viewPair1(PointCloud::Ptr cloud1)
{
    boost::shared_ptr<pcl::visualization::PCLVisualizer> viewer(new pcl::visualization::PCLVisualizer("Viewer1"));
    viewer->initCameraParameters();
    viewer->setBackgroundColor(0, 0, 0);
    PointCloudColorHandlerCustom<PointT> green(cloud1, 0, 255, 0);
    viewer->addPointCloud(cloud1, green, "idntcarefu1");
    viewer->spin();
}

void viewPair2(PointCloud::Ptr cloud1)
{
    boost::shared_ptr<pcl::visualization::PCLVisualizer> viewer(new pcl::visualization::PCLVisualizer("Viewer2"));
    viewer->initCameraParameters();
    viewer->setBackgroundColor(0, 0, 0);
    PointCloudColorHandlerCustom<PointT> green(cloud1, 0, 255, 0);
    viewer->addPointCloud(cloud1, green, "idntcarefu2");
    viewer->spin();
}


#include <stdio.h>
#include "cjson.h"
#include <iostream>
#include <string.h>
#include <stdlib.h>
#include <vector>
#include <math.h>
#include <opencv2/opencv.hpp>


using namespace std;
using namespace cv;

#define interval 1
#define PI 3.1415926


/* Functions */
void json_print(int len_whole , struct WHOLE whole[])
{
//    for(int i = 0;i < len_whole;i ++)
//    {
//        cout << " whole " << i << endl;
//        cout << " type " << whole[i].type << endl;
//        cout << " name " << whole[i].name << endl;
//        cout << " contour start_point ( " << whole[i].contour.start_point.X << ' ' << whole[i].contour.start_point.Y << ' ' << whole[i].contour.start_point.Z << " )" << endl;
//        cout << " num_seg " << whole[i].contour.num_segment << endl;

//        for(int j = 0;j < whole[i].contour.num_segment ;j ++)
//        {
//            cout << " segment " << j << endl;
//            cout << " contour segments amplitude ( " << whole[i].contour.lis_segments[j].amplitude.X << ' ' << whole[i].contour.lis_segments[j].amplitude.Y << ' ' << whole[i].contour.lis_segments[j].amplitude.Z << " )" << endl;
//            cout << " contour segments end_point ( " << whole[i].contour.lis_segments[j].end_point.X << ' ' << whole[i].contour.lis_segments[j].end_point.Y << ' ' << whole[i].contour.lis_segments[j].end_point.Z << " )" << endl;
//        }
//        cout << endl << endl;
//    }
}

vector<vector<vector<double>>> all_points(char* type, int len_whole,  WHOLE whole[])
{
    vector<vector<vector<double>>> all_weld_endpoints;
    for (int whole_idx = 0; whole_idx < len_whole; whole_idx++)
    {

        if (strstr(whole[whole_idx].type, type))
        {
            vector<vector<double>> weld_endpoints;
            vector<double> startpoint;
            startpoint.push_back(whole[whole_idx].contour.start_point.X);
            startpoint.push_back(whole[whole_idx].contour.start_point.Y);
            startpoint.push_back(whole[whole_idx].contour.start_point.Z);
            startpoint.push_back(1.0);
            weld_endpoints.push_back(startpoint);

           // cout<<whole[whole_idx].contour.num_segment<<endl;
            for (int seg_idx = 0; seg_idx < whole[whole_idx].contour.num_segment; seg_idx++)
            {
                double amplitude_flg = 0.0;
                vector<double> endpoint;
                if (whole[whole_idx].contour.lis_segments[seg_idx].amplitude.X == 0 &&
                    whole[whole_idx].contour.lis_segments[seg_idx].amplitude.Y == 0 &&
                    whole[whole_idx].contour.lis_segments[seg_idx].amplitude.Z == 0)
                {
                    amplitude_flg = 1.0;
                }


                endpoint.push_back(whole[whole_idx].contour.lis_segments[seg_idx].end_point.X);
                endpoint.push_back(whole[whole_idx].contour.lis_segments[seg_idx].end_point.Y);
                endpoint.push_back(whole[whole_idx].contour.lis_segments[seg_idx].end_point.Z);
                endpoint.push_back(amplitude_flg);


                endpoint.push_back(whole[whole_idx].contour.lis_segments[seg_idx].amplitude.X);
                endpoint.push_back(whole[whole_idx].contour.lis_segments[seg_idx].amplitude.Y);
                endpoint.push_back(whole[whole_idx].contour.lis_segments[seg_idx].amplitude.Z);

                weld_endpoints.push_back(endpoint);
            }
            all_weld_endpoints.push_back(weld_endpoints);
        }
    }
    return all_weld_endpoints;
}

void print_all_points(vector<vector<vector<double>>> all_weld_endpoints)
{
//    cout << endl;
//    cout << "vector endpoints output" << endl;
//    for(int whole_idx = 0;whole_idx < all_weld_endpoints.size();whole_idx ++)
//    {
//        for(int i = 0 ; i < all_weld_endpoints[whole_idx].size();i ++ )
//        {
//            for(int j = 0;j < all_weld_endpoints[whole_idx][i].size();j ++)
//            {
//                cout << " " << all_weld_endpoints[whole_idx][i][j] << ' ';
//            }
//            cout << endl;
//        }
//        cout << endl << endl << endl;
//    }
}

struct line_fl line_compute(double xx1 , double yy1 , double zz1 , double xx2 , double yy2 , double zz2)
{
    double dis = sqrt((xx1 - xx2)*(xx1 - xx2) + (yy1 - yy2)*(yy1 - yy2) + (zz1 - zz2)*(zz1 - zz2));
//    cout <<"两点间的间距为："<< dis << endl;
    int pt_num =(dis/interval)+1;
//    cout << "线段点个数（含首尾两点）：" << pt_num << endl;
    vector <double>xx(pt_num), yy(pt_num), zz(pt_num);
    if ((xx1 == xx2) && (yy1 == yy2))
    {
        if (zz1 > zz2)
        {
            double temp;
            temp = zz1;
            zz1 = zz2;
            zz2 = temp;
        }
//        cout << "与Z坐标轴平行" << endl;
        for (int i = 0; i < pt_num; i++)
        {
            xx[i] = xx1;
            yy[i] = yy1;
            zz[i] = zz1+i* interval;
        }
//        for (int i = 0; i < pt_num; i++)
//        {
//            cout << xx[i] << "    " << yy[i] << "     " << zz[i] << endl;
//        }
        }
    if  ((xx1 == xx2) && (zz1 == zz2))
    {
        if (yy1 > yy2)
        {
            double temp;
            temp = yy1;
            yy1 = yy2;
            yy2 = temp;
        }
//        cout << "与Y坐标轴平行" << endl;
        for (int i = 0; i < pt_num; i++)
        {
            xx[i] = xx1;
            yy[i] = yy1 + i * interval;
            zz[i] = zz1;
        }
//        for (int i = 0; i < pt_num; i++)
//        {
//            cout << xx[i] << "    " << yy[i] << "     " << zz[i] << endl;
//        }
    }
    if  ((yy1 == yy2) && (zz1 == zz2))
    {
        if (xx1 > xx2)
        {
            double temp;
            temp = xx1;
            xx1 = xx2;
            xx2 = temp;
        }
//         cout << "与X坐标轴平行" << endl;
        for (int i = 0; i < pt_num; i++)
        {
            xx[i] = xx1 + i * interval;
            yy[i] = yy1;
            zz[i] = zz1;
        }
//        for (int i = 0; i < pt_num; i++)
//        {
//            cout << xx[i] << "    " << yy[i] << "     " << zz[i] << endl;
//        }
    }
    if ((xx1 == xx2) && (yy1 != yy2)&&(zz1!=zz2))
    {
        if (yy1 > yy2)
        {
            double temp;
            temp = yy1;
            yy1 = yy2;
            yy2 = temp;

            // dsq
            temp = zz1;
            zz1 = zz2;
            zz2 = temp;

        }
//        cout << "与YOZ平面平行" << endl;
        double interval_y = fabs(yy1 - yy2) / dis;
        for (int i = 0; i < pt_num; i++)
        {
            xx[i] = xx1;
            yy[i] = yy1+i* interval_y;
            zz[i] = ((yy[i]-yy1)*zz2-(yy[i]-yy2)*zz1)/(yy2-yy1);
        }
//        for (int i = 0; i < pt_num; i++)
//        {
//            cout << xx[i] << "    " << yy[i] << "     " << zz[i] << endl;
//        }
    }
    if ((xx1 != xx2) && (yy1 == yy2) && (zz1 != zz2))
    {
        if (zz1 > zz2)
        {
            double temp;
            temp = zz1;
            zz1 = zz2;
            zz2 = temp;

            // dsq
            temp = xx1;
            xx1 = xx2;
            xx2 = temp;

        }
//        cout << "与XOZ平面平行" << endl;
        double interval_z = fabs(zz1 - zz2) / dis;
        for (int i = 0; i < pt_num; i++)
        {
            yy[i] = yy1;
            zz[i] = zz1+i* interval_z;
            xx[i] = ((zz[i] - zz1)*xx2 - (zz[i] - zz2)*xx1) / (zz2 - zz1);
        }

    }
    if ((xx1 != xx2) && (yy1 != yy2) && (zz1 == zz2))
    {
        if (xx1 > xx2)
        {
            double temp;
            temp = xx1;
            xx1 = xx2;
            xx2 = temp;

            temp = yy1;
            yy1 = yy2;
            yy2 = temp;

        }
//        cout << "与XOY平面平行" << endl;
        double interval_x = fabs(xx1 - xx2) / dis;
        for (int i = 0; i < pt_num; i++)
        {
            xx[i] = xx1 + i * interval_x;
            yy[i] = ((xx[i] - xx1)*yy2 - (xx[i] - xx2)*yy1) / (xx2 - xx1);
            zz[i] = zz1;
        }
//        for (int i = 0; i < pt_num; i++)
//        {
//            cout << xx[i] << "    " << yy[i] << "     " << zz[i] << endl;
//        }
    }
    if ((xx1 != xx2) && (yy1 != yy2) && (zz1 != zz2))
    {
        if (zz1 > zz2)
        {
            double temp;
            temp = zz1;
            zz1 = zz2;
            zz2 = temp;

            // dsq
            temp = xx1;
            xx1 = xx2;
            xx2 = temp;
            temp = yy1;
            yy1 = yy2;
            yy2 = temp;

        }
        double interval_0 = (zz2 - zz1) / dis;
        for (int i = 0; i < pt_num; i++)
        {
            zz[i] = zz1+ interval_0*i;
            xx[i] = ((zz[i] - zz1)*xx2 - (zz[i] - zz2)*xx1) / (zz2 - zz1);
            yy[i] = ((zz[i] - zz1)*yy2 - (zz[i] - zz2)*yy1) / (zz2 - zz1);
        }
//        for (int i = 0; i < pt_num; i++)
//        {
//            cout << xx[i] << "    " << yy[i] << "     " << zz[i] << endl;
//        }
    }

    struct line_fl result;
    result.xx = xx;
    result.yy = yy;
    result.zz = zz;

    return result;
}

vector<Vec3d> Findpoints(double Xa, double Ya, double Za, double Xb, double Yb, double Zb, double Xc, double Yc, double Zc)
{
    Vec3d temp;
    vector<Vec3d>temppoints;
    double X1 = (Xa + Xb) / 2 + Xc;        //x1是xb,xb是xc                    //µÚÈýžöµã
    double Y1 = (Ya + Yb) / 2 + Yc;
    double Z1 = (Za + Zb) / 2 + Zc;
    double a1, b1, c1, d1;
    double a2, b2, c2, d2;
    double a3, b3, c3, d3;

    a1 = (Ya*Z1 - Y1*Za - Ya*Zb + Yb*Za + Y1*Zb - Yb*Z1);
    b1 = -(Xa*Z1 - X1*Za - Xa*Zb + Xb*Za + X1*Zb - Xb*Z1);
    c1 = (Xa*Y1 - X1*Ya - Xa*Yb + Xb*Ya + X1*Yb - Xb*Y1);
    d1 = -(Xa*Y1*Zb - Xa*Yb*Z1 - X1*Ya*Zb + X1*Yb*Za + Xb*Ya*Z1 - Xb*Y1*Za);

    a2 = 2 * (X1 - Xa);
    b2 = 2 * (Y1 - Ya);
    c2 = 2 * (Z1 - Za);
    d2 = Xa * Xa + Ya * Ya + Za * Za - X1 * X1 - Y1 * Y1 - Z1 * Z1;
    a3 = 2 * (Xb - Xa);
    b3 = 2 * (Yb - Ya);
    c3 = 2 * (Zb - Za);
    d3 = Xa * Xa + Ya * Ya + Za * Za - Xb * Xb - Yb * Yb - Zb * Zb;

    double Xcir, Ycir, Zcir;
    Xcir = -(b1*c2*d3 - b1*c3*d2 - b2*c1*d3 + b2*c3*d1 + b3*c1*d2 - b3*c2*d1)
        / (a1*b2*c3 - a1*b3*c2 - a2*b1*c3 + a2*b3*c1 + a3*b1*c2 - a3*b2*c1);
    Ycir = (a1*c2*d3 - a1*c3*d2 - a2*c1*d3 + a2*c3*d1 + a3*c1*d2 - a3*c2*d1)
        / (a1*b2*c3 - a1*b3*c2 - a2*b1*c3 + a2*b3*c1 + a3*b1*c2 - a3*b2*c1);
    Zcir = -(a1*b2*d3 - a1*b3*d2 - a2*b1*d3 + a2*b3*d1 + a3*b1*d2 - a3*b2*d1)
        / (a1*b2*c3 - a1*b3*c2 - a2*b1*c3 + a2*b3*c1 + a3*b1*c2 - a3*b2*c1);

    double R;
    R = sqrt((Xa - Xcir)*(Xa - Xcir) + (Ya - Ycir)*(Ya - Ycir) + (Za - Zcir)*(Za - Zcir));
    double A1 = Xa - Xcir, A2 = Ya - Ycir, A3 = Za - Zcir;
    double B1 = X1 - Xcir, B2 = Y1 - Ycir, B3 = Z1 - Zcir, D1 = Xb - Xcir, D2 = Yb - Ycir, D3 = Zb - Zcir;
    double C1 = B1 - (A1*A1*B1 + A1*A2*B2 + A1*A3*B3) / (A1*A1 + A2*A2 + A3*A3), C2 = B2 - (A1*A2*B1 + A2*A2*B2 + A2*A3*B3) / (A1*A1 + A2*A2 + A3*A3), C3 = B3 - (A1*A3*B1 + A2*A3*B2 + A3*A3*B3) / (A1*A1 + A2*A2 + A3*A3);
    double n1 = A1 / sqrt(A1*A1 + A2*A2 + A3*A3), n2 = A2 / sqrt(A1*A1 + A2*A2 + A3*A3), n3 = A3 / sqrt(A1*A1 + A2*A2 + A3*A3), N1 = C1 / sqrt(C1*C1 + C2*C2 + C3*C3), N2 = C2 / sqrt(C1*C1 + C2*C2 + C3*C3), N3 = C3 / sqrt(C1*C1 + C2*C2 + C3*C3);
    double theta = acos((A1*D1+A2*D2+A3*D3)/sqrt((A1*A1 + A2*A2 + A3*A3)*(D1*D1 + D2*D2 + D3*D3)));
    double L, theta0,Vx,Vy,Vz;
    int num;
    //theta0 = theta / num;
    if(sqrt(Xc * Xc + Yc * Yc + Zc * Zc)-R<0)
        L = R * theta, num = (int)abs(L) + 1,theta0 = theta / num;
    else L = R * (2 * PI - theta), num = (int)abs(L) + 1,theta0 = (2 * PI - theta) / num;

    for (int i = 1; i < num; i++)
    {
        Vx = Xcir + R*n1*cos(i*theta0) + R*N1*sin(i*theta0);
        Vy = Ycir + R*n2*cos(i*theta0) + R*N2*sin(i*theta0);
        Vz = Zcir + R*n3*cos(i*theta0) + R*N3*sin(i*theta0);
        temp[0] = Vx;
        temp[1] = Vy;
        temp[2] = Vz;
        temppoints.push_back(temp);
    }
    return temppoints;

}

struct line_fl arc_compute(double Ax, double Ay, double Az, double Bx, double By, double Bz, double Cx, double Cy, double Cz)
    {
        Point3d A, B, C;
        Point3d T;
        vector<Point3d> points;
        // cout << "Input the point A¡¢B¡¢C: \n";
        // cin >> A.x >> A.y >> A.z >> B.x >> B.y >> B.z>> C.x>> C.y>> C.z;

        A.x = Ax;
        A.y = Ay;
        A.z = Az;
        B.x = Bx;
        B.y = By;
        B.z = Bz;
        C.x = Cx;
        C.y = Cy;
        C.z = Cz;

        vector<Vec3d>pointsthree;
        pointsthree = Findpoints(A.x, A.y, A.z, C.x, C.y, C.z,B.x, B.y, B.z);
        for (int i = 0; i < pointsthree.size(); i++) {
            T.x = pointsthree[i][0];
            T.y = pointsthree[i][1];
            T.z = pointsthree[i][2];
            points.push_back(T);
        }

        vector<double> xx, yy, zz;
        for (int i = 0; i < points.size(); i++)
        {
            xx.push_back(points[i].x);
            yy.push_back(points[i].y);
            zz.push_back(points[i].z);
        }
        struct line_fl result;
        result.xx = xx;
        result.yy = yy;
        result.zz = zz;


        return result;


        //    return points;

    }

vector<vector<line_fl>> all_lines(vector<vector<vector<double>>> all_weld_endpoints)
{
//    FILE *fp_0;
//    fp_0=fopen("/media/jiangnanxiangmu/江南项目专属/jiangnan0918/0820_gongjianmodel/hole_5.txt", "w"); /*建立一个文字文件只写*/

    vector<vector<line_fl>> all_weld_lines;
    for(int i = 0 ;i < all_weld_endpoints.size();i ++)
    {
        vector<line_fl> weld_line_1;
        for(int j = 1 ;j < all_weld_endpoints[i].size();j ++)
        {
            struct line_fl weld_line;
            if(all_weld_endpoints[i][j][3] == 1.0)
            {
//            cout << " ( " << all_weld_endpoints[i][j-1][0] << ' ' << all_weld_endpoints[i][j-1][1] << ' ' << all_weld_endpoints[i][j-1][2] << " ) , ( "
//                 << all_weld_endpoints[i][j][0] << ' ' << all_weld_endpoints[i][j][1] << ' ' << all_weld_endpoints[i][j][2] << " ) " << endl;
                weld_line = line_compute(all_weld_endpoints[i][j - 1][0] , all_weld_endpoints[i][j - 1][1] , all_weld_endpoints[i][j - 1][2],
                                        all_weld_endpoints[i][j][0] , all_weld_endpoints[i][j][1] , all_weld_endpoints[i][j][2]);
                weld_line_1.push_back(weld_line);

//                for(int k = 0;k < weld_line.xx.size();k ++)
//                    fprintf(fp_0 , "%5.2f  %5.2f  %5.2f\r\n" , weld_line.xx[k] , weld_line.yy[k] , weld_line.zz[k]);
            }
            else
            {
                weld_line = arc_compute(all_weld_endpoints[i][j - 1][0] , all_weld_endpoints[i][j - 1][1] , all_weld_endpoints[i][j - 1][2],
                                                all_weld_endpoints[i][j][4] , all_weld_endpoints[i][j][5] , all_weld_endpoints[i][j][6],
                                                all_weld_endpoints[i][j][0] , all_weld_endpoints[i][j][1] , all_weld_endpoints[i][j][2]);
                weld_line_1.push_back(weld_line);

//                for(int k = 0;k < weld_line.xx.size();k ++)
//                    fprintf(fp_0 , "%5.2f  %5.2f  %5.2f\r\n" , weld_line.xx[k] , weld_line.yy[k] , weld_line.zz[k]);
            }
//            fprintf(fp_0 , "\n\n");
        }

        all_weld_lines.push_back(weld_line_1);
    }

//    fclose(fp_0);
    return all_weld_lines;
}


vector<vector<vector<Point3d>>> all_arcs(vector<vector<vector<double>>> all_weld_endpoints)
{

    vector<vector<vector<Point3d>>> all_weld_lines;
    for(int i = 0 ;i < all_weld_endpoints.size();i ++)
    {
        vector<vector<Point3d>> weld_line_1;
        for(int j = 1 ;j < all_weld_endpoints[i].size();j ++)
        {

            vector<Point3d> points;
            if(all_weld_endpoints[i][j][3] == 0.0)
            {

//                cout << " ( " << all_weld_endpoints[i][j-1][0] << ' ' << all_weld_endpoints[i][j-1][1] << ' ' << all_weld_endpoints[i][j-1][2] << " ) , ( "
//                     << all_weld_endpoints[i][j][4] << ' ' << all_weld_endpoints[i][j][5] << ' ' << all_weld_endpoints[i][j][6] << " ) , ( "
//                     << all_weld_endpoints[i][j][0] << ' ' << all_weld_endpoints[i][j][1] << ' ' << all_weld_endpoints[i][j][2] << " ) " << endl;

//                points = arc_compute(all_weld_endpoints[i][j - 1][0] , all_weld_endpoints[i][j - 1][1] , all_weld_endpoints[i][j - 1][2],
//                        all_weld_endpoints[i][j][4] , all_weld_endpoints[i][j][5] , all_weld_endpoints[i][j][6],
//                        all_weld_endpoints[i][j][0] , all_weld_endpoints[i][j][1] , all_weld_endpoints[i][j][2]);

//                for(int k = 0;k < points.size();k ++)
//                {
//                    cout << points[k] << endl;
//                }
//                cout << endl;
                weld_line_1.push_back(points);
//                cout << "test" << endl;
            }
        }
        all_weld_lines.push_back(weld_line_1);
    }


    return all_weld_lines;
}

void all_lines_print(vector<vector<line_fl>> all_weld_lines)
{
//    cout << endl;
//    cout << "vector all_lines test" << endl;
//    for(int i = 0 ;i < all_weld_lines.size();i ++)
//    {
//        for(int j = 0;j < all_weld_lines[i].size();j ++)
//        {
//            for(int k = 0 ; k < all_weld_lines[i][j].xx.size();k ++)
//            {
//               // cout << all_weld_lines[i][j].xx[k] << ' ' << all_weld_lines[i][j].yy[k] << ' ' << all_weld_lines[i][j].zz[k] << endl;
//            }
//           // cout << endl;
////            int k = all_weld_lines[i][j].xx.size() - 1;
////            cout << all_weld_lines[i][j].xx[0] << ' ' << all_weld_lines[i][j].yy[0] << ' ' << all_weld_lines[i][j].zz[0] << endl;
////            cout << all_weld_lines[i][j].xx[k] << ' ' << all_weld_lines[i][j].yy[k] << ' ' << all_weld_lines[i][j].zz[k] << endl;
////            cout << endl;
//        }
//        cout << endl << endl;
//    }
}


/* Functions */

vector<Point3d> ago2cloud(vector<vector<line_fl>> lines1,vector<vector<line_fl>> lines2,vector<vector<line_fl>> lines3)
{
    vector<Point3d> cloudjson_back;
   // PointCloud::Ptr cloudjson_back(new PointCloud);
    Point3d PP0;
    for(int i = 0 ;i < lines1.size();i ++)
    {
        for(int j = 0;j < lines1[i].size();j ++)
        {
            for(int k = 0 ; k < lines1[i][j].xx.size();k ++)
            {
               // cout << lines1[i][j].xx[k] << ' ' << lines1[i][j].yy[k] << ' ' << lines1[i][j].zz[k] << endl;
                PP0.x = lines1[i][j].xx[k];
                PP0.y = lines1[i][j].yy[k];
                PP0.z = lines1[i][j].zz[k];
                cloudjson_back.push_back(PP0);
            }
            //int k = lines1[i][j].xx.size() - 1;
        }
    }
    for (int i = 0; i < lines2.size(); i++)
    {
        for (int j = 0; j < lines2[i].size(); j++)
        {
            for (int k = 0; k < lines2[i][j].xx.size(); k++)
            {
                //cout << lines2[i][j].xx[k] << ' ' << lines2[i][j].yy[k] << ' ' << lines2[i][j].zz[k] << endl;
                PP0.x = lines2[i][j].xx[k];
                PP0.y = lines2[i][j].yy[k];
                PP0.z = lines2[i][j].zz[k];
                cloudjson_back.push_back(PP0);
            }
            //int k = lines1[i][j].xx.size() - 1;
        }
    }
    for (int i = 0; i < lines3.size(); i++)
    {
        for (int j = 0; j < lines3[i].size(); j++)
        {
            for (int k = 0; k < lines3[i][j].xx.size(); k++)
            {
                //cout << lines3[i][j].xx[k] << ' ' << lines3[i][j].yy[k] << ' ' << lines3[i][j].zz[k] << endl;
                PP0.x = lines3[i][j].xx[k];
                PP0.y = lines3[i][j].yy[k];
                PP0.z = lines3[i][j].zz[k];
                cloudjson_back.push_back(PP0);
            }
            //int k = lines1[i][j].xx.size() - 1;
        }
    }
//    for(int i=0;i<=cloudjson_back.size();i++)
//    {
//        cloudjson_back[i].x=cloudjson_back[i].x-77000;
//        cloudjson_back[i].y-=370;
//        cloudjson_back[i].z-=6700;
//    }

    return cloudjson_back;
}

vector<vector<Point3d>> weld2cloud(vector<vector<line_fl>> lines1)
{
    vector<vector<Point3d>> weld_back;
    weld_back.resize(lines1.size());
    Point3d PP0;
    for(int j=0;j<lines1.size();j++)
    {
        for(int i=0;i<lines1[j][0].xx.size();i++)
        {
            PP0.x = lines1[j][0].xx[i];
            PP0.y = lines1[j][0].yy[i];
            PP0.z = lines1[j][0].zz[i];
            weld_back[j].push_back(PP0);
        }
    }

    return weld_back;
}


vector<Point3d> plate2cloud(vector<vector<line_fl>> lines1)
{
    vector<Point3d> plate_back;
    //plate_back.resize(lines1.size());
    Point3d PP0;

    for (int i = 0; i < lines1.size(); i++)
    {
        for (int j = 0; j < lines1[i].size(); j++)
        {
            for (int k = 0; k < lines1[i][j].xx.size(); k++)
            {
                //cout << lines3[i][j].xx[k] << ' ' << lines3[i][j].yy[k] << ' ' << lines3[i][j].zz[k] << endl;
                PP0.x = lines1[i][j].xx[k];
                PP0.y = lines1[i][j].yy[k];
                PP0.z = lines1[i][j].zz[k];
                plate_back.push_back(PP0);
            }
            //int k = lines1[i][j].xx.size() - 1;
        }
    }

    return plate_back;
}
vector<vector<Point3d>> json_all(char* Path)
{
    FILE            *fp = NULL;
    cJSON           *json;
    char            *out;
    char            line[1024] = {0};
    //char            context[200000] = {0};
    char* context=(char*)malloc(1000000);
    memset(context,'\0',1000000);
    int             len_whole = 0;

    // read the whole json file
    //cout<<"path==="<<Path<<endl;
if(NULL != (fp = fopen(Path, "r")))
    {
        while (NULL != fgets(line, sizeof(line), fp)) {
            strcat(context , line);
        }
    }

    // parse the json file
    json = cJSON_Parse(context);
    len_whole = cJSON_GetArraySize(json);

    //cout<<"len_whole==="<<len_whole<<endl;

   struct WHOLE whole[len_whole];

    for(int i = 0;i < len_whole;i ++)
    {
        cJSON *object = cJSON_GetArrayItem(json,i);
        cJSON *type = cJSON_GetObjectItem(object , "type");
        cJSON *name = cJSON_GetObjectItem(object , "name");
        cJSON *contour = cJSON_GetObjectItem(object , "contour");
        cJSON *start_point = cJSON_GetObjectItem(contour , "start_point");
        cJSON *start_point_x = cJSON_GetObjectItem(start_point , "X");
        cJSON *start_point_y = cJSON_GetObjectItem(start_point , "Y");
        cJSON *start_point_z = cJSON_GetObjectItem(start_point , "Z");
        cJSON *lis_segments = cJSON_GetObjectItem(contour , "lis_segments");


        strcpy(whole[i].type , type->valuestring);
        strcpy(whole[i].name , name->valuestring);
        whole[i].contour.start_point.X = start_point_x->valuedouble;
        whole[i].contour.start_point.Y = start_point_y->valuedouble;
        whole[i].contour.start_point.Z = start_point_z->valuedouble;

        int len_lis_segments = cJSON_GetArraySize(lis_segments);
        whole[i].contour.num_segment = len_lis_segments;

        // cout << len_lis_segments << endl;lis_segment数量过多增加数组容量
        for(int j = 0; j < len_lis_segments;j ++)
        {
            cJSON *segment = cJSON_GetArrayItem(lis_segments , j);
            cJSON *amplitude = cJSON_GetObjectItem(segment , "amplitude");
            cJSON *amplitude_x = cJSON_GetObjectItem(amplitude , "X");
            cJSON *amplitude_y = cJSON_GetObjectItem(amplitude , "Y");
            cJSON *amplitude_z = cJSON_GetObjectItem(amplitude , "Z");

            cJSON *end_point = cJSON_GetObjectItem(segment , "end_point");
            cJSON *end_point_x = cJSON_GetObjectItem(end_point , "X");
            cJSON *end_point_y = cJSON_GetObjectItem(end_point , "Y");
            cJSON *end_point_z = cJSON_GetObjectItem(end_point , "Z");

            whole[i].contour.lis_segments[j].amplitude.X = amplitude_x->valuedouble;
            whole[i].contour.lis_segments[j].amplitude.Y = amplitude_y->valuedouble;
            whole[i].contour.lis_segments[j].amplitude.Z = amplitude_z->valuedouble;

            whole[i].contour.lis_segments[j].end_point.X = end_point_x->valuedouble;
            whole[i].contour.lis_segments[j].end_point.Y = end_point_y->valuedouble;
            whole[i].contour.lis_segments[j].end_point.Z = end_point_z->valuedouble;
        }

        // printf("%s\n" , cJSON_Print(start_point_x));
        cout<<"-----"<<endl;


    }

    //cout<<"json.cpp_________________cJSON_Delete(json)-----------1"<<endl;
    cJSON_Delete(json);
    fclose(fp);

    // json_print(len_whole , whole);

    vector<vector<vector<double>>> all_weld_endpoints = all_points("weld" , len_whole , whole);
    //cout<<"Finish Weld"<<endl;
    vector<vector<vector<double>>> all_hole_endpoints = all_points("hole" , len_whole , whole);
    //cout<<"Finish Hole"<<endl;
    vector<vector<vector<double>>> all_plate_endpoints = all_points("plate" , len_whole , whole);





    //cout<<"Finish Plate"<<endl;
    vector<vector<vector<double>>> all_stiffener_endpoints = all_points("stiffener" , len_whole , whole);


    vector<vector<line_fl>> all_weld_lines = all_lines(all_weld_endpoints);
    vector<vector<line_fl>> all_hole_lines = all_lines(all_hole_endpoints);
    vector<vector<line_fl>> all_plate_lines = all_lines(all_plate_endpoints);

    vector<vector<line_fl>> all_stiffener_lines = all_lines(all_stiffener_endpoints);
    //cout<<"Finish json"<<endl;

    vector<vector<Point3d>> all_point;
    all_point.resize(3);
    all_point[0]=ago2cloud(all_weld_lines,all_hole_lines,all_plate_lines);
    all_point[1] = plate2cloud(all_plate_lines);
    all_point[2] = plate2cloud(all_stiffener_lines);
    //cout<<"json.cpp_________________free(context)------1"<<endl;
    free(context);
    int num=0;
           PointCloud::Ptr Point(new PointCloud);


           for(int i=0;i<all_point.size();i++)
           {
           
               
               
               
               
               
               for(int j=0;j<all_point[i].size();j++)
               {


                   Point->width=num+1;
                   Point->height=1;
                   Point->points.resize(num+1);
                   Point->points[num].x=all_point[i][j].x;
                   Point->points[num].y=all_point[i][j].y;
                   Point->points[num].z=all_point[i][j].z;
                   num++;
               }
           }
           //pcl::io::savePCDFileASCII("Point.pcd",*Point);
           //viewPair1(Point);
    return all_point;

}

//vector<Point3d> json_all(char* Path)
//{
//    FILE            *fp = NULL;
//    cJSON           *json;
//    char            *out;
//    char            line[1024] = {0};
//    //char            context[200000] = {0};
//    char* context=(char*)malloc(1000000);
//    memset(context,'\0',1000000);
//    int             len_whole = 0;

//    // read the whole json file
//    //cout<<"path==="<<Path<<endl;
//if(NULL != (fp = fopen(Path, "r")))
//    {
//        while (NULL != fgets(line, sizeof(line), fp)) {
//            strcat(context , line);
//        }
//    }

//    // parse the json file
//    json = cJSON_Parse(context);
//    len_whole = cJSON_GetArraySize(json);

//    //cout<<"len_whole==="<<len_whole<<endl;

//    struct WHOLE whole[len_whole];

//    for(int i = 0;i < len_whole;i ++)
//    {
//        cJSON *object = cJSON_GetArrayItem(json,i);
//        cJSON *type = cJSON_GetObjectItem(object , "type");
//        cJSON *name = cJSON_GetObjectItem(object , "name");
//        cJSON *contour = cJSON_GetObjectItem(object , "contour");
//        cJSON *start_point = cJSON_GetObjectItem(contour , "start_point");
//        cJSON *start_point_x = cJSON_GetObjectItem(start_point , "X");
//        cJSON *start_point_y = cJSON_GetObjectItem(start_point , "Y");
//        cJSON *start_point_z = cJSON_GetObjectItem(start_point , "Z");
//        cJSON *lis_segments = cJSON_GetObjectItem(contour , "lis_segments");


//        strcpy(whole[i].type , type->valuestring);
//        strcpy(whole[i].name , name->valuestring);
//        whole[i].contour.start_point.X = start_point_x->valuedouble;
//        whole[i].contour.start_point.Y = start_point_y->valuedouble;
//        whole[i].contour.start_point.Z = start_point_z->valuedouble;

//        int len_lis_segments = cJSON_GetArraySize(lis_segments);
//        whole[i].contour.num_segment = len_lis_segments;

//        // cout << len_lis_segments << endl;
//        for(int j = 0; j < len_lis_segments;j ++)
//        {
//            cJSON *segment = cJSON_GetArrayItem(lis_segments , j);
//            cJSON *amplitude = cJSON_GetObjectItem(segment , "amplitude");
//            cJSON *amplitude_x = cJSON_GetObjectItem(amplitude , "X");
//            cJSON *amplitude_y = cJSON_GetObjectItem(amplitude , "Y");
//            cJSON *amplitude_z = cJSON_GetObjectItem(amplitude , "Z");

//            cJSON *end_point = cJSON_GetObjectItem(segment , "end_point");
//            cJSON *end_point_x = cJSON_GetObjectItem(end_point , "X");
//            cJSON *end_point_y = cJSON_GetObjectItem(end_point , "Y");
//            cJSON *end_point_z = cJSON_GetObjectItem(end_point , "Z");

//            whole[i].contour.lis_segments[j].amplitude.X = amplitude_x->valuedouble;
//            whole[i].contour.lis_segments[j].amplitude.Y = amplitude_y->valuedouble;
//            whole[i].contour.lis_segments[j].amplitude.Z = amplitude_z->valuedouble;

//            whole[i].contour.lis_segments[j].end_point.X = end_point_x->valuedouble;
//            whole[i].contour.lis_segments[j].end_point.Y = end_point_y->valuedouble;
//            whole[i].contour.lis_segments[j].end_point.Z = end_point_z->valuedouble;
//        }

//        // printf("%s\n" , cJSON_Print(start_point_x));


//    }

//    //cout<<"json.cpp_________________cJSON_Delete(json)-----------1"<<endl;
//    cJSON_Delete(json);
//    fclose(fp);

//    // json_print(len_whole , whole);

//    vector<vector<vector<double>>> all_weld_endpoints = all_points("weld" , len_whole , whole);
//    //cout<<"Finish Weld"<<endl;
//    vector<vector<vector<double>>> all_hole_endpoints = all_points("hole" , len_whole , whole);
//    //cout<<"Finish Hole"<<endl;
//    vector<vector<vector<double>>> all_plate_endpoints = all_points("plate" , len_whole , whole);
//    //cout<<"Finish Plate"<<endl;

//    // print_all_points(all_weld_endpoints);
//    // print_all_points(all_hole_endpoints);
//    // print_all_points(all_plate_endpoints);


//    vector<vector<line_fl>> all_weld_lines = all_lines(all_weld_endpoints);
//    vector<vector<line_fl>> all_hole_lines = all_lines(all_hole_endpoints);
//    vector<vector<line_fl>> all_plate_lines = all_lines(all_plate_endpoints);

//    //cout<<"Finish json"<<endl;

////    all_lines_print(all_weld_lines);
////    all_lines_print(all_hole_lines);
////    all_lines_print(all_plate_lines);
//    vector<Point3d> all_point;
//    all_point=ago2cloud(all_weld_lines,all_hole_lines,all_plate_lines);

//    //cout<<"json.cpp_________________free(context)------1"<<endl;
//    free(context);
//    return all_point;

//}



vector<vector<Point3d>> json_hf(char* Path)
{
    FILE            *fp = NULL;
    cJSON           *json;
    char            *out;
    char            line[1024] = {0};
    //char            context[200000] = {0};
    char* context=(char*)malloc(1000000);
    memset(context,'\0',1000000);
    int             len_whole = 0;

    // read the whole json file
if(NULL != (fp = fopen(Path, "r")))
    {
        while (NULL != fgets(line, sizeof(line), fp)) {
            strcat(context , line);
        }
    }
   // cout<<"read the whole json file"<<endl;
    // parse the json file
    json = cJSON_Parse(context);
    len_whole = cJSON_GetArraySize(json);

  struct WHOLE whole[len_whole];
    //cout<<"for has started!!"<<endl;
    for(int i = 0;i < len_whole;i ++)
    {
        cJSON *object = cJSON_GetArrayItem(json,i);
        cJSON *type = cJSON_GetObjectItem(object , "type");
        cJSON *name = cJSON_GetObjectItem(object , "name");
        cJSON *contour = cJSON_GetObjectItem(object , "contour");
        cJSON *start_point = cJSON_GetObjectItem(contour , "start_point");
        cJSON *start_point_x = cJSON_GetObjectItem(start_point , "X");
        cJSON *start_point_y = cJSON_GetObjectItem(start_point , "Y");
        cJSON *start_point_z = cJSON_GetObjectItem(start_point , "Z");
        cJSON *lis_segments = cJSON_GetObjectItem(contour , "lis_segments");


        strcpy(whole[i].type , type->valuestring);
        strcpy(whole[i].name , name->valuestring);
        whole[i].contour.start_point.X = start_point_x->valuedouble;
        whole[i].contour.start_point.Y = start_point_y->valuedouble;
        whole[i].contour.start_point.Z = start_point_z->valuedouble;

        int len_lis_segments = cJSON_GetArraySize(lis_segments);
        whole[i].contour.num_segment = len_lis_segments;

        // cout << len_lis_segments << endl;
        for(int j = 0; j < len_lis_segments;j ++)
        {
            cJSON *segment = cJSON_GetArrayItem(lis_segments , j);
            cJSON *amplitude = cJSON_GetObjectItem(segment , "amplitude");
            cJSON *amplitude_x = cJSON_GetObjectItem(amplitude , "X");
            cJSON *amplitude_y = cJSON_GetObjectItem(amplitude , "Y");
            cJSON *amplitude_z = cJSON_GetObjectItem(amplitude , "Z");

            cJSON *end_point = cJSON_GetObjectItem(segment , "end_point");
            cJSON *end_point_x = cJSON_GetObjectItem(end_point , "X");
            cJSON *end_point_y = cJSON_GetObjectItem(end_point , "Y");
            cJSON *end_point_z = cJSON_GetObjectItem(end_point , "Z");

            whole[i].contour.lis_segments[j].amplitude.X = amplitude_x->valuedouble;
            whole[i].contour.lis_segments[j].amplitude.Y = amplitude_y->valuedouble;
            whole[i].contour.lis_segments[j].amplitude.Z = amplitude_z->valuedouble;

            whole[i].contour.lis_segments[j].end_point.X = end_point_x->valuedouble;
            whole[i].contour.lis_segments[j].end_point.Y = end_point_y->valuedouble;
            whole[i].contour.lis_segments[j].end_point.Z = end_point_z->valuedouble;
        }

        // printf("%s\n" , cJSON_Print(start_point_x));


    }

    //cout<<"json.cpp_________________cJSON_Delete(json)-----------2"<<endl;
    cJSON_Delete(json);
    fclose(fp);

    // json_print(len_whole , whole);


    vector<vector<vector<double>>> all_weld_endpoints = all_points("weld" , len_whole , whole);
    vector<vector<vector<double>>> all_hole_endpoints = all_points("hole" , len_whole , whole);
    vector<vector<vector<double>>> all_plate_endpoints = all_points("plate" , len_whole , whole);

    // print_all_points(all_weld_endpoints);
    // print_all_points(all_hole_endpoints);
    // print_all_points(all_plate_endpoints);


    vector<vector<line_fl>> all_weld_lines = all_lines(all_weld_endpoints);
    vector<vector<line_fl>> all_hole_lines = all_lines(all_hole_endpoints);
    vector<vector<line_fl>> all_plate_lines = all_lines(all_plate_endpoints);

//    all_lines_print(all_weld_lines);
//    all_lines_print(all_hole_lines);
//    all_lines_print(all_plate_lines);
    vector<vector<Point3d>> weld_back;
    weld_back=weld2cloud(all_weld_lines);
    //cout<<"json.cpp_________________free(context)-----------2"<<endl;
     free(context);
    return  weld_back;
}

