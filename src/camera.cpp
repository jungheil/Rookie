//
// Created by li on 2020/6/12.
//

#include "camera.h"


Realsense::Realsense() {
    cam_type_ = CAMERA_TYPE_REALSENSE;
    rs2::config cfg;
    cfg.enable_stream(RS2_STREAM_DEPTH,1280,720,RS2_FORMAT_ANY,30);
    cfg.enable_stream(RS2_STREAM_COLOR,1280,720,RS2_FORMAT_BGR8,30);
    profile_=pipe_.start(cfg);

    rs2::frameset data = pipe_.wait_for_frames(); // Wait for next set of frames from the camera
    data = align_to_color_.process(data);
    rs2::depth_frame depth = data.get_depth_frame();
    intrinsics_ = rs2::video_stream_profile(depth.get_profile()).get_intrinsics();
}
bool Realsense::GetImg(Ximg &img) {
    rs2::frameset data = pipe_.wait_for_frames(); // Wait for next set of frames from the camera
    data = align_to_color_.process(data);

//    rs2::frame depth = data.get_depth_frame().apply_filter(color_map_);
    rs2::depth_frame depth = data.get_depth_frame();
    rs2::frame color = data.get_color_frame();

    // Query frame size (width and height)
    const int w = depth.as<rs2::video_frame>().get_width();
    const int h = depth.as<rs2::video_frame>().get_height();

    // Create OpenCV matrix of size (w,h) from the colorized depth data
    cv::Mat img_d(cv::Size(w, h), CV_8UC3, (void*)depth.get_data(), cv::Mat::AUTO_STEP);
    cv::Mat img_c(cv::Size(w, h), CV_8UC3, (void*)color.get_data(), cv::Mat::AUTO_STEP);
    img = Ximg(this,img_c, depth, intrinsics_);
    return true;
}

UVC::UVC(int cam) {
    cam_type_ = CAMERA_TYPE_UVC;
    fb_ = cv::VideoCapture(cam);
}

UVC::UVC(std::string path) {
    cam_type_ = CAMERA_TYPE_UVC;
    fb_ = cv::VideoCapture(path);
}

bool UVC::GetImg(Ximg &img) {
    cv::Mat src;
    fb_ >> src;
    img = Ximg(this,src);
    return true;
}