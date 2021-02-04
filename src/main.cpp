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
#include "KCFTracker.h"

using namespace cv;


std::vector<Person> PERSON;
std::vector<Person> SORT_PERSON;
cv::Mat SORT_IMG;
bool SORT_UPDATE = false;
std::vector<PersonHistory> PERSON_HIS;
bool PERSON_USED = false;
bool SORT_IMG_USED = true;
bool KCF_IMG_USED = true;


mutex CAM_MUT;
mutex PER_MUT;
mutex KCF_MUT;


[[noreturn]] void CameraThread(Camera *cam, Ximg *img){
    Ximg temp;

    while(true){
        //cout << 33<<endl;
        cam->GetImg(temp);
        {
            lock_guard<mutex> lock(CAM_MUT);
            //mut.lock();
            //if(img->is_used_) cam->GetImg(*img);
            *img = temp;
            //mut.unlock();

            SORT_IMG_USED = false;
            KCF_IMG_USED = false;
        }
    }
}

[[noreturn]] void ProcessThread(Ximg *img){
    XRDetector::Detector detector;
    CTracker tracker(0.3, 0.5, 60.0, 10, 20);
    Ximg src;
    sleep(1);
    std::vector<Person> temp_person;
    std::vector<PersonHistory> KCF_person;
    ImgService ser(1);


    while(true){
        {
            lock_guard<mutex> lock(CAM_MUT);
            //        mut.lock();
            if (SORT_IMG_USED) continue;
            src = *img;
            //        mut.unlock();
            SORT_IMG_USED = true;
        }
        detector.UpdatePerson(src, temp_person);
        if(!temp_person.empty()){
            tracker.Update(temp_person);
        }

        PER_MUT.lock();
        PERSON = temp_person;
        PERSON_USED = false;
        PER_MUT.unlock();
        tracker.draw_person(src.get_cv_color(),temp_person);
        DrawPred(src.get_cv_color(),temp_person);
        ser.Public(src.get_cv_color());
        imshow("DL",src.get_cv_color());
        waitKey(1);

//        KCF_MUT.lock();
//        SORT_PERSON.clear();
//        SORT_PERSON = temp_person;
//        img->get_cv_color().copyTo(SORT_IMG);
//        SORT_UPDATE = true;
////        is_per_used = false;
//        KCF_person = PERSON_HIS;
//        KCF_MUT.unlock();
//        for(auto &s:KCF_person){
//            tracker.update_by_kcf(s.get_id(),s.located);
//        }
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

[[noreturn]] void KCFThread(Ximg *img){
    Ximg src;
    sleep(1);
    bool update = false;
    cv::Mat sort_img;
    KCFTracker tracker;
    std::vector<Person> temp_person;
    std::vector<Person*> track_person;
    std::vector<PersonHistory> person_his;
    ImgService ser(1);


    while(true){
        KCF_MUT.lock();
        update = SORT_UPDATE;
        if(SORT_UPDATE) {
            temp_person = SORT_PERSON;
            SORT_IMG.copyTo(sort_img);
            SORT_UPDATE = false;
        }
        KCF_MUT.unlock();
        if(update){
            cout<<11111111<<endl;
            person_his.clear();
            track_person.clear();
            for(auto &s:temp_person){
                if(s.get_tracked()) track_person.push_back(&s);
                person_his.emplace_back(s.get_id());
            }
        }else{
            cout<<2222222<<endl;
        }

        if(track_person.empty()) {
            continue;
        }
        if(update){
            tracker.InitKCF(sort_img, track_person);
        }

        CAM_MUT.lock();
        if (KCF_IMG_USED) {
            CAM_MUT.unlock();
            continue;
        }
        src = *img;
        KCF_IMG_USED = true;
        CAM_MUT.unlock();

        tracker.Update(src, track_person);
        int i = 0;
        for(auto s: track_person){
            if(s->get_kcf_tracked()){
                person_his[i].located.push_back(s->get_located_xz());
            }
            i++;
        }
        KCF_MUT.lock();
        PERSON_HIS = person_his;
        KCF_MUT.unlock();
        PER_MUT.lock();
        PERSON = temp_person;
        PERSON_USED = false;
        PER_MUT.unlock();

        DrawPred(src.get_cv_color(),temp_person);
        ser.Public(src.get_cv_color());
//        imshow("DL",src.get_cv_color());
        DrawPred(sort_img, temp_person);

        imshow("111",sort_img);
        waitKey(1);
    }


}

[[noreturn]] void ControlThread(CarController *cctrl){
    Motion3D motion(cctrl);
    ProService<int> tar_service(2);
    ProService<bool> run_service(3);
    int target = -1;
    bool is_run = true;
    tar_service.Public(&target);
    run_service.Public(&is_run);
    ProClient<int> tar_client(2);
    ProClient<bool> run_client(3);
    while (true){
        tar_client.Subscribe(target);
        run_client.Subscribe(is_run);
        motion.set_target(target);
        if(!PERSON_USED){
            PER_MUT.lock();
            motion.UpdatePerson(PERSON);
            PERSON_USED = true;
            PER_MUT.unlock();
        }
        motion.Move(!is_run);
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
//    thread t3(KCFThread, &img);
    thread t4(ControlThread, &cctrl);

    t1.join();
    t2.join();
//    t3.join();
    t4.join();

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

