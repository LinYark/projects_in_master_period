#ifndef LOCALDATA_H
#define LOCALDATA_H
#include<common.h>

class localdata
{
public:
    void myinit(string txtPath,string framePath,int txtCounts=0, int frameCounts=0){
        this->setCounts(txtCounts,  frameCounts);
        this->setPath( txtPath, framePath);
        ioInit();
        initFlg = true;
    }

    void update(){
        this->myLGD=this->updGPS();
        this->myFrame = this->updFrame();
    }
    Mat getImage(){
        return Mat(this->myFrame);
    }
    localGPSData getGpsData(){
        return localGPSData(this->myLGD);
    }

    static bool initFlg ;
private:
    //in
    int txtCounts;
    int frameCounts;
    string txtPath;
    string framePath;

    //ctl
    ifstream txtReader;
    VideoCapture frameReader;

    //out
    Mat myFrame;
    localGPSData myLGD;

private:
    void setCounts(int txt, int frame){
        this->txtCounts=txt;
        this->frameCounts=frame;
    }
    void setPath(string txt,string frame ){
        this->txtPath=txt;
        this->framePath=frame;
    }
    void ioInit(){
        this->txtReader.open(this->txtPath);
        this->frameReader.open(this->framePath);
    }

    localGPSData updGPS(){
        double d0,d1,d2,d3,d4,d5,d6,d7,d8;
        static bool o1=false;
        if(o1==false){
            int times=30;
            for(int i =0;i<times;i++){
                txtReader>>d0>>d1>>d2>>d3>>d4>>d5>>d6>>d7>>d8;
            }
            o1=true;
        }

        //get
        if(!(txtReader>>d0>>d1>>d2>>d3>>d4>>d5>>d6>>d7>>d8)){
            cout<<"txt is finish"<<endl;
            txtReader.close();
            exit(0);
        };

        return localGPSData{d0,d1,d2,d3,d4,d5,d6,d7,d8};
    }

    Mat updFrame(){
        Mat currentFrame;
        static bool o=false;
        if(o==false){
            int times=20;
            for(int i =0;i<times;i++){
                frameReader.read(currentFrame);
            }
            o=true;
        }

        //get
        if(!(frameReader.read(currentFrame))){
            cout<<"video is finish"<<endl;
            frameReader.release();
            exit(0);
        };

        return currentFrame;
    }

};

class onlinedata{

};

#endif // LOCALDATA_H
