#ifndef REALSENSECTL_H
#define REALSENSECTL_H
#include"common.h"
#include"PartRealSense/realsenseh.h"

class realsensehlp{
public:
//    rs2::pipeline_profile selection_obser;
//    int threshold_obser=1;
//    char path_obser[100];

    void* dealhandle(void* args)
    {
        Observer* pObserver_deal = (Observer *)args;

        Mat rgb;
        Mat depth;
        Mat color_rgb=Mat::zeros(540,960,CV_8UC3);;
        Mat color_dep=Mat::zeros(360,640,CV_8UC3);;
        sleep(3);
        double max=7000,min=10;
        double alpha=255.0/(max-min);
        bool stop = false;
        int i=0;
        Point P1;
        Point P2;
        P1.x=1107;
        P1.y=432 ;
        P2.x=1418;
        P2.y=606 ;
        while(!stop)
        {
            if(pObserver_deal->rgb_isnew)
            {
                i++;
                pObserver_deal->Get_rgb().copyTo(rgb);
                cv::putText(rgb,to_string(i),cv::Point(50,50),cv::FONT_HERSHEY_SIMPLEX,1.5,(255,255,255),1);

            }
            if(pObserver_deal->dep_isnew)
            {
                pObserver_deal->Get_dep().copyTo(depth);
                depth.convertTo(depth,CV_8U,alpha,-min*alpha);
                cv::applyColorMap(depth, depth, cv::COLORMAP_JET);
                resize(depth,color_dep,Size(640,360));
            }
            cv::rectangle(rgb,P1 ,P2, CV_RGB(255, 255, 0), 2);
            uint32_t distance=RoiDistanceInColor(depth,rgb,pObserver_deal->Get_rs2(),P1,P2);
            resize(rgb,color_rgb,Size(960,540));
            cout<<distance<<endl;
            moveWindow("color Image",0,0);
            moveWindow("depth Image",1080,0);
            imshow("color Image", color_rgb);
            imshow("depth Image", color_dep);
            waitKey(10);
        }
    }

    float get_depth_scale(rs2::device dev)
    {
        // Go over the device's sensors
        for (rs2::sensor& sensor : dev.query_sensors())
        {
            // Check if the sensor if a depth sensor
            if (rs2::depth_sensor dpt = sensor.as<rs2::depth_sensor>())
            {
                return dpt.get_depth_scale();
            }
        }
        throw std::runtime_error("Device does not have a depth sensor");
    }
    //深度图对齐到彩色图函数
    Mat align_Depth2Color(Mat depth,Mat color,rs2::pipeline_profile profile){
        //声明数据流
        auto depth_stream=profile.get_stream(RS2_STREAM_DEPTH).as<rs2::video_stream_profile>();
        auto color_stream=profile.get_stream(RS2_STREAM_COLOR).as<rs2::video_stream_profile>();

        //获取内参
        const auto intrinDepth=depth_stream.get_intrinsics();
        const auto intrinColor=color_stream.get_intrinsics();

        //直接获取从深度摄像头坐标系到彩色摄像头坐标系的欧式变换矩阵
        //auto  extrinDepth2Color=depth_stream.get_extrinsics_to(color_stream);
        rs2_extrinsics  extrinDepth2Color;
        rs2_error *error;
        rs2_get_extrinsics(depth_stream,color_stream,&extrinDepth2Color,&error);

        //平面点定义
        float pd_uv[2],pc_uv[2];
        //空间点定义
        float Pdc3[3],Pcc3[3];

        //获取深度像素与现实单位比例（D435默认1毫米）
        float depth_scale = get_depth_scale(profile.get_device());
    //    uint16_t depth_max=0;
    //    for(int row=0;row<depth.rows;row++){
    //        for(int col=0;col<depth.cols;col++){
    //            if(depth_max<depth.at<uint16_t>(row,col))
    //                depth_max=depth.at<uint16_t>(row,col);
    //        }
    //    }
        int y=0,x=0;
        //初始化结果
        Mat result=Mat::zeros(color.rows,color.cols,CV_8UC3);
        //对深度图像遍历

        for(int row=0;row<depth.rows;row++){
            for(int col=0;col<depth.cols;col++){
                //将当前的(x,y)放入数组pd_uv，表示当前深度图的点
                pd_uv[0]=col;
                pd_uv[1]=row;
                //取当前点对应的深度值
                uint16_t depth_value=depth.at<uint16_t>(row,col);
                //换算到米
                float depth_m=depth_value*depth_scale;
                //将深度图的像素点根据内参转换到深度摄像头坐标系下的三维点
                rs2_deproject_pixel_to_point(Pdc3,&intrinDepth,pd_uv,depth_m);
                //将深度摄像头坐标系的三维点转化到彩色摄像头坐标系下
                rs2_transform_point_to_point(Pcc3,&extrinDepth2Color,Pdc3);
                //将彩色摄像头坐标系下的深度三维点映射到二维平面上
                rs2_project_point_to_pixel(pc_uv,&intrinColor,Pcc3);

                //取得映射后的（u,v)
                x=(int)pc_uv[0];
                y=(int)pc_uv[1];
    //            if(x<0||x>color.cols)
    //                continue;
    //            if(y<0||y>color.rows)
    //                continue;
                //最值限定
                x=x<0? 0:x;
                x=x>color.cols-1 ? color.cols-1:x;
                y=y<0? 0:y;
                y=y>color.rows-1 ? color.rows-1:y;

                //将成功映射的点用彩色图对应点的RGB数据覆盖
                for(int k=0;k<3;k++){
                    //这里设置了只显示1米距离内的东西
                    if(depth_m<10)
                    result.at<cv::Vec3b>(y,x)[k]=
                            color.at<cv::Vec3b>(y,x)[k];
                }
            }
        }
        return result;
    }

    //roi depth in color image
    uint32_t RoiDistanceInColor(Mat depth,Mat color,rs2::pipeline_profile profile,Point P1,Point P2)
    {
        //声明数据流
        auto depth_stream=profile.get_stream(RS2_STREAM_DEPTH).as<rs2::video_stream_profile>();
        auto color_stream=profile.get_stream(RS2_STREAM_COLOR).as<rs2::video_stream_profile>();

        //获取内参
        const auto intrinDepth=depth_stream.get_intrinsics();
        const auto intrinColor=color_stream.get_intrinsics();

        //直接获取从深度摄像头坐标系到彩色摄像头坐标系的欧式变换矩阵
        //auto  extrinDepth2Color=depth_stream.get_extrinsics_to(color_stream);
        rs2_extrinsics  extrinDepth2Color;
        rs2_error *error;
        rs2_get_extrinsics(depth_stream,color_stream,&extrinDepth2Color,&error);

        //平面点定义
        float pd_uv[2],pc_uv[2];
        //空间点定义
        float Pdc3[3],Pcc3[3];

        //获取深度像素与现实单位比例（D435默认1毫米）
        float depth_scale = get_depth_scale(profile.get_device());
    //    uint16_t depth_max=0;
    //    for(int row=0;row<depth.rows;row++){
    //        for(int col=0;col<depth.cols;col++){
    //            if(depth_max<depth.at<uint16_t>(row,col))
    //                depth_max=depth.at<uint16_t>(row,col);
    //        }
    //    }
        int y=0,x=0;
        //初始化结果
    //    Mat result=Mat::zeros(color.rows,color.cols,CV_8UC3);
        //对深度图像遍历
        uint32_t average=0;
        vector<uint32_t> points;
        for(int row=0;row<depth.rows;row++)
        {
            for(int col=0;col<depth.cols;col++)
            {
                //将当前的(x,y)放入数组pd_uv，表示当前深度图的点
                pd_uv[0]=col;
                pd_uv[1]=row;
                //取当前点对应的深度值
                uint16_t depth_value=depth.at<uint16_t>(row,col);
                //换算到米
                float depth_m=depth_value*depth_scale;
                //将深度图的像素点根据内参转换到深度摄像头坐标系下的三维点
                rs2_deproject_pixel_to_point(Pdc3,&intrinDepth,pd_uv,depth_m);
                //将深度摄像头坐标系的三维点转化到彩色摄像头坐标系下
                rs2_transform_point_to_point(Pcc3,&extrinDepth2Color,Pdc3);
                //将彩色摄像头坐标系下的深度三维点映射到二维平面上
                rs2_project_point_to_pixel(pc_uv,&intrinColor,Pcc3);

                //取得映射后的（u,v)
                x=(int)pc_uv[0];
                y=(int)pc_uv[1];

                //最值限定
    //            bool isinroi=true;
    //            isinroi=x<P1.x? false:true;
    //            isinroi=x>P2.x? false:true;
    //            isinroi=y<P1.y? false:true;
    //            isinroi=y>P2.y? false:true;

                if(x>P1.x && x<P2.x && y>P1.y && y<P2.y)
                {
    //                if(depth_m<10)
    //                {sum +=depth_m;
    //                    num ++ ;}
                    if(depth_value != 0)
                    {points.push_back(depth_value);}
    //                num ++ ;
                }
            }
        }
    //    imshow ("colorblock",result);
    //    waitKey(10);
        sort(points.begin(),points.end());
        int num = points.size()*0.5;
        int sum=0;
        for(int i;i<num;i++)
        {sum +=points[i];}
        average = sum/num;
        return average;
    }

    Point readpoints_1(char* filetxt)
    {
        int x1tmp,y1tmp,x2tmp,y2tmp;
        ifstream fr(filetxt);
        Point P1;
        if (! fr.is_open())
        {cout<<"Error opening file";}
        fr>>x1tmp>>y1tmp>>x2tmp>>y2tmp;
        P1.x=x1tmp;
        P1.y=y1tmp;
        fr.close();
        return P1;
    }

    Point readpoints_2(char* filetxt)
    {
        int x1tmp,y1tmp,x2tmp,y2tmp;
        ifstream fr(filetxt);
        Point P2;
        if (! fr.is_open())
        {cout<<"Error opening file";}
        fr>>x1tmp>>y1tmp>>x2tmp>>y2tmp;
        P2.x=x2tmp;
        P2.y=y2tmp;
        fr.close();
        return P2;
    }
};

class realsensectl
{
public:
    void myinit();
    vector<Mat> oneShot() ;
    uint32_t getDistance(Rect rect ,vector<Mat> frames =vector<Mat>()){
        frames=this->myframes;
        Point point1=Point(rect.x,rect.y);
        Point point2=Point(rect.x+rect.width,rect.y+rect.height);
        uint32_t dis = rsh.RoiDistanceInColor(frames[1],frames[0],this->myrs2,point1,point2);
        return dis ;
    }

private:
    rs2::pipeline_profile myrs2;
    vector<Mat> myframes;

    ConcreteSubject *pSubject= new ConcreteSubject() ;
    Observer *pObserver_deal = new ConcreteObserver(pSubject);

    realsensehlp rsh;
    //static
public:
    static bool initFlg ;
private:

     static void realsense(ConcreteSubject *pSubject);
};






#endif // REALSENSECTL_H
