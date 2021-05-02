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

template <class T>
class ProTrans{
public:
    ProTrans() = default;
    ProTrans(char index):
            is_init_(true),
            index_(index){};

protected:
    virtual void MutexInit() = 0;
    virtual void BufInit() = 0;
public:
    bool is_init_ = false;

protected:
    unsigned char index_;
    char PATHNAME_[10] = "/dev/null";
    int mutex_;
    int shm_id_;
    void *buffer_;
    time_t time_;
    struct sembuf sem_lock{0,-1,SEM_UNDO};
    struct sembuf sem_unlock{0,1,SEM_UNDO};
};

template <class T>
class ProService: public ProTrans<T>{
public:
    ProService() = default;
    ProService(unsigned char index);
    ~ProService();
    bool Public(const T *data, bool change_time = true);

private:
    void MutexInit();
    void BufInit();
};

template <class T>
class ProClient: public ProTrans<T>{
public:
    ProClient() = default;
    ProClient(unsigned char index);
    ~ProClient() = default;

    bool Subscribe(T &output);
    bool Subscribe(T &output, time_t &time);
private:
    void MutexInit();
    void BufInit();
};

template class ProService<ImageStr>;

class ImgService: public ProService<ImageStr>{
public:
    ImgService() = default;
    ImgService(unsigned char index):ProService<ImageStr>(index){};

    bool Public(const cv::Mat &img);
};

template class ProClient<ImageStr>;

class ImgClient: public ProClient<ImageStr>{
public:
    ImgClient() = default;
    ImgClient(unsigned char index):ProClient<ImageStr>(index){};

    bool Subscribe(cv::Mat &img);
};

template class ProClient<int>;
template class ProService<int>;

template class ProClient<bool>;
template class ProService<bool>;




#endif //ROOKIE_PROSHARE_H
