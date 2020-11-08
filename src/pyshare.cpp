//
// Created by li on 7/11/2020.
//
#include "proshare.h"

extern "C"{
ImgClient client(0);
ImageStr *imgstr = new ImageStr ;

//ImageStr* PSubscribe_(){
//    ImageStr *imgstr = new ImageStr ;
//    client.Subscribe(*imgstr);
//    return imgstr;
//}

bool PSubscribe(){
    client.Subscribe(*imgstr);
}

int PGetWidth(){
    return imgstr->width_;
}

int PGetHeight(){
    return imgstr->height_;
}

uchar* PGetData(){
    return imgstr->data_;
}

void PRelease(){
    delete imgstr;
    imgstr = nullptr;
}

}