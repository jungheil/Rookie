//
// Created by li on 2020/7/1.
//

#ifndef XIROBO_NAVIGATION_H
#define XIROBO_NAVIGATION_H

#include "common.h"

//class Navigation {
//
//};

class VFH{
public:
    VFH(Ximg img);
    ~VFH();
    Gap get(Ximg img,Person person);

private:
    void GetGap(float *histogram);
    float GetAngle(int pixel, float fx);


private:
    rs2_intrinsics intrin_;
    int roi_height_;
    int img_width_;
    int img_height_;
    float *histogram;
    int dis_threshold_;
    int gap_threshold_;
    std::vector<Gap> gap_;
    float last_angle=0;
};


#endif //XIROBO_NAVIGATION_H
