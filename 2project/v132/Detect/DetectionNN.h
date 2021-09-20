#ifndef DETECTIONNN_H
#define DETECTIONNN_H

#include <iostream>
#include <signal.h>
#include <stdlib.h>    
#include <unistd.h>
#include <mutex>
#include "utils.h"
#include <algorithm>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include "common.h"
#include "pipeline/pipelinectl.h"

#include "tkdnn.h"

// #define OPENCV_CUDACONTRIB //if OPENCV has been compiled with CUDA and contrib.

#ifdef OPENCV_CUDACONTRIB
#include <opencv2/cudawarping.hpp>
#include <opencv2/cudaarithm.hpp>
#endif


namespace tk { namespace dnn {

class DetectionNN {

    protected:
        tk::dnn::NetworkRT *netRT = nullptr;
        dnnType *input_d;

        std::vector<cv::Size> originalSize;

        cv::Scalar colors[256];

        int nBatches = 1;

#ifdef OPENCV_CUDACONTRIB
        cv::cuda::GpuMat bgr[3];
        cv::cuda::GpuMat imagePreproc;
#else
        cv::Mat bgr[3];
        cv::Mat imagePreproc;
        dnnType *input;
#endif
        /**
         * This method preprocess the image, before feeding it to the NN.
         *
         * @param frame original frame to adapt for inference.
         * @param bi batch index
         */
        virtual void preprocess(cv::Mat &frame, const int bi=0) = 0;

        /**
         * This method postprocess the output of the NN to obtain the correct 
         * boundig boxes. 
         * 
         * @param bi batch index
         * @param mAP set to true only if all the probabilities for a bounding 
         *            box are needed, as in some cases for the mAP calculation
         */
        virtual void postprocess(const int bi=0,const bool mAP=false) = 0;

    public:
        int classes = 0;
        float confThreshold = 0.7; /*threshold on the confidence of the boxes*/

        std::vector<tk::dnn::box> detected; /*bounding boxes in output*/
        std::vector<std::vector<tk::dnn::box>> batchDetected; /*bounding boxes in output*/
        std::vector<double> stats; /*keeps track of inference times (ms)*/
        std::vector<std::string> classesNames;
        DetectionNN() {};
        ~DetectionNN(){};

        /**
         * Method used to inialize the class, allocate memory and compute 
         * needed data.
         * 
         * @param tensor_path path to the rt file og the NN.
         * @param n_classes number of classes for the given dataset.
         * @param n_batches maximum number of batches to use in inference
         * @return true if everything is correct, false otherwise.
         */
        virtual bool init(const std::string& tensor_path, const int n_classes=80, const int n_batches=1, const float conf_thresh=0.7) = 0;
        
        /**
         * This method performs the whole detection of the NN.
         * 
         * @param frames frames to run detection on.
         * @param cur_batches number of batches to use in inference
         * @param save_times if set to true, preprocess, inference and postprocess times 
         *        are saved on a csv file, otherwise not.
         * @param times pointer to the output stream where to write times
         * @param mAP set to true only if all the probabilities for a bounding 
         *            box are needed, as in some cases for the mAP calculation
         */
        void update(std::vector<cv::Mat>& frames, const int cur_batches=1, bool save_times=false, std::ofstream *times=nullptr, const bool mAP=false){
            if(save_times && times==nullptr)
                FatalError("save_times set to true, but no valid ofstream given");
            if(cur_batches > nBatches)
                FatalError("A batch size greater than nBatches cannot be used");

            originalSize.clear();
            if(TKDNN_VERBOSE) printCenteredTitle(" TENSORRT detection ", '=', 30); 
            {
                TKDNN_TSTART
                for(int bi=0; bi<cur_batches;++bi){
                    if(!frames[bi].data)
                        FatalError("No image data feed to detection");
                    originalSize.push_back(frames[bi].size());
                    preprocess(frames[bi], bi);    
                }
                TKDNN_TSTOP
                if(save_times) *times<<t_ns<<";";
            }

            //do inference
            tk::dnn::dataDim_t dim = netRT->input_dim;
            dim.n = cur_batches;
            {
                if(TKDNN_VERBOSE) dim.print();
                TKDNN_TSTART
                netRT->infer(dim, input_d);
                TKDNN_TSTOP
                if(TKDNN_VERBOSE) dim.print();
                stats.push_back(t_ns);
                if(save_times) *times<<t_ns<<";";
            }

            batchDetected.clear();
            {
                TKDNN_TSTART
                for(int bi=0; bi<cur_batches;++bi)
                    postprocess(bi, mAP);
                TKDNN_TSTOP
                if(save_times) *times<<t_ns<<"\n";
            }
        }      

        /**
         * Method to draw boundixg boxes and labels on a frame.
         * 
         * @param frames orginal frame to draw bounding box on.
         */
        std::vector<Bbox> draw(std::vector<cv::Mat>& frames, pipelineData* plDin) {
            tk::dnn::box b;
            int x0, x1, y0, y1;
            std::string det_class;
            std::vector<Bbox> returnBox;
            Bbox bbox_temp;
            plDin->SDetResult.TargetLoss=false;
            plDin->SDetResult.point.clear();
            plDin->SBox.clear();

            int baseline = 0;
            float font_scale = 0.5;
            int thickness = 2;   

            for(int bi=0; bi<frames.size(); ++bi){
                // draw dets
                float width = frames[0].cols;
                float height = frames[0].rows;
                float probselect = 0;
                bool getSurfaceFlg = false;
                for(int i=0; i<batchDetected[bi].size(); i++) { 
                    b           = batchDetected[bi][i];
                    x0   		= max(b.x + width, width) - width;
                    x1   		= min(x0 + b.w, width);
                    y0   		= max(b.y + height, height) - height;
                    y1   		= min(y0 + b.h,height);
                    det_class 	= classesNames[b.cl];
                    bbox_temp.bx0 = x0;
                    bbox_temp.by0 = y0;
                    bbox_temp.bx1 = x1;
                    bbox_temp.by1 = y1;
                    bbox_temp.bcl = b.cl;
                    bbox_temp.bw = b.w;
                    bbox_temp.bh = b.h;
                    bbox_temp.prob = b.prob;
                    returnBox.push_back(bbox_temp);
                    plDin->SBox.push_back(bbox_temp);

                    if(plDin->SDetResult.Reset)
                    {
                        plDin->track_init=false;
                    }
                    if(b.cl==0)
                    {
                        b.prob=b.prob*0.1;
                    }
                    if(b.cl==1) {
                        plDin->resetWithSurface = true;
                        getSurfaceFlg = true;
                    }
                    if(probselect<b.prob)
                    {
                        probselect=b.prob;
                        plDin->detBoxBest.bx0 = x0;
                        plDin->detBoxBest.bx1 = x1;
                        plDin->detBoxBest.by0 = y0;
                        plDin->detBoxBest.by1 = y1;
                        plDin->detBoxBest.bw = b.w;
                        plDin->detBoxBest.bh = b.h;
                        plDin->detBoxBest.bcl = b.cl;
                    }

                    // draw rectangle
                    cv::rectangle(frames[bi], cv::Point(x0, y0), cv::Point(x1, y1), colors[b.cl], 2); 

                    // draw label
                    cv::Size text_size = getTextSize(det_class, cv::FONT_HERSHEY_SIMPLEX, font_scale, thickness, &baseline);
                    cv::rectangle(frames[bi], cv::Point(x0, y0), cv::Point((x0 + text_size.width - 2), (y0 - text_size.height - 2)), colors[b.cl], -1);                      
                    cv::putText(frames[bi], det_class, cv::Point(x0, (y0 - (baseline / 2))), cv::FONT_HERSHEY_SIMPLEX, font_scale, cv::Scalar(255, 255, 255), thickness);
                }
                if(!getSurfaceFlg) {
                    plDin->resetWithSurface = false;
                }
                plDin->ShowFrame = frames[bi].clone();
            }
            return returnBox;
        }
};

}}

#endif /* DETECTIONNN_H*/
