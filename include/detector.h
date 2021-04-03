//
// Created by li on 2020/6/12.
//

#ifndef XIROBO_DETECTOR_H
#define XIROBO_DETECTOR_H
#include <opencv2/opencv.hpp>
#include "torch/script.h"

#include "common.h"

namespace XRDetector{
    class ML{
    public:
        ML() = default;
        virtual void Predictor(std::vector<cv::Rect> &boxes, cv::Mat src) = 0;
    protected:
        float confThreshold_ = 0.5; // Confidence threshold
        float nmsThreshold_ = 0.5;  // Non-maximum suppression threshold
        int inpWidth_ = 640;  // Width of network's input image
        int inpHeight_ = 384; // Height of network's input image

        cv::Size src_size_;
    };

    class MLCV:ML{
    public:
        MLCV();
        void Predictor(std::vector<cv::Rect> &boxes, cv::Mat src) override;

    private:
        std::vector<cv::String> GetOutputsNames(const cv::dnn::Net& net);
        void Process(const std::vector<cv::Mat>& outs);

    private:
        cv::String modelConfiguration_ = "../ml/yolov3.cfg";
        cv::String modelWeights_ = "../ml/yolov3.weights";

        std::vector<std::string> classes_;
        cv::dnn::Net net_;
        cv::Mat blob_;

        std::vector<float> confidences_;
        std::vector<cv::Rect> boxes_;
    };

    class MLTorch:ML{
    public:
        MLTorch();
        void Predictor(std::vector<cv::Rect> &boxes, cv::Mat src) override;

    private:
        std::vector<torch::Tensor> NonMaxSuppression(torch::Tensor preds);

    private:
        std::string modelWeights_ = "../ml/yolov5s.torchscript.pt";
        torch::jit::script::Module module_;
    };

    class Detector {
    public:
        Detector();
        void UpdatePerson(Ximg img, std::vector<Person> &person);
        void GetMask(Ximg img, cv::Rect &box, cv::Mat& out, float &out_dis);

    private:
        MLTorch ml;
//        MLCV ml;
        cv::Mat R_b_cam;  //相机坐标系在基坐标系下表达
    };
}


#endif //XIROBO_DETECTOR_H
