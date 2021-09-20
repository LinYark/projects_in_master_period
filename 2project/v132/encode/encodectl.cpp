#include "encodectl.h"

encodectl::encodectl(){ }
bool encodectl::initFlg=false ;
void encodectl::imageProcessing_store(ImageData* imgData)
{
    int Video_time;
    bool ProcessResult=true;

    cout<<"RGB imageProcessing open\n";
//    QThread::sleep(30);
    cout<<"get "<<imgData->codeFlg<<endl;
    while(imgData->codeFlg)
    {
        Video_time = getTickCount();

        string video_name="/home/nvidia/VID/rgb"+to_string(Video_time)+".265" ;
        imgData->visPath="/home/nvidia/VID/pkg" +to_string(Video_time) +".txt";
        cout<<"Video: "<<Video_time <<endl;
        cout<<"path : "<<imgData->visPath<<endl;
//        common::m1.lock();
//        ImageData imgData_in={ imgData->vis.clone(),imgData->visNew,imgData->visPath,imgData->codeFlg
//                             };
//        common::m1.unlock();

        ProcessResult = imageProcessmain(imgData, video_name,"1920","1080");

        cout<<"over recording!!!!!!!"<<endl;
    }

}
