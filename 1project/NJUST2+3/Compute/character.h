#pragma once
#include <gflags/gflags.h>
#include <glog/logging.h>
#include "boost/algorithm/string.hpp"
#include <sys/types.h>
#include <dirent.h>
#include <errno.h>
#include <vector>
#include <utility>
#include <iosfwd>
#include <string>
#include <iostream>
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/dnn.hpp>
#include <caffe/caffe.hpp>
#include <ctime>
#include <iostream>
#include <iterator>
#include <algorithm>
#include <iomanip>
#include <iosfwd>
#include <memory>
#include <string>
#include <utility>
#include <vector>
#include <stdio.h>
#include "stdio.h"
#include <cstdio>
#include <stdlib.h>
#include "Server/parameter.h"


using namespace std;
using namespace cv;
using namespace dnn;
using namespace caffe;


class character
{
    public:
     character();
      void Chara_Detec(Mat Camera_Img, vector<string> &alltext, vector<Point2f> &center_point);
      void detect_crop(Mat Camera_Img, vector<Mat> &character_srcs, vector<Point2f> &center_point);
      vector<Mat> Eastoutput(vector<Mat> Camera_Img);
      vector<Mat> East_crop(vector<Mat> Camera_Img);
      void load_east_net();
      void load_ssd();


    private:
     struct label_and_centerpoint_and_score
     {
         char label;
         Point centerpoint;
         float score;
     }lc;

     vector<label_and_centerpoint_and_score> all_lc;
     string ssd_weights_file;
     string ssd_model_file;
     string east_model_file;
     string txt_file_path;
     Size input_geometry_;
     vector<char> Alpha;
     vector<string> AllText;

     dnn::Net east_net;

     boost::shared_ptr<caffe::Net<float> > ssd_net_;
     int num_channels_;
     Mat mean_;

     Point pt1;
     Point pt2;

     Point single_character_center_point;
     vector<Point> all_single_character_center_point;

     float confidence_threshold;

//     const string& mean_file = "";
//     const string& mean_value = "104,117,123";

     int east_inpWidth;
     int east_inpHeight;
     float confThreshold;
     float nmsThreshold;

     int increment_width;
     int increment_height;


    private:
     vector<string> readtxt(string txt_file_path);
     int min(int a, int b);
     int edit(string str1, string str2);
     void compare_string(vector<string> &strr1, vector<string> &strr2, vector<Point2f> &cp );
//     void load_east_net();
//     void load_ssd();
     RotatedRect DealBox(Point2f ratio, Mat frame, RotatedRect box);
     void decode(const Mat& scores, const Mat& geometry, float scoreThresh, vector<RotatedRect>& detections, vector<float>& confidences);
     void SetMean(const string& mean_file, const string& mean_value);
     void WrapInputLayer(vector<Mat>* input_channels);
     void Preprocess(const Mat& img, vector<Mat>* input_channels);
     void initializeDetector(const string& model_file, const string& weights_file, const string& mean_file, const string& mean_value);
     vector<vector<float> > Detect(const Mat& img);
     void detectOneImg(Mat img);
     void packet_handler(u_char *param, const struct pcap_pkthdr *header, const u_char *pkt_data);
     static bool cmp(const label_and_centerpoint_and_score &lc1, const label_and_centerpoint_and_score &lc2);
     float getDistance(Point pointO, Point point1);


    public:
//     vector<Point2f> duandian_1;
//     vector<Point2f> duandian_2;
//     vector<Point2f> duandian_3;
//     vector<Point2f> duandian_4;
     vector<Point2f> center_point;
     vector<string> alltext;
};
