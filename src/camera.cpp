//
// Created by li on 2020/6/12.
//

#include "camera.h"


Realsense::Realsense() {
    cam_type_ = CAMERA_TYPE_REALSENSE;
    rs2::config cfg;
    cfg.enable_stream(RS2_STREAM_DEPTH,1280,720,RS2_FORMAT_ANY,30);
    cfg.enable_stream(RS2_STREAM_COLOR,1280,720,RS2_FORMAT_BGR8,30);
    cfg.enable_stream(RS2_STREAM_ACCEL, RS2_FORMAT_MOTION_XYZ32F);

    profile_=pipe_.start(cfg);

    rs2::frameset data = pipe_.wait_for_frames(); // Wait for next set of frames from the camera
    data = align_to_color_.process(data);
    rs2::depth_frame depth = data.get_depth_frame();
    intrinsics_ = rs2::video_stream_profile(depth.get_profile()).get_intrinsics();

    auto sensor = profile_.get_device().query_sensors()[1];
    sensor.set_option(RS2_OPTION_GAIN,64);
    sensor.set_option(RS2_OPTION_GAMMA,500);
    sensor.set_option(RS2_OPTION_SHARPNESS,55);

    sensor.set_option(RS2_OPTION_ENABLE_AUTO_WHITE_BALANCE, true);
    sensor.set_option(RS2_OPTION_ENABLE_AUTO_EXPOSURE, true);
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

    auto fa = data.first(RS2_STREAM_ACCEL, RS2_FORMAT_MOTION_XYZ32F);
    rs2::motion_frame accel = fa.as<rs2::motion_frame>();
    rs2_vector av = accel.get_motion_data();
    float R = sqrtf(av.x * av.x + av.y * av.y + av.z * av.z);
//    float newRoll = acos(av.x / R);
//    float newYaw = acos(av.y / R);
    float pitch = 90-acos(av.z / R)/3.1415926*180;

    img = Ximg(this,img_c, depth, intrinsics_,pitch);
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