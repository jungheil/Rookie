//
// Created by li on 2020/6/12.
//

#include "common.h"
#include <sys/time.h>

Ximg& Ximg::operator=(Ximg &ximg){
    cam_ = ximg.cam_;
    rs_intrinsics_ = ximg.rs_intrinsics_;
    is_used_ = ximg.is_used_;
    header_ = ximg.header_;
    cv_color_ = ximg.cv_color_;
    cv_depth_ = ximg.cv_depth_;
    rs_depth_ = ximg.rs_depth_;
    ximg.is_used_ = true;
    return *this;
}

Ximg& Ximg::operator=(const Ximg &ximg){
    cam_ = ximg.cam_;
    rs_intrinsics_ = ximg.rs_intrinsics_;
    is_used_ = ximg.is_used_;
    header_ = ximg.header_;
    cv_color_ = ximg.cv_color_;
    cv_depth_ = ximg.cv_depth_;
    rs_depth_ = ximg.rs_depth_;
    return *this;
}


cv::Point3f Pixel2Point(Ximg img, cv::Point pixel){
    float _point[3];
    float _pixel[2] = {float(pixel.x),float(pixel.y)};
    rs2_deproject_pixel_to_point(_point,&img.rs_intrinsics_,_pixel,img.get_rs_depth().get_distance(_pixel[0],_pixel[1]));
    return cv::Point3f(_point[0],_point[1],_point[2]);
}

cv::Point3f Pixel2Point(Ximg img, cv::Point pixel, float distance){
    float _point[3];
    float _pixel[2] = {float(pixel.x),float(pixel.y)};
    rs2_deproject_pixel_to_point(_point,&img.rs_intrinsics_,_pixel,distance);
    return cv::Point3f(_point[0],_point[1],_point[2]);
}



Header::Header(uint64_t seq):seq_(seq) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    stamp_=tv.tv_sec*1E6+tv.tv_usec;
}

void DrawPred(cv::Mat &src, std::vector<Person> person)
{
    for(size_t i = 0; i<person.size(); i++){
        int left = person[i].get_box().x;
        int top = person[i].get_box().y;
        int x = person[i].get_box().x+0.5*person[i].get_box().width;
        int y = person[i].get_box().y+0.5*person[i].get_box().height;
        int right = person[i].get_box().x+person[i].get_box().width;
        int bottom = person[i].get_box().y+person[i].get_box().height;
        //Draw a rectangle displaying the bounding box
        cv::circle(src, cv::Point(x, y),2, cv::Scalar(255,0,0),-1);
        cv::rectangle(src, cv::Point(left, top), cv::Point(right, bottom), cv::Scalar(255, 255, 0), 3);
        //Get the label for the class name and its confidence
        std::stringstream strStream;
        strStream<<person[i].get_located();
        std::string label = strStream.str();
        putText(src, label, cv::Point(x, y), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0,255,0),1);
        std::string temp_id = "";
        temp_id +=std::to_string(person[i].get_id());
        cv::Point temp_point = cv::Point(left, top);
        temp_point.y +=5;
        //行人检测时左上角标号绘制
        putText(src, temp_id, temp_point, cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0,255,0),1);

    }
}

bool RectSafety(cv::Rect &brect, int rows, int cols) {
    cv::Rect out_rect=cv::Rect(0,0,cols,rows);
    brect=brect&out_rect;
    return brect.width != 0 && brect.height != 0;
}

cv::Mat To3Channels(const cv::Mat& binImg)
{
    cv::Mat three_channel = cv::Mat::zeros(binImg.rows,binImg.cols,CV_8UC3);
    std::vector<cv::Mat> channels;
    for (int i=0;i<3;i++)
    {
        channels.push_back(binImg);
    }
    merge(channels,three_channel);
    return three_channel;
}


Coordinate::Coordinate(Coordinate *found, cv::Mat &trans) {
    found_ = found;
    is_base_=false;
    trans_=trans.clone();
}

void GetTransfer(cv::Mat &matrix, float tx=0,float ty=0, float tz=0, float rx=0, float ry=0, float rz=0){
    rx = rx/180*3.1415926;
    ry = ry/180*3.1415926;
    rz = rz/180*3.1415926;

    cv::Mat trans = cv::Mat::zeros(cv::Size(4,4),CV_32FC1);
    trans.at<float>(0,0) = cos(rz)*cos(ry);
    trans.at<float>(0,1) = cos(rz)*sin(ry)*sin(rx)-sin(rz)*cos(rx);
    trans.at<float>(0,2) = cos(rz)*sin(ry)*cos(rx)+sin(rz)*sin(rx);
    trans.at<float>(0,3) = tx;

    trans.at<float>(1,0) = sin(rz)*cos(ry);
    trans.at<float>(1,1) = sin(rz)*sin(ry)*sin(rx)-cos(rz)*cos(rx);
    trans.at<float>(1,2) = sin(rz)*sin(ry)*cos(rx)-cos(rz)*sin(rx);
    trans.at<float>(1,3) = ty;

    trans.at<float>(2,0) = -sin(ry);
    trans.at<float>(2,1) = cos(ry)*sin(rx);
    trans.at<float>(2,2) = cos(ry)*cos(rx);
    trans.at<float>(2,3) = tz;

    trans.at<float>(3,0) = 0;
    trans.at<float>(3,1) = 0;
    trans.at<float>(3,2) = 0;
    trans.at<float>(3,3) = 1;
    trans.copyTo(matrix);
}

cv::Mat Coordinate::get_trans_ (){
    if(this->is_base_==false){
        return this->found_->get_trans_()*trans_;
    }else{
        return trans_;
    }
}