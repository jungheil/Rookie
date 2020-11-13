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

template <class T>
ProService<T>::ProService(unsigned char index):ProTrans<T>(index){
    MutexInit();
    BufInit();
};

template <class T>
ProService<T>::~ProService()   {
    shmctl(this->shm_id_, IPC_RMID, 0);
    shmctl(this->mutex_, IPC_RMID, 0);
}

template <class T>
void ProService<T>::MutexInit() {
    key_t key = ftok(this->PATHNAME_,0x66+this->index_*2-1);
    this->mutex_ = semget(key, 1, IPC_CREAT|0666);

    if (this->mutex_ < 0) {
        perror("[MutexInit] ser sem_open failed");
        exit(-1);
    }
    semctl(this->mutex_, 0, SETVAL, 1);
}

template <class T>
void ProService<T>::BufInit() {
    key_t key = ftok(this->PATHNAME_,0x66+this->index_*2);
    this->shm_id_ = shmget(key, sizeof(T), IPC_CREAT|0666);
    if (this->shm_id_ < 0){
        perror("[ImgBufInit] ser shmget failed.");
        exit(-1);
    }
    this->buffer_ = (T*)shmat(this->shm_id_, NULL, 0);
    if ((long)this->buffer_ == -1){
        perror("[ImgBufInit] shmat failed.");
        exit(-1);
    }
}

template <class T>
bool ProService<T>::Public(const T *data) {
//    unsigned int img_size = width*height*3;
//    if (img_size > IMG_MAX_SIZE) return false;
//    unsigned char *data_temp = (unsigned char*)data;
//    ImageStr *image = new ImageStr ;
//    image->width_=width;
//    image->height_=height;
//    for(unsigned int i=0; i<img_size; i++){
//        image->data_[i] = *data_temp;
//        data_temp++;
//    }
    semop(this->mutex_, &this->sem_lock, 1);
    memcpy(this->buffer_, data, sizeof(*data));
    semop(this->mutex_, &this->sem_unlock, 1);
    return true;
}



template <class T>
ProClient<T>::ProClient(unsigned char index):ProTrans<T>(index){

    MutexInit();
    BufInit();
};

//template <class T>
//ProClient<T>::~ProClient() noexcept {
////    shmctl(shm_id_, IPC_RMID, 0);
////    shmctl(mutex_, IPC_RMID, 0);
//}

template <class T>
void ProClient<T>::MutexInit() {
    key_t key = ftok(this->PATHNAME_,0x66+this->index_*2-1);
    this->mutex_ = semget(key, 1, 0);

    if (this->mutex_ < 0) {
        perror("[MutexInit] cli sem_open failed");
        exit(-1);
    }
    semctl(this->mutex_, 0, SETVAL, 1);
}

template <class T>
void ProClient<T>::BufInit() {
    key_t key = ftok(this->PATHNAME_,0x66+this->index_*2);
    this->shm_id_ = shmget(key, sizeof(T), 0666);

    if (this->shm_id_ < 0){
        perror("[ImgBufInit] cli shmget failed");
        exit(-1);
    }
    this->buffer_ = (T*)shmat(this->shm_id_, NULL, 0);
    if ((long)this->buffer_ == -1){
        perror("[ImgBufInit] shmat failed");
        exit(-1);
    }
}

template <class T>
bool ProClient<T>::Subscribe(T &output) {
    semop(this->mutex_, &this->sem_lock, 1);
    memcpy(&output, this->buffer_, sizeof(output));
    semop(this->mutex_, &this->sem_unlock, 1);
    return true;
}

//bool ImgClient::Subscribe(cv::Mat &img) {
//    ImageStr *imgstr = new ImageStr ;
//    Subscribe(*imgstr);
//    int count = 0;
//    uchar* pxvec;
//    cv::Mat temp(imgstr->height_, imgstr->width_, CV_8UC3);
//    for (int row = 0; row < imgstr->height_; row++)
//    {
//        pxvec = temp.ptr<uchar>(row);
//        for(int col = 0; col < imgstr->width_; col++)
//        {
//            for(int c = 0; c < 3; c++)
//            {
//                pxvec[col*3+c] = imgstr->data_[count];
//                count++;
//            }
//        }
//    }
//    img = temp;
//    delete imgstr;
//    if(img.empty()) return false;
//    else return true;
//}

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

    unsigned int img_size = img.cols*img.rows*3;
    if (img_size > IMG_MAX_SIZE) return false;
    unsigned char *data_temp = (unsigned char*)data;
    ImageStr *image = new ImageStr ;
    image->width_=img.cols;
    image->height_=img.rows;
    for(unsigned int i=0; i<img_size; i++){
        image->data_[i] = *data_temp;
        data_temp++;
    }
    ProService<ImageStr>::Public(image);
    delete image;
}

bool ImgClient::Subscribe(cv::Mat &img) {
    ImageStr *imgstr = new ImageStr ;
    ProClient<ImageStr>::Subscribe(*imgstr);
    int count = 0;
    uchar* pxvec;
    cv::Mat temp(imgstr->height_, imgstr->width_, CV_8UC3);
    for (int row = 0; row < imgstr->height_; row++)
    {
        pxvec = temp.ptr<uchar>(row);
        for(int col = 0; col < imgstr->width_; col++)
        {
            for(int c = 0; c < 3; c++)
            {
                pxvec[col*3+c] = imgstr->data_[count];
                count++;
            }
        }
    }
    img = temp;
    delete imgstr;
    if(img.empty()) return false;
    else return true;
}