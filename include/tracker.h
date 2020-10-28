//
// Created by yang on 2020/7/11.
//

#ifndef XIROBO_TRACKER_H
#define XIROBO_TRACKER_H

#endif //XIROBO_TRACKER_H
#include <opencv4/opencv2/core/utility.hpp>
#include <opencv4/opencv2/tracking.hpp>
#include <opencv4/opencv2//videoio.hpp>
#include <opencv4/opencv2//highgui.hpp>
#include <opencv4/opencv2/opencv.hpp>
#include <iostream>
#include <cstring>
#include <vector>
#include<stdlib.h>
#include "common.h"
class TracKer{
public:
    TracKer(){};
    ~TracKer(){};
    int tracking(cv::Mat &src,std::vector<Person> &person,cv::VideoCapture capture);
    void getRandomColors(std::vector<cv::Scalar> &colors, int numColors);
};
