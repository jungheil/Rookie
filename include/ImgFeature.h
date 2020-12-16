//
// Created by li on 12/12/2020.
//

#ifndef ROOKIE_IMGFEATURE_H
#define ROOKIE_IMGFEATURE_H
#include <vector>
#include "opencv2/opencv.hpp"


class ClcHog{
public:
    ClcHog();
    ~ClcHog();
    std::vector<float> Get(const cv::Mat& img);

private:
    cv::HOGDescriptor *hog_= nullptr;
    cv::Size img_size_ = cv::Size(128,256);
};

class HOGSimilarity{
public:
    HOGSimilarity() = default;
    void Init(cv::Mat img);
    float GetSimilarity(cv::Mat img);
    void Update(const cv::Mat& img);

private:
    void Normalize(std::vector<float> &feature);
    void Crop(const cv::Mat& img, std::vector<cv::Mat> &imgs);
    void Rotate(const cv::Mat &srcImage, cv::Mat &destImage, double angle);

private:
    ClcHog hog_;

private:
    std::vector<float> filter_;
    float learning_rate = 0.25;
    cv::Size img_size_ = cv::Size(128,256);
};


class HSFeature{
public:
    HSFeature() = default;
    void Init(const cv::Mat &img);
    float GetSimilarity(const cv::Mat &img);
    void Update(const cv::Mat &img);

//private:
public:
    static void CalcHist(const cv::Mat &img, cv::Mat &hist);
    static void GetFeature(const cv::Mat &img, std::vector<cv::Mat> &hists);

private:
    std::vector<cv::Mat> hists_;
    cv::Size img_size_ = cv::Size(128,256);
    float learning_rate = 0.25;
};

class CLFeature{
public:
    CLFeature() = default;
    void Init(const cv::Mat &img);
    float GetSimilarity(const cv::Mat &img);
    void Update(const cv::Mat &img);

//private:
private:
    static void CalcHist(const cv::Mat &img, cv::Mat &hist);
    void Crop(const cv::Mat& img, std::vector<cv::Mat> &imgs);
    void Rotate(const cv::Mat &srcImage, cv::Mat &destImage, double angle);


private:
    std::vector<cv::Mat> hists_;
    cv::Size img_size_ = cv::Size(16,32);
    float learning_rate = 0.25;
    std::vector<double> filter_a_;
    std::vector<double> filter_b_;
};


class HOGLoss{
public:
    HOGLoss() = default;
    void Init(cv::Mat img);
    double GetLoss(cv::Mat img);
    void Update(const cv::Mat& img);

private:
    void Normalize(std::vector<float> &feature);
    void Crop(const cv::Mat& img, std::vector<cv::Mat> &imgs);
    void Rotate(const cv::Mat &srcImage, cv::Mat &destImage, double angle);

private:
    ClcHog hog_;

private:
    std::vector<double> filter_;
    std::vector<double> fa_;
    std::vector<double> fb_;

    double learning_rate = 0.25;
    cv::Size img_size_ = cv::Size(128,256);
};



#endif //ROOKIE_IMGFEATURE_H
