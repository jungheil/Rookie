#include <librealsense2/rs.hpp> // Include RealSense Cross Platform API
#include <opencv2/opencv.hpp>   // Include OpenCV API
#include <thread>
#include <mutex>
#include <stdbool.h>
#include "camera.h"
#include "detector.h"
#include "tracker.h"
#include "usart.h"

using namespace cv;

std::vector<Person> person;

mutex cam_mut;



void CameraThread(Camera *cam, Ximg *img){
    while(true){
//        cout << 33<<endl;
        lock_guard<mutex> lock(cam_mut);
        //mut.lock();
        //if(img->is_used_) cam->GetImg(*img);
        cam->GetImg(*img);
        //mut.unlock();

    }
}
void ProcessThread(Ximg *img){
    XRDetector::Detector detector;
    Ximg src;
    sleep(1);

    while(true){
        {
            lock_guard<mutex> lock(cam_mut);
            //        mut.lock();
            if (img->is_used_) continue;
            src = *img;
            //        mut.unlock();
        }
        detector.UpdatePerson(src, person);

        DrawPred(src.get_cv_color(),person);
        imshow("src",src.get_cv_color());
        waitKey(1);
    }
}

int main(int argc, char * argv[])
{
    UVC cam(0);
    Ximg img;

    thread t1(CameraThread, &cam, &img);
    thread t2(ProcessThread, &img);

    t1.join();
    t2.join();

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
