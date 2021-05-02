//
// Created by li on 4/12/2020.
//

#include "KCFTracker.h"

using namespace cv;
using namespace std;

KCFTracker::KCFTracker() {
    params_.detect_thresh = detect_thresh_;
    R_b_cam = Mat::eye(3,3,CV_32FC1);
    R_b_cam.at<float>(0,0) =cos(10/180*3.1415926);
    R_b_cam.at<float>(0,1) =-sin(10/180*3.1415926);
    R_b_cam.at<float>(1,0) =sin(10/180*3.1415926);
    R_b_cam.at<float>(1,1) =cos(10/180*3.1415926);
}

void KCFTracker::InitKCF(Mat &img, vector<Person*> &person) {
    trackers_.clear();
    for(auto &p:person){
        trackers_.emplace_back(TrackerKCF::create(params_));
        trackers_.back()->init(img, p->get_box());
        p->set_kcf_tracked(true);
    }
}

void KCFTracker::Update(Ximg &img,  vector<Person*> &person) {
    Rect2d box;
    int i = 0;
    for(auto &t:trackers_){
        if(!person[i]->get_kcf_tracked()) continue;
        bool ret = t->update(img.get_cv_color(), box);
        RectSafety(box,img.get_cv_color().cols,img.get_cv_color().rows);
        if(ret){
            // 非深度没写
            if (img.cam_->GetCamType() == Camera::CAMERA_TYPE_REALSENSE){
                Mat mask;
                GetMask(img,box,mask);
                float distance=0;
                float dists[16];
                for (int j = 0; j<4;j++){
                    for(int k=0; k<4;k++){
                        if (mask.at<uchar>(mask.rows/5*(1+k),mask.cols/5*(1+j))==255){
                            dists[j*4+k]=img.get_rs_depth().get_distance(mask.cols/5*(1+j)+box.x,mask.rows/5*(1+k)+box.y);
                        }else{
                            dists[j*4+k]=0;
                        }
                    }
                }
                int count=0;
                float sum=0;
                for (int i=0; i<16;i++){
                    if (dists[i] !=0){
                        sum+=dists[i];
                        count++;
                    }
                }
                distance=sum/count;
                if(!isnormal(distance) || distance < 0.1 || distance > 8) continue;
//                cout<<"dist:"<<distance<<endl;
                Point pixel = Point(box.x+0.5*box.width,box.y+0.5*box.height);
                Point3f point = Pixel2Point(img, pixel,distance);
                Mat vec = R_b_cam*cv::Vec3f(point.x,point.y,point.z);

                point = Point3f(vec.at<float>(0,0),vec.at<float>(0,1),vec.at<float>(0,2));
                person[i]->Update(distance,point,box);
            }
        }  else{
            person[i]->set_kcf_tracked(false);
        }
        i++;
    }
}