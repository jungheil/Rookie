#ifndef ROOKIE_NTRACKER_H
#define ROOKIE_NTRACKER_H
#pragma once
#include "Kalman.h"
#include "Hungarian.h"
#include "opencv2/opencv.hpp"
#include <iostream>
#include <vector>
#include "common.h"
#include "ImgFeature.h"
#include "Ctracker.h"
using namespace cv;
using namespace std;
static  int NextTrackID;
class NTracker{
public:
    vector<Point2d> trace;//轨迹
    size_t tracker_id;//跟踪器标号
    size_t skipped_frames;//丢失帧数量
    Point2f prediction;//预测坐标
    TKalmanFilter* KF;//卡尔曼滤波器
    HOGLoss hog;//hogs描述子
    CLFeature hs;//hs描述子
    Point2f left_top;//行人左上坐标
    Point2f right_bottom;//行人右下坐标
    bool distribution;//是否被分配检测器
    bool existing;//当前是否被使用
    NTracker(Point2f p, float dt, float Accel_noise_mag);
    void compute(Person &person);
    ~NTracker();
};


class Tracker_Set{
public:
    float dt;
    float Accel_noise_mag;
    double dist_thres;
    int maximum_allowed_skipped_frames;
    int max_trace_length;
    vector<NTracker*> tracks;
    void Update(std::vector<Person> &person);
    void draw_person(cv::Mat &src);
    Tracker_Set(float _dt,
                float _Accel_noise_mag,
                double _dist_thres=60,
                int _maximum_allowed_skipped_frames=50,
                int _max_trace_length=10);
    ~Tracker_Set();
    void Draw_map_by_track(int num);

};

#endif //ROOKIE_NTRACKER_H
