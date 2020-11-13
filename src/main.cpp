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
    XRDetector::Detector detector;
    CTracker tracker(0.2, 0.5, 60.0, 10, 30);
    Ximg src;
    sleep(1);
    std::vector<Person> temp_person;
    ImgService ser(1);


    while(true){
        {
            lock_guard<mutex> lock(cam_mut);
            //        mut.lock();
            if (img->is_used_) continue;
            src = *img;
            //        mut.unlock();
        }
        detector.UpdatePerson(src, temp_person);
        if(temp_person.size()>0){
            tracker.Update(temp_person);
        }
        per_mut.lock();
        person = temp_person;
        is_per_used = false;
        per_mut.unlock();
        tracker.draw_person(src.get_cv_color(),person);
        DrawPred(src.get_cv_color(),person);
        ser.Public(src.get_cv_color());
//        imshow("DL",src.get_cv_color());
//        waitKey(1);
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

