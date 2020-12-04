//
// Created by li on 4/12/2020.
//

#include "KCFTracker.h"

using namespace cv;
using namespace std;

KCFTracker::KCFTracker() {
    params_.detect_thresh = detect_thresh_;
}

void KCFTracker::InitKCF(Mat &img, vector<Person*> person) {
    Ptr<Tracker> tracker;
    trackers_.clear();
    for(auto p:person){
        tracker = TrackerKCF::create(params_);
        tracker->init(img, p->get_box());
        trackers_.push_back(tracker);
        p->set_kcf_tracked(true);
    }
}

void KCFTracker::Update(Ximg &img,  vector<Person*> person) {
    Rect2d box;
    int i = 0;
    for(auto s:trackers_){
        if(!person[i]->get_kcf_tracked()) continue;
        bool ret = s->update(img.get_cv_color(), box);
        if(ret){
            person[i]->set_box(box);
        }  else{
            person[i]->set_kcf_tracked(false);
        }
        i++;
    }
}