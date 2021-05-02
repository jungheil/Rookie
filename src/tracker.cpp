//
// Created by yang on 2020/6/30.
//

#include "tracker.h"
//KCF调用的跟踪函数，设定跟踪person.id为0的
int TracKer::tracking(cv::Mat &src,std::vector<Person> &person,cv::VideoCapture capture) {
    cv::namedWindow("Tracker",0);//创建窗口
    cv::resizeWindow("Tracker", 1500, 1500); //创建一个500*500大小的窗口
    double fps;
    char string[10];
    int left = person[0].get_box().x;
    int top = person[0].get_box().y;
    std::string temp_id = "";
    temp_id +=std::to_string(person[0].get_id());
    cv::Point temp_point = cv::Point(left, top);
    temp_point.y +=5;
    std::vector<cv::Rect> personboxes;
    std::vector<cv::Scalar> colors;
    cv::Mat frame;
    cv::Ptr<cv::Tracker> tracker;
    cv::TrackerKCF::Params params;
    params.detect_thresh = 0.4f;
    tracker = cv::TrackerKCF::create(params);
    std::cout<<"test"<<person.size()<<std::endl;
    personboxes.push_back(cv::Rect(person[0].get_box().x, person[0].get_box().y, person[0].get_box().width, person[0].get_box().height));
    getRandomColors(colors, personboxes.size());
    cv::Ptr<cv::MultiTracker> multiTracker = cv::MultiTracker::create();
    multiTracker->add(tracker, src, cv::Rect2d(personboxes[0]));
    multiTracker->add(tracker, src, cv::Rect2d(personboxes[1]));
    while (capture.isOpened()){
        char key = cv::waitKey(1);

        if (key == 'w')  // 按w键退出跟踪
        {
            return 0;

        }
        capture >> frame;
        double time0 = static_cast<double>(cv::getTickCount());
        bool ok = multiTracker->update(frame);
        if (ok == true) {
            std::cout << "Tracking success" << std::endl;
        } else {
            std::cout << "Tracking failure" << std::endl;
        }
        rectangle(frame, multiTracker->getObjects()[0], colors[0], 2, 1);
        rectangle(frame, multiTracker->getObjects()[1], colors[1], 2, 1);
        time0 = ((double)cv::getTickCount() - time0) / cv::getTickFrequency();
        fps = 1.0 / time0;
        sprintf(string,"%.2f", fps);
        putText(frame, temp_id, temp_point, cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0,255,0),1);
        putText(frame, string, cv::Point(5, 50), cv::FONT_HERSHEY_SCRIPT_SIMPLEX, 2, cv::Scalar(0, 255, 0));
        imshow("Tracker", frame);
        cv::waitKey(1);
        if (ok == true) {
            continue;
        } else {
            return 0;
        }
    }

}
//获得随机颜色框
void TracKer:: getRandomColors(std::vector<cv::Scalar> &colors, int numColors)
{
    cv::RNG rng(0);
    for (int i = 0; i < numColors; i++){
        colors.push_back(cv::Scalar(rng.uniform(0, 255), rng.uniform(0, 255), rng.uniform(0, 255)));
    }
}