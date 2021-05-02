//
// Created by li on 2020/6/12.
//

#ifndef XIROBO_CAMERA_H
#define XIROBO_CAMERA_H

#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API
#include <opencv2/opencv.hpp>   // Include OpenCV API

#include "common.h"


class Realsense: public Camera{
public:
    Realsense();
    bool GetImg(Ximg &img);

protected:
    rs2::colorizer color_map_;
    rs2::pipeline pipe_;
    rs2::align align_to_color_ = rs2::align(RS2_STREAM_COLOR);
    rs2::pipeline_profile profile_;
    rs2_intrinsics intrinsics_;
};



class UVC: public Camera{
public:
    UVC(int cam);
    UVC(std::string path);
    bool GetImg(Ximg &img);

protected:
    cv::VideoCapture fb_;
};




#endif //XIROBO_CAMERA_H
