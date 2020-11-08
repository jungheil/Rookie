//
// Created by li on 29/10/2020.
//
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/shm.h>
#include <fcntl.h>


#include "proshare.h"

using namespace std;

ImgService::ImgService(unsigned char index):ImgTrans(index){
    MutexInit();
    ImgBufInit();
};
ImgService::~ImgService()   {
    shmctl(shm_id_, IPC_RMID, 0);
    shmctl(mutex_, IPC_RMID, 0);
}

void ImgService::MutexInit() {
    key_t key = ftok(PATHNAME_,0x66+index_*2-1);
    mutex_ = semget(key, 1, IPC_CREAT|0666);

    if (mutex_ < 0) {
        perror("[MutexInit] ser sem_open failed");
        exit(-1);
    }
    semctl(mutex_, 0, SETVAL, 1);
}

void ImgService::ImgBufInit() {
    key_t key = ftok(PATHNAME_,0x66+index_*2);
    shm_id_ = shmget(key, sizeof(ImageStr), IPC_CREAT|0666);
    if (shm_id_ < 0){
        perror("[ImgBufInit] ser shmget failed.");
        exit(-1);
    }
    buffer_ = (ImageStr*)shmat(shm_id_, NULL, 0);
    if ((long)buffer_ == -1){
        perror("[ImgBufInit] shmat failed.");
        exit(-1);
    }
}

bool ImgService::Public(int width, int height, void* data) {
    unsigned int img_size = width*height*3;
    if (img_size > IMG_MAX_SIZE) return false;
    semop(mutex_, &sem_lock, 1);
    unsigned char *data_temp = (unsigned char*)data;
    ImageStr image;
    image.width_=width;
    image.height_=height;
    for(int i=0; i<img_size; i++){
        image.data_[i] = *data_temp;
        data_temp++;
    }
    memcpy(buffer_, &image, sizeof(image));
    semop(mutex_, &sem_unlock, 1);
    return true;
}

bool ImgService::Public(const cv::Mat &img) {
    int count = 0;
    const uchar* pxvec;
    unsigned char data[img.cols*img.rows*3];
    for (int row = 0; row < img.rows; row++)
    {
        pxvec = img.ptr<uchar>(row);
        for(int col = 0; col < img.cols; col++)
        {
            for(int c = 0; c < 3; c++)
            {
                data[count] = pxvec[col*3+c];
                count++;
            }
        }
    }
    Public(img.cols,img.rows,data);
}

ImgClient::ImgClient(unsigned char index):ImgTrans(index){
    MutexInit();
    ImgBufInit();
};
ImgClient::~ImgClient() noexcept {
//    shmctl(shm_id_, IPC_RMID, 0);
//    shmctl(mutex_, IPC_RMID, 0);
}

void ImgClient::MutexInit() {
    key_t key = ftok(PATHNAME_,0x66+index_*2-1);
    mutex_ = semget(key, 1, 0);

    if (mutex_ < 0) {
        perror("[MutexInit] cli sem_open failed");
        exit(-1);
    }
    semctl(mutex_, 0, SETVAL, 1);
}

void ImgClient::ImgBufInit() {
    key_t key = ftok(PATHNAME_,0x66+index_*2);
    shm_id_ = shmget(key, sizeof(ImageStr), 0666);

    if (shm_id_ < 0){
        perror("[ImgBufInit] cli shmget failed");
        exit(-1);
    }
    buffer_ = (ImageStr*)shmat(shm_id_, NULL, 0);
    if ((long)buffer_ == -1){
        perror("[ImgBufInit] shmat failed");
        exit(-1);
    }
}

bool ImgClient::Subscribe(ImageStr &output) {
    semop(mutex_, &sem_lock, 1);
    memcpy(&output, buffer_, sizeof(output));
    semop(mutex_, &sem_unlock, 1);
    return true;
}

bool ImgClient::Subscribe(cv::Mat &img) {
    ImageStr imgstr;
    Subscribe(imgstr);
    int count = 0;
    uchar* pxvec;
    cv::Mat temp(imgstr.height_, imgstr.width_, CV_8UC3);
    for (int row = 0; row < imgstr.height_; row++)
    {
        pxvec = temp.ptr<uchar>(row);
        for(int col = 0; col < imgstr.width_; col++)
        {
            for(int c = 0; c < 3; c++)
            {
                pxvec[col*3+c] = imgstr.data_[count];
                count++;
            }
        }
    }
    img = temp;
    if(img.empty()) return false;
    else return true;
}
