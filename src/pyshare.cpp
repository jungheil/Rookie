//
// Created by li on 7/11/2020.
//
#include "proshare.h"

extern "C"{
// 圖像傳輸
ProClient<ImageStr> img_client(1);
ImageStr *imgstr = new ImageStr ;
int target = -1;

//ImageStr* PSubscribe_(){
//    ImageStr *imgstr = new ImageStr ;
//    client.Subscribe(*imgstr);
//    return imgstr;
//}

bool PImg_Subscribe(){
    img_client.Subscribe(*imgstr);
}

int PImg_GetWidth(){
    return imgstr->width_;
}

int PImg_GetHeight(){
    return imgstr->height_;
}

uchar* PImg_GetData(){
    return imgstr->data_;
}

void PImg_Release(){
    delete imgstr;
    imgstr = nullptr;
}

// 跟蹤目標編號傳輸
ProService<int> tar_service(2);

bool PTar_Public(int tar){
    target = tar;
    tar_service.Public(&target);
}

}