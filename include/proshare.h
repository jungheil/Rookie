//
// Created by li on 29/10/2020.
//

#ifndef ROOKIE_PROSHARE_H
#define ROOKIE_PROSHARE_H

#include <semaphore.h>
#include <opencv2/opencv.hpp>
#include <sys/sem.h>


#define IMG_MAX_SIZE 1920*1080*3

typedef struct{
    int width_;
    int height_;
    unsigned char data_[IMG_MAX_SIZE];
}ImageStr;

class ImgTrans{
public:
    ImgTrans() = default;
    ImgTrans(char index):
            is_init_(true),
            index_(index){};
protected:
    virtual void MutexInit() = 0;
    virtual void ImgBufInit() = 0;
public:
    bool is_init_ = false;

protected:
    unsigned char index_;
    char PATHNAME_[5] = "Rkmt";
    int mutex_;
    int shm_id_;
    ImageStr *buffer_;

    struct sembuf sem_lock{0,-1,SEM_UNDO};
    struct sembuf sem_unlock{0,1,SEM_UNDO};
};

class ImgService: public ImgTrans{
public:
    ImgService() = default;
    ImgService(unsigned char index);
    ~ImgService();
    bool Public(int width, int height, void* data);
    bool Public(const cv::Mat &img);

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
    bool Subscribe(cv::Mat &img);
private:
    void MutexInit();
    void ImgBufInit();

};




#endif //ROOKIE_PROSHARE_H
