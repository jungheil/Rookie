#ifndef ROOKIE_KCFTRACKER_H
#define ROOKIE_KCFTRACKER_H

#include <opencv2/tracking.hpp>
#include "common.h"

class KCFTracker {
public:
KCFTracker() ;
void InitKCF(cv::Mat &img, std::vector<Person*> person);
void Update(Ximg &img, std::vector<Person*> person);

private:
float detect_thresh_ = 0.4f;
cv::TrackerKCF::Params params_;
std::vector<cv::Ptr<cv::Tracker>> trackers_;

};

#endif