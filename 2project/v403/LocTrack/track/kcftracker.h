#pragma once
#include "tracker.h"  

#ifndef _OPENCV_KCFTRACKER_HPP_  
#define _OPENCV_KCFTRACKER_HPP_  
#endif  

class KCFTracker : public Tracker
{
public:
	// Constructor  
	// 构造KCF跟踪器的类  
	KCFTracker(bool hog = true,                 // 使用hog特征  
		bool fixed_window = true,       //使用固定窗口大小  
		bool multiscale = true,         //使用多尺度  
		bool lab = true);               //使用lab色空间特征  

										// Initialize tracker  
										// 初始化跟踪器， roi 是目标初始框的引用， image 是进入跟踪的第一帧图像  
	virtual void init(const cv::Rect &roi, cv::Mat image);

	// Update position based on the new frame  
	// 使用新一帧更新图像， image 是新一帧图像  
	virtual cv::Rect update(cv::Mat image,cv::Rect flashRoi);

	float interp_factor;        // linear interpolation factor for adaptation  
								// 自适应的线性插值因子，会因为hog，lab的选择而变化  
	float sigma;                // gaussian kernel bandwidth  
								// 高斯卷积核带宽，会因为hog，lab的选择而变化  
	float lambda;               // regularization  
								// 正则化，0.0001  
	int cell_size;              // HOG cell size  
								// HOG元胞数组尺寸，4  
	int cell_sizeQ;             // cell size^2, to avoid repeated operations  
								// 元胞数组内像素数目，16，为了计算省事  
	float padding;              // extra area surrounding the target  
								// 目标扩展出来的区域，2.5  
	float output_sigma_factor;  // bandwidth of gaussian target  
								// 高斯目标的带宽，不同hog，lab会不同  
	int template_size;          // template size  
								// 模板大小，在计算_tmpl_sz时，  
								// 较大变成被归一成96，而较小边长按比例缩小  
	float scale_step;           // scale step for multi-scale estimation  
								// 多尺度估计的时候的尺度步长  
	float scale_weight;         // to downweight detection scores of other scales for added stability  
								// 为了增加其他尺度检测时的稳定性，给检测结果峰值做一定衰减，为原来的0.95倍  

protected:
	// Detect object in the current frame.  
	// 检测当前帧的目标  
	//z是前一阵的训练/第一帧的初始化结果， x是当前帧当前尺度下的特征， peak_value是检测结果峰值  
	cv::Point2f detect(cv::Mat z, cv::Mat x, float &peak_value);

	// train tracker with a single image  
	// 使用当前图像的检测结果进行训练  
	// x是当前帧当前尺度下的特征， train_interp_factor是interp_factor  
	void train(cv::Mat x, float train_interp_factor);

	// Evaluates a Gaussian kernel with bandwidth SIGMA for all relative shifts between input images X and Y,  
	// which must both be MxN. They must also be periodic (ie., pre-processed with a cosine window).  
	// 使用带宽SIGMA计算高斯卷积核以用于所有图像X和Y之间的相对位移  
	// 必须都是MxN大小。二者必须都是周期的（即，通过一个cos窗口进行预处理）  
	cv::Mat gaussianCorrelation(cv::Mat x1, cv::Mat x2);

	// Create Gaussian Peak. Function called only in the first frame.  
	// 创建高斯峰函数，函数只在第一帧的时候执行  
	cv::Mat createGaussianPeak(int sizey, int sizex);

	// Obtain sub-window from image, with replication-padding and extract features  
	// 从图像得到子窗口，通过赋值填充并检测特征  
	cv::Mat getFeatures(const cv::Mat & image, bool inithann, float scale_adjust = 1.0f);

	// Initialize Hanning window. Function called only in the first frame.  
	// 初始化hanning窗口。函数只在第一帧被执行。  
	void createHanningMats();

	// Calculate sub-pixel peak for one dimension  
	// 计算一维亚像素峰值  
	float subPixelPeak(float left, float center, float right);

	cv::Mat _alphaf;            // 初始化/训练结果alphaf，用于检测部分中结果的计算  
	cv::Mat _prob;              // 初始化结果prob，不再更改，用于训练  
	cv::Mat _tmpl;              // 初始化/训练的结果，用于detect的z  
	cv::Mat _num;               // 貌似都被注释掉了  
	cv::Mat _den;               // 貌似都被注释掉了  
	cv::Mat _labCentroids;      // lab质心数组  

private:
	int size_patch[3];          // hog特征的sizeY，sizeX，numFeatures  
	cv::Mat hann;               // createHanningMats()的计算结果  
	cv::Size _tmpl_sz;          // hog元胞对应的数组大小  
	float _scale;               // 修正成_tmpl_sz后的尺度大小  
	int _gaussian_size;         // 未引用？？？  
	bool _hogfeatures;          // hog标志位  
	bool _labfeatures;          // lab标志位  
};
