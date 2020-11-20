#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API
#include <opencv2/opencv.hpp>   // Include OpenCV API
#include <thread>
#include <mutex>
#include <stdbool.h>
#include <proshare.h>
#include "camera.h"
#include "detector.h"
#include "tracker.h"
#include "usart.h"
#include "Ctracker.h"
#include "motion.h"
#include "proshare.h"

using namespace cv;


std::vector<Person> person;
bool is_per_used = false;

mutex cam_mut;
mutex per_mut;

void mode_print(cv::Mat &src,int flag){
    if(flag==0){
        putText(src, // 图像矩阵
                "yolov5",                  // string型文字内容
                Point2f(200,50),           // 文字坐标，以左下角为原点
                cv::FONT_HERSHEY_SIMPLEX,   // 字体类型
                3, // 字体大小
                cv::Scalar(255, 255, 0));
    }else{
        putText(src, // 图像矩阵
                "kcf",                  // string型文字内容
                Point2f(200,50),           // 文字坐标，以左下角为原点
                cv::FONT_HERSHEY_SIMPLEX,   // 字体类型
                3, // 字体大小
                cv::Scalar(255, 255, 0));
    }

}
void CameraThread(Camera *cam, Ximg *img){
    Ximg temp;

    while(true){
        //cout << 33<<endl;
        cam->GetImg(temp);
        {
            lock_guard<mutex> lock(cam_mut);
            //mut.lock();
            //if(img->is_used_) cam->GetImg(*img);
            *img = temp;
            //mut.unlock();
        }
    }
}
void ProcessThread(Ximg *img){
    char fps[10];
    XRDetector::Detector detector;
//    TracKer kcf_tracker;
    //------------------------------kcf_test
//    cv::Ptr<cv::Tracker> kcf_tracker;
//    cv::TrackerKCF::Params params;
//    params.detect_thresh = 0.3f;
//    kcf_tracker = cv::TrackerKCF::create(params);
    cv::Ptr<cv::MultiTracker> multiTracker = cv::MultiTracker::create();
    std::vector<Person> test_person;

    //------------------------------kcf_test
    CTracker tracker(0.3, 0.5, 60.0, 10, 20);
    Ximg src;
    sleep(1);
    std::vector<Person> temp_person;
    long long timer=0;
    ImgService ser(1);
    int flag=0;
    int FLAG =0;
    while(true){
        double time0 = static_cast<double>(cv::getTickCount());
        {
            lock_guard<mutex> lock(cam_mut);
            //        mut.lock();
            if (img->is_used_) continue;
            src = *img;
            //        mut.unlock();
        }
//        if(flag==0){
//            detector.UpdatePerson(src, temp_person);
//        }else if(flag==1){
////            detector.UpdatePerson_by_kcf(src,temp_person);
//        }
        detector.UpdatePerson(src, temp_person);
//        detector.UpdatePerson_by_kcf(src,test_person);
        if(temp_person.size()>0){
            timer++;
//            cout<<timer%30<<"\n";
            tracker.Update(temp_person);
//            if(flag==0){
//                cv::Ptr<cv::Tracker> kcf_tracker;
//                cv::TrackerKCF::Params params;
//                params.detect_thresh = 0.4f;
//                kcf_tracker = cv::TrackerKCF::create(params);
//                cv::Ptr<cv::MultiTracker> new_multiTracker = cv::MultiTracker::create();
//                multiTracker.swap(new_multiTracker);
////                cout<<"debug"<<multiTracker.empty()<<"\n";
//                multiTracker->add(kcf_tracker, src.get_cv_color(), cv::Rect2d(temp_person[0].get_box()));
//                flag =1;
//            }else if(flag==1){
//                multiTracker->update(src.get_cv_color());
//                rectangle(src.get_cv_color(), multiTracker->getObjects()[0],  1, 3);
//            }
//            if(kcf_tracker.success==0){
//                kcf_tracker.creat(src.get_cv_color(),temp_person);
//                kcf_tracker.update(src.get_cv_color(),temp_person);
//            }else if(kcf_tracker.success==1){
//                kcf_tracker.update(src.get_cv_color(),temp_person);
//            }
        }
        per_mut.lock();
        person = temp_person;
        is_per_used = false;
        per_mut.unlock();
//        if(flag==0){
//            tracker.draw_person(src.get_cv_color(),person);
//
//        }
        tracker.draw_person(src.get_cv_color(),person);
//        tracker.draw_person(src.get_cv_color(),test_person);
        mode_print(src.get_cv_color(),flag);

        DrawPred(src.get_cv_color(),person);


        ser.Public(src.get_cv_color());
        time0 = ((double)cv::getTickCount() - time0) / cv::getTickFrequency();
        time0 = 1.0 / time0;
        sprintf(fps,"%.2f", time0);
        putText(src.get_cv_color(), fps, cv::Point(5, 50), cv::FONT_HERSHEY_SCRIPT_SIMPLEX, 2, cv::Scalar(0, 255, 0));

        imshow("DL",src.get_cv_color());
        waitKey(1);
    }

//ImgClient client(0);
//Mat p;
//
//while(1){
//    client.Subscribe(p);
//    imshow("1",p);
//    waitKey(1);
//
//}
}

void ControlThread(CarController *cctrl){
    Motion3D motion(cctrl);
    ProService<int> tar_service(2);
    int temp_tar = -1;
    tar_service.Public(&temp_tar);
    ProClient<int> tar_client(2);
    int target = -1;
    while (true){
        tar_client.Subscribe(target);
        motion.set_target(target);
        if(!is_per_used){
            per_mut.lock();
            motion.UpdatePerson(person);
            is_per_used = true;
            per_mut.unlock();
        }
        motion.Move();
    }
}

int main(int argc, char * argv[])
{
//    UVC cam(0);
    Realsense cam;
    Ximg img;
    CarController cctrl;
    thread t1(CameraThread, &cam, &img);
    thread t2(ProcessThread, &img);
    thread t3(ControlThread, &cctrl);

    t1.join();
    t2.join();
    t3.join();

//    while(true){
//        cam.GetImg(img);
//        XRDetector::Detector detector;
//        CTracker tracker(0.2, 0.5, 60.0, 10, 30);
//        detector.UpdatePerson(img, person);
//        if(person.size()>0){
//            tracker.Update(person);
//        }
//        tracker.draw_person(img.get_cv_color(),person);
//        DrawPred(img.get_cv_color(),person);
//        imshow("111",img.get_cv_color());
//        waitKey(1);
//    }

    return 0;
}





//int main(int argc, char * argv[])
//{
//    Usart usart;
//    while (true){
//        unsigned char data[2] = {1,128};
//        usart.UsartSend(data);
//        sleep(1);
//    }
//
//}

//int main() {
//
//    Coordinate base;
//    Mat trans1,trans2;
//    GetTransfer(trans1,1,2,3,30,0,0);
//    GetTransfer(trans2,3,4,5,30,0,0);
//    Coordinate s(&base, trans1);
//    Coordinate r(&s, trans2);
//    std::cout<<r.get_trans_()<<std::endl;
//    std::cout<<trans1*trans2<<std::endl;
//
//}

