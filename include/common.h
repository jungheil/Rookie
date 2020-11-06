//
// Created by li on 2020/6/12.
//

#ifndef XIROBO_COMMON_H
#define XIROBO_COMMON_H

#include <opencv2/opencv.hpp>
#include <librealsense2/rsutil.h>
#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API


class Header{
public:
    Header(uint64_t seq = 0);
public:
    uint64_t stamp_;
    uint64_t seq_=0;
};

class Camera;

class Ximg{
public:
    Ximg(Camera *cam,cv::Mat color, rs2::frame depth,rs2_intrinsics intrinsics, uint64_t seq=0):
            cam_(cam),
            cv_color_(std::move(color)),
            rs_depth_(std::move(depth)),
            rs_intrinsics_(intrinsics),
            header_(seq),
            is_used_(false){};
    Ximg(Camera *cam,cv::Mat color, cv::Mat depth, uint64_t seq=0):
            cam_(cam),
            cv_color_(std::move(color)),
            cv_depth_(std::move(depth)),
            header_(seq),
            is_used_(false){};
    Ximg(Camera *cam,cv::Mat color, uint64_t seq=0):
            cam_(cam),
            cv_color_(std::move(color)),
            header_(seq),
            is_used_(false){};
    Ximg(){};
    Ximg(Ximg &ximg){*this = ximg;};
    Ximg& operator=(Ximg &ximg);
    Ximg& operator=(const Ximg &ximg);
    inline cv::Mat& get_cv_color() {is_used_=true; return cv_color_;};
    inline rs2::depth_frame& get_rs_depth() {is_used_=true; return rs_depth_;};
public:
    Camera *cam_ = nullptr;
    rs2_intrinsics rs_intrinsics_;
    bool is_used_ = true;
private:
    Header header_;
    cv::Mat cv_color_;
    cv::Mat cv_depth_;
    rs2::depth_frame rs_depth_= rs2::depth_frame(rs2::frame());
};

class Camera {
public:
    enum CAMERA_TYPE{
        CAMERA_TYPE_UNKOWN = 0,
        CAMERA_TYPE_REALSENSE = 1,
        CAMERA_TYPE_UVC = 2
    };
//    Camera();
    virtual bool GetImg(Ximg &img)=0;
    inline CAMERA_TYPE GetCamType(){return cam_type_;};
protected:
    CAMERA_TYPE cam_type_ = CAMERA_TYPE_UNKOWN;
};

class Person{
public:
    Person(cv::Point3f located, cv::Rect box):if_3d_(true),located_(located),box_(box),width_(box.width){
        box_center_ = cv::Point2f(box_.x+0.5*box_.width,box_.y+0.5*box_.height);
    };
    Person(cv::Rect box):box_(box),width_(box.width){
        box_center_ = cv::Point2f(box_.x+0.5*box_.width,box_.y+0.5*box_.height);
    };
    inline cv::Point3f get_located(){return located_;};
    inline cv::Rect get_box(){return box_;};
    inline cv::Point2f get_box_center(){return box_center_;};
    inline int get_id(){return id_;};

    inline void set_id(int id){id_=id;};
    inline bool if_track(){return if_track_;};
    inline bool if_3d(){return if_3d_;}
    //cv::Point2f pixel;
private:
    cv::Point3f located_;
    cv::Rect box_;
    cv::Point2f box_center_;
    int width_;
    int id_ = -1;
    bool if_track_;
    bool if_3d_ = false;
};

typedef struct{
    bool no_way_;
    float angle;
    int width;
} Gap;

void DrawPred(cv::Mat &src, std::vector<Person> person);

cv::Point3f Pixel2Point(Ximg img, cv::Point pixel);
cv::Point3f Pixel2Point(Ximg img, cv::Point pixel, float distance);

bool RectSafety(cv::Rect &brect, int rows, int cols) ;

cv::Mat To3Channels(const cv::Mat& binImg);


void GetTransfer(cv::Mat &matrix, float tx,float ty, float tz, float rx, float ry, float rz);

class Coordinate{
public:
    Coordinate()=default;
    Coordinate(Coordinate *found, cv::Mat &trans);

    bool get_is_base_() { return is_base_; }
    cv::Mat get_trans_ ();

public:
    Coordinate *found_ = NULL;
private:
    bool is_base_ = true;
    cv::Mat trans_ = cv::Mat::eye(cv::Size(4,4),CV_32FC1);
};







#endif //XIROBO_COMMON_H
