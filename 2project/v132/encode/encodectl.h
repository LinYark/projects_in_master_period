#ifndef ENCODECTL_H
#define ENCODECTL_H
#include"common.h"

#include "encode/lib/video_encode_main.cpp"
class encodectl
{
public:
    encodectl();
    ImageData* imgData;
    void myinit(){
        thread t1(imageProcessing_store,imgData);
        t1.detach();
        initFlg =true;
    }
    static void imageProcessing_store(ImageData* imgData);
    static bool initFlg ;
};

#endif // ENCODECTL_H
