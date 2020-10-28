//
// Created by li on 2020/7/1.
//

#include "navigation.h"
#include <math.h>

VFH::VFH(Ximg img) {
    int car_width = 400;
    dis_threshold_ = 800;
    intrin_=img.rs_intrinsics_;
    gap_threshold_ = car_width/dis_threshold_*img.rs_intrinsics_.fx;
    img_width_ = img.get_rs_depth().get_width();
    img_height_ = img.get_rs_depth().get_height();
    roi_height_ = img_height_ / 2;
    histogram = new float[img_width_];
}

VFH::~VFH() {
    delete[] histogram;
}

Gap VFH::get(Ximg img,Person person) {
    float histogram[img_width_];
    for(int i = 0; i<img_width_; i++){
        float min = 100;
        for(int j = 0; j<roi_height_; j++){
            float distance=img.get_rs_depth().get_distance(img_width_,img_height_-j);
            if(distance<min) min=distance;
        }
        histogram[i]=min;
    }

    GetGap(histogram);

    Gap gap;
    if(gap_.size() == 0){
        gap.no_way_=true;
    } else{
        int *loss = new int[gap_.size()];
        for(int i=0; i<gap_.size(); i++){
            int a=2;
            int b=1;
            int c=1;
            float target_direction = abs(atan2(person.get_located().x,person.get_located().z) - gap_[i].angle);
            float previous_direction = abs(last_angle-gap_[i].angle);
            loss[i]=a*target_direction+b*abs(gap_[i].angle)+c*previous_direction;
        }
        for(int j=0; j< gap_.size();j++){
            float min = 100;
            if (loss[j]<min){
                gap=gap_[j];
                min = loss[j];
            }
        }
    }

    return gap;
}

void VFH::GetGap(float *histogram) {
    gap_.clear();
    int x_size = sizeof(histogram)/sizeof(*histogram);
    bool *pass = new bool[x_size];
    for (int i=0; i<x_size; i++){
        if ((int)histogram[i]*1000 > dis_threshold_) pass[i]=true;
    }

    // 中值滤波
    int kernel = 5;
    bool *pass_m = new bool[x_size-kernel+1];
    for(int j=0; j<(x_size-kernel+1); j++){
        int sum = 0;
        for(int k=0; k<kernel; k++){
            sum+=pass[j+k];
        }
        pass_m[j]=sum/kernel;
    }

    for(int i=0; i<x_size-kernel+1; i++){
        int length=0;
        int start=-1;
        if (pass_m[i]) {
            if(start==-1){
                start=i;
                length++;
            }else{
                length++;
            }
        } else {
            if(start!=-1){
                if(length>gap_threshold_){
                    Gap gap;
                    gap.angle=GetAngle(start+length/2,intrin_.fx);
                    gap.width=length;
                    gap_.push_back(gap);
                }
                start=-1;
                length=0;
            }
        }
    }
    delete[] pass;
}

float VFH::GetAngle(int pixel, float fx){
    return atan2(pixel,fx);
}