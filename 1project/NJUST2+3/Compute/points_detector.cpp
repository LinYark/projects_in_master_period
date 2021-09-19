//===================================================================================================================================
#include <opencv2/opencv.hpp>
#include <iostream>
#include "points_detector.h"
//-------------------------------------
using namespace cv;
using namespace std;

Points_Processing::Points_Processing()
{
//    Net1();
//    r			= 2;			//nms最大半径
//    s			= 2;			//最大边界半径
//    m			= 1;			//最大监督系数
//    nThreads	= 12;			//用于计算的线程数
}

//==============================================================================================================
// 边缘检测主函数
//==============================================================================================================
Mat Points_Processing::Img_detector(Mat origiImage)
{
    Mat image = origiImage;

    vector<Mat> IMG_ceils,Det_ceils; //切割块vector
    IMG_ceils = Image_Cut(image);

    for (uint i = 0; i < IMG_ceils.size(); i++)
    {
        Mat result;
        Mat img = IMG_ceils[i];
        result=Predict(img);
        result.convertTo(result,CV_8U,255);
        Det_ceils.push_back(result);
    }

    Mat imageDete = Image_Stitching(Det_ceils);

    imageDete =imageDete*255;

    Mat weldThin = Mat::zeros(imageDete.rows, imageDete.cols, CV_8UC1);
    ImgThin(imageDete, weldThin, 5);
    Mat WeldBinary;
    threshold(weldThin, WeldBinary, 180, 255, THRESH_BINARY);
    Mat Weld_RemoveSmall = Mat::zeros(imageDete.rows, imageDete.cols, CV_8UC1);
    RemoveSmallRegion(WeldBinary, Weld_RemoveSmall, 10, 1, 1);
    Mat Weld_Dilate = Mat::zeros(imageDete.rows, imageDete.cols, CV_8UC1);
    Mat element = getStructuringElement(MORPH_RECT, Size(4, 4));
    dilate(Weld_RemoveSmall, Weld_Dilate, element);

    return Weld_Dilate;
}
//==============================================================================================================
// RCF网络初始化
//==============================================================================================================
void Points_Processing::Net1()
{
    path_source         = PP->compute_path().toStdString();
    model_file   = path_source+"/wangluo/jiangnan-RCF_new.prototxt";        //训练模型地址
    trained_file = path_source+"/wangluo/rcf__points_iter_29000.caffemodel";       //网络地址
    //Caffe网络初始化------------------------------------------------
    Caffe::set_mode(Caffe::GPU);                    //设置计算模式为GPU
    net_.reset(new Net<float>(model_file,TEST));    //加载网络训练模型
    net_->CopyTrainedLayersFrom(trained_file);      //加载网络

    CHECK_EQ(net_->num_inputs(), 1) << "Network should have exactly one input.";
    CHECK_EQ(net_->num_outputs(), 1) << "Network should have exactly one output.";
    //cout<<net_->num_outputs();

    //输入层
    Blob<float>* input_layer = net_->input_blobs()[0];
    num_channels_ = input_layer->channels();
    //输入层一般是彩色图像、或灰度图像，因此需要进行判断
    CHECK(num_channels_ == 3 || num_channels_ == 1) << "Input layer should have 1 or 3 channels.";
    //网络输入层的图片的大小
    input_geometry_ = cv::Size(input_layer->width(), input_layer->height());
}

//==============================================================================================================
//Caffe预测函数，输入一张图片
//==============================================================================================================
Mat Points_Processing::Predict(const cv::Mat& img)
{
    Blob<float>* input_layer = net_->input_blobs()[0];
    input_layer->Reshape(1, num_channels_, input_geometry_.height, input_geometry_.width);
    net_->Reshape();

    //输入带预测的图片数据，然后进行预处理，包括归一化、缩放等操作-------------
    std::vector<cv::Mat> input_channels;
    WrapInputLayer(&input_channels);
    Preprocess(img, &input_channels);

    //前向传导---------------------------------------------------------
    net_->Forward();
    Blob<float>* output_layer = net_->output_blobs()[0];
    cv::Mat merged_output_image = cv::Mat(output_layer->height(), output_layer->width(), CV_32FC1, const_cast<float *>(output_layer->cpu_data()));
    return merged_output_image;

}

//==============================================================================================================
//Caffe这个其实是为了获得net_网络的输入层数据的指针，然后后面我们直接把输入图片数据拷贝到这个指针里面
//==============================================================================================================
void Points_Processing::WrapInputLayer(std::vector<cv::Mat>* input_channels)
{
    Blob<float>* input_layer = net_->input_blobs()[0];
    int width = input_layer->width();
    int height = input_layer->height();
    float* input_data = input_layer->mutable_cpu_data();

    for (int i = 0; i < input_layer->channels(); ++i)
    {
        cv::Mat channel(height, width, CV_32FC1, input_data);
        input_channels->push_back(channel);
        input_data += width * height;
    }
}

//==============================================================================================================
//Caffe图片预处理函数，包括图片缩放、归一化、3通道图片分开存储
//对于三通道输入CNN，经过该函数返回的是std::vector<cv::Mat>因为是三通道数据，索引用了vector
//==============================================================================================================
void Points_Processing::Preprocess(const cv::Mat& img, std::vector<cv::Mat>* input_channels)
{

    //1、通道处理，因为我们如果是Alexnet网络，那么就应该是三通道输入
    cv::Mat sample;
    //如果输入图片是一张彩色图片，但是CNN的输入是一张灰度图像，那么我们需要把彩色图片转换成灰度图片
    if (img.channels() == 3 && num_channels_ == 1)
        cv::cvtColor(img, sample, cv::COLOR_BGR2GRAY);
    else if (img.channels() == 4 && num_channels_ == 1)
        cv::cvtColor(img, sample, cv::COLOR_BGRA2GRAY);
    //如果输入图片是灰度图片，或者是4通道图片，而CNN的输入要求是彩色图片，因此我们也需要把它转化成三通道彩色图片
    else if (img.channels() == 4 && num_channels_ == 3)
        cv::cvtColor(img, sample, cv::COLOR_BGRA2BGR);
    else if (img.channels() == 1 && num_channels_ == 3)
        cv::cvtColor(img, sample, cv::COLOR_GRAY2BGR);
    else
     sample = img;
    //缩放处理，因为我们输入的一张图片如果是任意大小的图片，那么我们就应该把它缩放到
    cv::Mat sample_resized;
    if (sample.size() != input_geometry_)
        cv::resize(sample, sample_resized, input_geometry_);
    else
        sample_resized = sample;
    //数据类型处理，因为我们的图片是uchar类型，我们需要把数据转换成float类型
    cv::Mat sample_float;
    if (num_channels_ == 3)
        sample_resized.convertTo(sample_float, CV_32FC3);
    else
        sample_resized.convertTo(sample_float, CV_32FC1);
    //均值归一化
    cv::Mat sample_normalized;
    cv::subtract(sample_float, 0, sample_normalized);

    //通道数据分开存储
    cv::split(sample_normalized, *input_channels);

    CHECK(reinterpret_cast<float*>(input_channels->at(0).data)
        == net_->input_blobs()[0]->cpu_data())
        << "Input channels are not wrapping the input layer of the network.";
}

//==============================================================================================================
// 切割图片函数
//==============================================================================================================
vector<Mat> Points_Processing::Image_Cut(const Mat  image)
{
    vector<Mat> ceilImg;

    width = image.cols;
    height = image.rows;

    cut_width = (width - Dst_img_width_comp * 2) / (Dst_img_width - Dst_img_width_comp * 2);
    cut_height = (height - Dst_img_height_comp * 2) / (Dst_img_height - Dst_img_height_comp * 2);

    ceil_down_width = width - cut_width * (Dst_img_width - Dst_img_width_comp * 2) - Dst_img_width_comp * 2;
    ceil_down_height = height - cut_height * (Dst_img_height - Dst_img_height_comp * 2) - Dst_img_height_comp * 2;

    if (ceil_down_width > 0)
    {
        cut_width += 1;
    }
    //else
    //{
    //	ceil_down_width = Dst_img_width;
    //}

    if (ceil_down_height > 0)
    {
        cut_height += 1;
    }
    //else
    //{
    //	ceil_down_height = Dst_img_height;
    //}

    for (int i = 0; i < cut_height - 1; i++)
    {
        for (int j = 0; j < cut_width; j++)
        {
            if (j < cut_width - 1)
            {
                Rect rect(j*Dst_img_width - j * Dst_img_width_comp * 2,   \
                          i*Dst_img_height - i * Dst_img_height_comp * 2, \
                          Dst_img_width,                                  \
                          Dst_img_height);
                ceilImg.push_back(image(rect));
            }
            else  //最右端
            {
                Rect rect(width - Dst_img_width,                          \
                          i*Dst_img_height - i * Dst_img_height_comp * 2, \
                          Dst_img_width,                                  \
                          Dst_img_height);
                ceilImg.push_back(image(rect));
            }
        }
    }

    for (int i = 0; i < cut_width; i++)
    {
        if (i < cut_width - 1)
        {
            Rect rect(i*Dst_img_width - i * Dst_img_width_comp * 2, \
                      height - Dst_img_height,                      \
                      Dst_img_width,                                \
                      Dst_img_height);
            ceilImg.push_back(image(rect));
        }
        else   //右下角
        {
            Rect rect(width - Dst_img_width,   \
                      height - Dst_img_height, \
                      Dst_img_width,           \
                      Dst_img_height);
            ceilImg.push_back(image(rect));
        }
    }

//    for (int i = 0; i < ceilImg.size(); i++)
//    {
//        Mat A = ceilImg[i];
//        imwrite("D:/江南项目/cuttest/A"+num2str(i)+".bmp",A);
//    }


    return ceilImg;
}

//==============================================================================================================
// 图片合并
//==============================================================================================================
Mat Points_Processing::Image_Stitching(vector<Mat> Imgs_in)
{
    Mat srcImage(height, width, CV_8UC1, Scalar(0));
    Mat imageROI;
    vector<Mat> Imgs;

    for (uint i = 0; i < Imgs_in.size(); i++)
    {
//        Mat A = Imgs_in[i];
//        Mat B = 255 * (1 - A);
        Rect rect(Dst_img_width_comp,                      \
                  Dst_img_height_comp,                     \
                  Dst_img_width - Dst_img_width_comp * 2,  \
                  Dst_img_height - Dst_img_height_comp * 2);
        Imgs.push_back(Imgs_in[i](rect));
    }

    for (int i = 0; i < cut_height - 1; i++)
    {
        for (int j = 0; j < cut_width; j++)
        {
            if (j < cut_width - 1)
            {
                imageROI = srcImage(Rect(j*(Dst_img_width - Dst_img_width_comp * 2) + Dst_img_width_comp,   \
                                         i*(Dst_img_height - Dst_img_height_comp * 2) + Dst_img_height_comp,\
                                         Dst_img_width - Dst_img_width_comp * 2,                            \
                                         Dst_img_height - Dst_img_height_comp * 2));
                Imgs[uint(i * cut_width + j)].copyTo(imageROI);
            }
            else
            {
                imageROI = srcImage(Rect(width - (Dst_img_width - Dst_img_width_comp * 2) - Dst_img_width_comp, \
                                         i*(Dst_img_height - Dst_img_height_comp * 2) + Dst_img_height_comp,    \
                                         Dst_img_width - Dst_img_width_comp * 2,                                \
                                         Dst_img_height - Dst_img_height_comp * 2));
                Imgs[uint(i * cut_width + j)].copyTo(imageROI);
            }
        }
    }

    for (int i = 0; i < cut_width; i++)
    {
        if (i < cut_width - 1)
        {
            imageROI = srcImage(Rect(i*(Dst_img_width - Dst_img_width_comp * 2) + Dst_img_width_comp,           \
                                     height - (Dst_img_height - Dst_img_height_comp * 2) - Dst_img_height_comp, \
                                     Dst_img_width - Dst_img_width_comp * 2,                                    \
                                     Dst_img_height - Dst_img_height_comp * 2));
            Imgs[uint((cut_height - 1)*cut_width + i)].copyTo(imageROI);
        }
        else   //右下角
        {
            imageROI = srcImage(Rect(width - (Dst_img_width - Dst_img_width_comp * 2) - Dst_img_width_comp,     \
                                     height - (Dst_img_height - Dst_img_height_comp * 2) - Dst_img_height_comp, \
                                     Dst_img_width - Dst_img_width_comp * 2,                                    \
                                     Dst_img_height - Dst_img_height_comp * 2));
            Imgs[uint((cut_height - 1)*cut_width + i)].copyTo(imageROI);
        }
    }

    return srcImage;
}

//==============================================================================================================
//数字量转变成字符串量子函数
//==============================================================================================================
string Points_Processing::num2str(int i)
{
    stringstream ss;
    ss << i;
    return ss.str();
}
//==============================================================================================================
//遍历路径下文件
//==============================================================================================================
void Points_Processing::getFiles(const char* path, vector<string>& files)
{
    const string path0 = path;
    DIR* pDir;
    struct dirent* ptr;

    struct stat s;
    lstat(path, &s);

    if(!S_ISDIR(s.st_mode))
    {
        cout << "not a valid directory: " << path << endl;
        return;
    }

    if(!(pDir = opendir(path))){
        cout << "opendir error: " << path << endl;
        return;
    }
    int i = 0;
    string subFile;
    while((ptr = readdir(pDir)) != 0)
    {
        subFile = ptr -> d_name;
        if(subFile == "." || subFile == "..")
            continue;
        subFile = path0 + subFile;
        cout << ++i << ": " << subFile << endl;
        files.push_back(subFile);
    }
    closedir(pDir);

}
//===================================================================================================================================
//函数名称：void RemoveSmallRegion(Mat &Src, Mat &Dst, int AreaLimit, int CheckMode, int NeihborMode)
//函数功能： 去除较小连通域,CheckMode: 0代表去除黑区域，1代表去除白区域; NeihborMode：0代表4邻域，1代表8邻域;
//===================================================================================================================================
void Points_Processing::RemoveSmallRegion(Mat &Src, Mat &Dst, int AreaLimit, int CheckMode, int NeihborMode)
{
    int RemoveCount = 0;
    //新建一幅标签图像初始化为0像素点，为了记录每个像素点检验状态的标签，0代表未检查，1代表正在检查,2代表检查不合格（需要反转颜色），3代表检查合格或不需检查
    //初始化的图像全部为0，未检查
    Mat PointLabel = Mat::zeros(Src.size(), CV_8UC1);
    if (CheckMode == 1)//去除小连通区域的白色点
    {
        //cout << "去除小连通域.";
//#pragma omp parallel for
        for (int i = 0; i < Src.rows; i++)
        {
            for (int j = 0; j < Src.cols; j++)
            {
                if (Src.at<uchar>(i, j) < 10)
                {
                    PointLabel.at<uchar>(i, j) = 3;//将背景黑色点标记为合格，像素为3
                }
            }
        }
    }
    else//去除孔洞，黑色点像素
    {
        //cout << "去除孔洞";
//#pragma omp parallel for
        for (int i = 0; i < Src.rows; i++)
        {
            for (int j = 0; j < Src.cols; j++)
            {
                if (Src.at<uchar>(i, j) > 10)
                {
                    PointLabel.at<uchar>(i, j) = 3;//如果原图是白色区域，标记为合格，像素为3
                }
            }
        }
    }


    vector<Point2i>NeihborPos;//将邻域压进容器
    NeihborPos.push_back(Point2i(-1, 0));
    NeihborPos.push_back(Point2i(1, 0));
    NeihborPos.push_back(Point2i(0, -1));
    NeihborPos.push_back(Point2i(0, 1));
    if (NeihborMode == 1)
    {
        //cout << "Neighbor mode: 8邻域." << endl;
        NeihborPos.push_back(Point2i(-1, -1));
        NeihborPos.push_back(Point2i(-1, 1));
        NeihborPos.push_back(Point2i(1, -1));
        NeihborPos.push_back(Point2i(1, 1));
    }
   // else cout << "Neighbor mode: 4邻域." << endl;
    int NeihborCount = 4 + 4 * NeihborMode;
    int CurrX = 0, CurrY = 0;
    //开始检测
    for (int i = 0; i < Src.rows; i++)
    {
        for (int j = 0; j < Src.cols; j++)
        {
            if (PointLabel.at<uchar>(i, j) == 0)//标签图像像素点为0，表示还未检查的不合格点
            { //开始检查
                vector<Point2i>GrowBuffer;//记录检查像素点的个数
                GrowBuffer.push_back(Point2i(j, i));
                PointLabel.at<uchar>(i, j) = 1;//标记为正在检查
                int CheckResult = 0;


                for (int z = 0; z < GrowBuffer.size(); z++)
                {
                    for (int q = 0; q < NeihborCount; q++)
                    {
                        CurrX = GrowBuffer.at(z).x + NeihborPos.at(q).x;
                        CurrY = GrowBuffer.at(z).y + NeihborPos.at(q).y;
                        if (CurrX >= 0 && CurrX < Src.cols&&CurrY >= 0 && CurrY < Src.rows) //防止越界
                        {
                            if (PointLabel.at<uchar>(CurrY, CurrX) == 0)
                            {
                                GrowBuffer.push_back(Point2i(CurrX, CurrY)); //邻域点加入buffer
                                PointLabel.at<uchar>(CurrY, CurrX) = 1;   //更新邻域点的检查标签，避免重复检查
                            }
                        }
                    }
                }
                if (GrowBuffer.size() > AreaLimit) //判断结果（是否超出限定的大小），1为未超出，2为超出
                    CheckResult = 2;
                else
                {
                    CheckResult = 1;
                    RemoveCount++;//记录有多少区域被去除
                }


                for (int z = 0; z < GrowBuffer.size(); z++)
                {
                    CurrX = GrowBuffer.at(z).x;
                    CurrY = GrowBuffer.at(z).y;
                    PointLabel.at<uchar>(CurrY, CurrX) += CheckResult;//标记不合格的像素点，像素值为2
                }
                //********结束该点处的检查**********


            }
        }


    }


    CheckMode = 255 * (1 - CheckMode);
    //开始反转面积过小的区域
    for (int i = 0; i < Src.rows; ++i)
    {
        for (int j = 0; j < Src.cols; ++j)
        {
            if (PointLabel.at<uchar>(i, j) == 2)
            {
                Dst.at<uchar>(i, j) = CheckMode;
            }
            else if (PointLabel.at<uchar>(i, j) == 3)
            {
                Dst.at<uchar>(i, j) = Src.at<uchar>(i, j);

            }
        }
    }
    //cout << RemoveCount << " objects removed." << endl;
}

//===================================================================================================================================
//函数名称：void ImgThin(cv::Mat& src, cv::Mat& dst, int intera)
//函数功能：图像细化,取单像素线宽
//===================================================================================================================================
void Points_Processing::ImgThin(cv::Mat& src, cv::Mat& dst, int intera)
{
    if (src.type() != CV_8UC1)
    {
        printf("只能处理二值或灰度图像\n");
        return;
    }
    //非原地操作时候，copy src到dst
    if (dst.data != src.data)
    {
        src.copyTo(dst);
    }

    int i, j, n;
    int width, height;
    width = src.cols - 1;
    //之所以减1，是方便处理8邻域，防止越界
    height = src.rows - 1;
    int step = src.step;
    int  p2, p3, p4, p5, p6, p7, p8, p9;
    uchar* img;
    bool ifEnd;
    int A1;
    cv::Mat tmpimg;
    //n表示迭代次数
    for (n = 0; n < intera; n++)
    {
        dst.copyTo(tmpimg);
        ifEnd = false;
        img = tmpimg.data;
        for (i = 1; i < height; i++)
        {
            img += step;
            for (j = 1; j < width; j++)
            {
                uchar* p = img + j;
                A1 = 0;
                if (p[0] > 0)
                {
                    if (p[-step] == 0 && p[-step + 1] > 0) //p2,p3 01模式
                    {
                        A1++;
                    }
                    if (p[-step + 1] == 0 && p[1] > 0) //p3,p4 01模式
                    {
                        A1++;
                    }
                    if (p[1] == 0 && p[step + 1] > 0) //p4,p5 01模式
                    {
                        A1++;
                    }
                    if (p[step + 1] == 0 && p[step] > 0) //p5,p6 01模式
                    {
                        A1++;
                    }
                    if (p[step] == 0 && p[step - 1] > 0) //p6,p7 01模式
                    {
                        A1++;
                    }
                    if (p[step - 1] == 0 && p[-1] > 0) //p7,p8 01模式
                    {
                        A1++;
                    }
                    if (p[-1] == 0 && p[-step - 1] > 0) //p8,p9 01模式
                    {
                        A1++;
                    }
                    if (p[-step - 1] == 0 && p[-step] > 0) //p9,p2 01模式
                    {
                        A1++;
                    }
                    p2 = p[-step] > 0 ? 1 : 0;
                    p3 = p[-step + 1] > 0 ? 1 : 0;
                    p4 = p[1] > 0 ? 1 : 0;
                    p5 = p[step + 1] > 0 ? 1 : 0;
                    p6 = p[step] > 0 ? 1 : 0;
                    p7 = p[step - 1] > 0 ? 1 : 0;
                    p8 = p[-1] > 0 ? 1 : 0;
                    p9 = p[-step - 1] > 0 ? 1 : 0;
                    if ((p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9) > 1 && (p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9) < 7 && A1 == 1)
                    {
                        if ((p2 == 0 || p4 == 0 || p6 == 0) && (p4 == 0 || p6 == 0 || p8 == 0)) //p2*p4*p6=0 && p4*p6*p8==0
                        {
                            dst.at<uchar>(i, j) = 0; //满足删除条件，设置当前像素为0
                            ifEnd = true;
                        }
                    }
                }
            }
        }

        dst.copyTo(tmpimg);
        img = tmpimg.data;
        for (i = 1; i < height; i++)
        {
            img += step;
            for (j = 1; j < width; j++)
            {
                A1 = 0;
                uchar* p = img + j;
                if (p[0] > 0)
                {
                    if (p[-step] == 0 && p[-step + 1] > 0) //p2,p3 01模式
                    {
                        A1++;
                    }
                    if (p[-step + 1] == 0 && p[1] > 0) //p3,p4 01模式
                    {
                        A1++;
                    }
                    if (p[1] == 0 && p[step + 1] > 0) //p4,p5 01模式
                    {
                        A1++;
                    }
                    if (p[step + 1] == 0 && p[step] > 0) //p5,p6 01模式
                    {
                        A1++;
                    }
                    if (p[step] == 0 && p[step - 1] > 0) //p6,p7 01模式
                    {
                        A1++;
                    }
                    if (p[step - 1] == 0 && p[-1] > 0) //p7,p8 01模式
                    {
                        A1++;
                    }
                    if (p[-1] == 0 && p[-step - 1] > 0) //p8,p9 01模式
                    {
                        A1++;
                    }
                    if (p[-step - 1] == 0 && p[-step] > 0) //p9,p2 01模式
                    {
                        A1++;
                    }
                    p2 = p[-step] > 0 ? 1 : 0;
                    p3 = p[-step + 1] > 0 ? 1 : 0;
                    p4 = p[1] > 0 ? 1 : 0;
                    p5 = p[step + 1] > 0 ? 1 : 0;
                    p6 = p[step] > 0 ? 1 : 0;
                    p7 = p[step - 1] > 0 ? 1 : 0;
                    p8 = p[-1] > 0 ? 1 : 0;
                    p9 = p[-step - 1] > 0 ? 1 : 0;
                    if ((p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9) > 1 && (p2 + p3 + p4 + p5 + p6 + p7 + p8 + p9) < 7 && A1 == 1)
                    {
                        if ((p2 == 0 || p4 == 0 || p8 == 0) && (p2 == 0 || p6 == 0 || p8 == 0)) //p2*p4*p8=0 && p2*p6*p8==0
                        {
                            dst.at<uchar>(i, j) = 0; //满足删除条件，设置当前像素为0
                            ifEnd = true;
                        }
                    }
                }
            }
        }

        //如果两个子迭代已经没有可以细化的像素了，则退出迭代
        if (!ifEnd) break;
    }
}

/*************************************************
//  Method:    convertTo3Channels
//  Description: 将单通道图像转为三通道图像
//  Returns:   cv::Mat
//  Parameter: binImg 单通道图像对象
*************************************************/
Mat Points_Processing::convertTo3Channels(const Mat binImg)
{
    Mat three_channel = Mat::zeros(binImg.rows,binImg.cols,CV_8UC3);
    vector<Mat> channels;
    for (int i=0;i<3;i++)
    {
        channels.push_back(binImg);
    }
    merge(channels,three_channel);
    return three_channel;
}
