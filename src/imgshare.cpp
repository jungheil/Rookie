//
// Created by li on 29/10/2020.
//
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <fcntl.h>


#include "imgshare.h"

using namespace std;

ImgService::ImgService(unsigned char index):ImgTrans(index){
    MutexInit();
    ImgBufInit();
};
ImgService::~ImgService()   {
    sem_close(mutex_);
    sem_unlink(SEM_NAME_);
    shmctl(shm_id_, IPC_RMID, 0);
}

void ImgService::MutexInit() {
    mutex_ = sem_open(SEM_NAME_, O_CREAT, 0644, 1);
    if (mutex_ == SEM_FAILED) {
        perror("[MutexInit] sem_open failed.");
        sem_unlink(SEM_NAME_);
        exit(-1);
    }
}

void ImgService::ImgBufInit() {
    shm_id_ = shmget(KEY_+index_, sizeof(ImageStr), IPC_CREAT | 0666);
    if (shm_id_ < 0){
        perror("[ImgBufInit] shmget failed.");
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
    sem_wait(mutex_);
    unsigned char *data_temp = (unsigned char*)data;
    ImageStr image;
    image.width_=width;
    image.height_=height;
    for(int i=0; i<img_size; i++){
        image.data_[i] = *data_temp;
        data_temp++;
    }
    memcpy(buffer_, &image, sizeof(image));
    sem_post(mutex_);
    return true;
}

ImgClient::ImgClient(unsigned char index):ImgTrans(index){
    MutexInit();
    ImgBufInit();
};
ImgClient::~ImgClient() noexcept {
    sem_close(mutex_);
    sem_unlink(SEM_NAME_);
    shmctl(shm_id_, IPC_RMID, 0);
}

void ImgClient::MutexInit() {
    mutex_ = sem_open(SEM_NAME_, 0, 0644, 1);
    if (mutex_ == SEM_FAILED) {
        perror("[MutexInit] sem_open failed.");
        sem_unlink(SEM_NAME_);
        exit(-1);
    }
}

void ImgClient::ImgBufInit() {
    shm_id_ = shmget(KEY_+index_, sizeof(ImageStr), 0666);
    if (shm_id_ < 0){
        perror("[ImgBufInit] shmget failed.");
        exit(-1);
    }
    buffer_ = (ImageStr*)shmat(shm_id_, NULL, 0);
    if ((long)buffer_ == -1){
        perror("[ImgBufInit] shmat failed.");
        exit(-1);
    }
}

bool ImgClient::Subscribe(ImageStr &output) {
    sem_wait(mutex_);
    memcpy(&output, buffer_, sizeof(output));
    sem_post(mutex_);
    return true;
}