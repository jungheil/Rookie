//
// Created by li on 2020/6/12.
//

#ifndef XIROBO_CAMERA_H
#define XIROBO_CAMERA_H

#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API
#include <opencv4/opencv2/opencv.hpp>   // Include OpenCV API

#include "common.h"


class Realsense:public Camera{
public:
    Realsense();
    virtual bool GetImg(Ximg &img);

private:
    CAMERA_TYPE cam_type_ = CAMERA_TYPE_REALSENSE;

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
    cv::VideoCapture fb_;
private:
    CAMERA_TYPE cam_type_ = CAMERA_TYPE_UVC;

};




#endif //XIROBO_CAMERA_H
