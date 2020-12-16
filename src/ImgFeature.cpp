//
// Created by li on 11/12/2020.
//

#include <numeric>
#include "ImgFeature.h"

using namespace std;
using namespace cv;

ClcHog::ClcHog() {
    hog_ = new cv::HOGDescriptor(img_size_,
                                 cv::Size(16, 16),
                                 cv::Size(8, 8),
                                 cv::Size(8, 8), 9);//创建HOG描述器

}

ClcHog::~ClcHog() {
    delete hog_;
}

std::vector<float> ClcHog::Get(const cv::Mat &img) {
    cv::Mat data;
    std::vector<float> out;
    cv::cvtColor(img,data,cv::COLOR_BGR2GRAY);
    resize(data,data,img_size_, CV_8UC1);//调整样本大小

    hog_->compute(data, out, cv::Size(1, 1),cv::Size(0, 0)); //Hog特征计算
    return out;
}


void HOGSimilarity::Normalize(std::vector<float> &feature) {
    long double sum=0;
    long double mean=0;
    for(const auto &s: feature){
        sum += s;
    }
    mean = sum / feature.size();
    for(auto &s: feature){
        s = s - mean;
    }
}

void HOGSimilarity::Crop(const cv::Mat& img, std::vector<cv::Mat> &imgs) {
    Mat src;
    resize(img,src,img_size_);
    Mat temp;
    for(int i = 1; i<=8; i++){
        for(int j = 1; j<=8; j++){
            temp = src(Rect(i,j,img_size_.width-2*i,img_size_.height-2*j));
            resize(temp,temp,img_size_);
            imgs.push_back(temp);
        }
    }

    copyMakeBorder( src, src, 8, 8, 8, 8, BORDER_REPLICATE);
    for(int i = 1; i<=16; i++){
        float angle = i * .2f;
        Rotate(src,temp,angle);
        temp = temp(Rect(8,8,img_size_.width,img_size_.height));
//        resize(temp,temp,img_size_);
        imgs.push_back(temp);
        Rotate(src,temp,-angle);
        temp = temp(Rect(8,8,img_size_.width,img_size_.height));
//        resize(temp,temp,img_size_);
        imgs.push_back(temp);
    }
}

void HOGSimilarity::Rotate(const Mat &srcImage, Mat &destImage, double angle) {
    Point2f center(srcImage.cols / 2, srcImage.rows / 2);//中心
    Mat M = getRotationMatrix2D(center, angle, 1);//计算旋转的仿射变换矩阵
    warpAffine(srcImage, destImage, M, Size(srcImage.cols, srcImage.rows));//仿射变换
}

void HOGSimilarity::Init(cv::Mat img) {
    vector<Mat> imgs;
    vector<vector<float>> features;
    vector<float> feature_temp;

    Crop(img,imgs);
    filter_ = hog_.Get(imgs[0]);
    Normalize(filter_);

    for(auto it = imgs.begin()+1;it!=imgs.end();it++){
        feature_temp = hog_.Get(*it);
        Normalize(feature_temp);
        auto nft = feature_temp.begin();
        for(auto &s:filter_){
            s += *nft;
            nft++;
        }
    }
    for(auto &s:filter_){
        s /= imgs.size();
    }


}

float HOGSimilarity::GetSimilarity(cv::Mat img) {
    vector<float> feature;
    float out = 0;
    Mat src;
    feature = hog_.Get(img);
    Normalize(feature);

//    auto fit = filter_.begin();
//    for(auto &s:feature){
//        s *= (*fit);
//        fit++;
//        out += s;
//    }

    auto it = feature.begin();
    for(const auto &s:filter_){
        (*it) *= s;
        out += (*it);
        it++;
    }

    return out;
}

void HOGSimilarity::Update(const cv::Mat& img) {
    vector<float> feature;
    feature = hog_.Get(img);
    Normalize(feature);

    auto fit = feature.begin();
    for(auto &s:filter_){
        s = learning_rate * (*fit) + (1 - learning_rate) * (s);
        fit++;
    }
}

void HSFeature::CalcHist(const Mat &img, Mat &hist) {
//    Mat hsv;
//    cvtColor( img, hsv, COLOR_BGR2HSV );
//    int h_bins = 60; int s_bins = 64;
//    int histSize[] = { h_bins, s_bins };
//
//    // hue的取值范围从0到256, saturation取值范围从0到180
//    float h_ranges[] = { 0, 180 };
//    float s_ranges[] = { 0, 256 };
//
//    const float* ranges[] = { h_ranges, s_ranges };
//
//    // 使用第0和第1通道
//    int channels[] = { 0, 1 };
//    calcHist( &hsv, 1, channels, Mat(), hist, 2, histSize, ranges);
//    normalize( hist, hist, 0, 1, NORM_MINMAX);

    int histSize[] = { 64, 64, 64 };

    // hue的取值范围从0到256, saturation取值范围从0到180
    float range[] = { 0, 256 };

    const float* ranges[] = { range, range, range };

    // 使用第0和第1通道
    int channels[] = { 0, 1 ,2 };
    calcHist( &img, 1, channels, Mat(), hist, 3, histSize, ranges);
    normalize( hist, hist, 0, 1, NORM_MINMAX);
}

void HSFeature::GetFeature(const Mat &img, vector<cv::Mat> &hists) {
    int dim = 4;
    for(int i = 0; i<dim; i++){
        Mat hist;
        CalcHist(img(Rect(0,i*img.rows/dim,img.cols,img.rows/dim)), hist);
        if (i == 0 || i == dim-1) continue;
        hists.push_back(hist);
    }
}

void HSFeature::Init(const Mat &img) {
    Mat src;
//    resize(img,src, img_size_);
    GetFeature(img,hists_);
}

float HSFeature::GetSimilarity(const Mat &img) {
    Mat src;
    vector<Mat> hists;
    float out = 0;
//    resize(img,src, img_size_);
    GetFeature(img, hists);

//    auto it = hists_.begin();
//    for(const auto &s:hists){
//        out += compareHist(s,*it,0);
//        it++;
//    }

    auto it = hists.begin();
    for(const auto &s:hists_){
        out += compareHist(s,*it,0);
        it++;
    }
    return out;
}

void HSFeature::Update(const Mat &img) {
    Mat src,a;
    vector<Mat> hists;
    float out = 0;
//    resize(img,src, img_size_);
    GetFeature(img, hists);

    auto it = hists_.begin();
    for(const auto &s:hists){
        *it = learning_rate*s+(1-learning_rate)*(*it);
    }
}

void CLFeature::CalcHist(const Mat &img, Mat &hist) {
    Mat lab;
    cvtColor( img, lab, COLOR_BGR2Lab );

    int histSize[] = { 64, 64 };

    // hue的取值范围从0到256, saturation取值范围从0到180
    float range[] = { 0, 256 };

    const float* ranges[] = { range, range };

    // 使用第0和第1通道
    int channels[] = { 1, 2 };
    calcHist( &lab, 1, channels, Mat(), hist, 2, histSize, ranges);
    normalize( hist, hist, 0, 1, NORM_MINMAX);
}

void CLFeature::Init(const Mat &img) {
    Mat lab;
    vector<Mat> imgs;
    vector<double> filter_a;
    vector<double> filter_b;
    resize(img,lab,img_size_);
    cvtColor(lab, lab, COLOR_BGR2Lab);

    for(MatIterator_<Vec3b> it = lab.begin<Vec3b>(); it < lab.end<Vec3b>(); it++){
        filter_a.push_back((*it)[1]-128);
        filter_b.push_back((*it)[2]-128);
    }

    Crop(lab,imgs);
    for(auto &s:imgs){
        int i = 0;
        for(MatIterator_<Vec3b> it = s.begin<Vec3b>(); it < s.end<Vec3b>(); it++){
            filter_a[i] += ((*it)[1]-128);
            filter_b[i] += ((*it)[2]-128);
            i++;
        }
    }

    for(auto &s:filter_a){
        s /= imgs.size();
    }
    for(auto &s:filter_b){
        s /= imgs.size();
    }
    double s2a;
    double s2b;
    for(int i = 0; i<filter_a.size(); i++){
        s2a += pow(filter_a[i],2);
        s2b += pow(filter_b[i], 2);
    }
    for(auto &s:filter_a){
        s = s / (s2a/filter_a.size());
    }
    for(auto &s:filter_b){
        s = s / (s2b/filter_b.size());
    }

    filter_a_ = filter_a;
    filter_b_ = filter_b;
}

void CLFeature::Crop(const cv::Mat& img, std::vector<cv::Mat> &imgs) {
    Mat src;
    resize(img,src,img_size_);
    Mat temp;
    for(int i = 1; i<=4; i++){
        for(int j = 1; j<=4; j++){
            temp = src(Rect(i,j,img_size_.width-2*i,img_size_.height-2*j));
            resize(temp,temp,img_size_);
            imgs.push_back(temp);
        }
    }

    copyMakeBorder( src, src, 8, 8, 8, 8, BORDER_REPLICATE);
    for(int i = 1; i<=16; i++){
        float angle = i * .1f;
        Rotate(src,temp,angle);
        temp = temp(Rect(8,8,img_size_.width,img_size_.height));
//        resize(temp,temp,img_size_);
        imgs.push_back(temp);
        Rotate(src,temp,-angle);
        temp = temp(Rect(8,8,img_size_.width,img_size_.height));
//        resize(temp,temp,img_size_);
        imgs.push_back(temp);
    }
}
void CLFeature::Rotate(const Mat &srcImage, Mat &destImage, double angle) {
    Point2f center(srcImage.cols / 2, srcImage.rows / 2);//中心
    Mat M = getRotationMatrix2D(center, angle, 1);//计算旋转的仿射变换矩阵
    warpAffine(srcImage, destImage, M, Size(srcImage.cols, srcImage.rows));//仿射变换
}

float CLFeature::GetSimilarity(const Mat &img) {
    vector<double> filter_a;
    vector<double> filter_b;
    float out = 0;
    Mat lab;

    resize(img,lab,img_size_);
    cvtColor(lab, lab, COLOR_BGR2Lab);

    for(MatIterator_<Vec3b> it = lab.begin<Vec3b>(); it < lab.end<Vec3b>(); it++){
        filter_a.push_back((*it)[1]-128);
        filter_b.push_back((*it)[2]-128);
    }

    double s2a;
    double s2b;
    for(int i = 0; i<filter_a.size(); i++){
        s2a += pow(filter_a[i],2);
        s2b += pow(filter_b[i], 2);
    }
    for(auto &s:filter_a){
        s = s / (s2a/filter_a.size());
    }
    for(auto &s:filter_b){
        s = s / (s2b/filter_b.size());
    }

    for(int i = 0; i<filter_a_.size();i++){
//        if(abs(filter_a[i]*filter_a_[i])<100&&abs(filter_b[i]*filter_b_[i])<100) continue;
        out += filter_a[i]*filter_a_[i]*filter_b[i]*filter_b_[i];
    }

    return out;
}

void CLFeature::Update(const Mat &img) {
    Mat lab;
    vector<double> filter_a;
    vector<double> filter_b;

    resize(img,lab,img_size_);
    cvtColor(lab, lab, COLOR_BGR2Lab);

    for(MatIterator_<Vec3b> it = lab.begin<Vec3b>(); it < lab.end<Vec3b>(); it++){
        filter_a.push_back((*it)[1]-128);
        filter_b.push_back((*it)[2]-128);
    }

    double s2a;
    double s2b;
    for(int i = 0; i<filter_a.size(); i++){
        s2a += pow(filter_a[i],2);
        s2b += pow(filter_b[i], 2);
    }
    for(auto &s:filter_a){
        s = s / (s2a/filter_a.size());
    }
    for(auto &s:filter_b){
        s = s / (s2b/filter_b.size());
    }

    int i = 0;
    for(MatIterator_<Vec3b> it = lab.begin<Vec3b>(); it < lab.end<Vec3b>(); it++){
//        filter_a_[i] = learning_rate*((*it)[1]-128) + (1 - learning_rate)*filter_a_[i];
//        filter_b_[i] = learning_rate*((*it)[2]-128) + (1 - learning_rate)*filter_b_[i];

        filter_a_[i] = learning_rate*(filter_a[i]) + (1 - learning_rate)*filter_a_[i];
        filter_b_[i] = learning_rate*(filter_b[i]) + (1 - learning_rate)*filter_b_[i];
        i++;
    }
}

