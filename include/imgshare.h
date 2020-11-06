//
// Created by li on 29/10/2020.
//

#ifndef ROOKIE_IMGSHARE_H
#define ROOKIE_IMGSHARE_H

#include <semaphore.h>
#include <opencv2/opencv.hpp>

#define KEY 650
#define IMG_MAX_SIZE 1920*1080*3

typedef struct{
    int width_;
    int height_;
    unsigned char data_[IMG_MAX_SIZE];
}ImageStr;

class ImgTrans{
public:
    ImgTrans() = default;
    ImgTrans(unsigned char index):
            is_init_(true),
            index_(index){
        SEM_NAME_[4]=index;
    };
protected:
    virtual void MutexInit() = 0;
    virtual void ImgBufInit() = 0;
public:
    bool is_init_ = false;

protected:
    int KEY_ = KEY;
    unsigned char index_;
    char SEM_NAME_[5] = "Rkmt";
    sem_t *mutex_;
    int shm_id_;
    ImageStr *buffer_;
};

class ImgService: public ImgTrans{
public:
    ImgService() = default;
    ImgService(unsigned char index);
    ~ImgService();
    bool Public(int width, int height, void* data);

private:
    void MutexInit();
    void ImgBufInit();

};

class ImgClient: public ImgTrans{
public:
    ImgClient() = default;
    ImgClient(unsigned char index);
    ~ImgClient();

    bool Subscribe(ImageStr &output);
    bool Subscribe(cv::Mat );
private:
    void MutexInit();
    void ImgBufInit();

};

extern "C"{
    ImgService service;

}


#endif //ROOKIE_IMGSHARE_H
