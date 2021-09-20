#ifndef PIPELINEDATA_H
#define PIPELINEDATA_H

#include "common.h"
#include <rs.hpp>

struct airAttitude
{
    uint32_t aTime;
    double Lon;
    double LonO;
    double LonN;
    double Lat;
    double LatO;
    double LatN;
    float Height;
    float HeightO;
    float HeightN;
    float Roll;
    float RollO;
    float RollN;
    float Pitch;
    float PitchO;
    float PitchN;
    float Yaw;
    float YawO;
    float YawN;
    unsigned short hLidar;
    unsigned short hLidarO;
    unsigned short hLidarN;
    unsigned short oLidar;
    unsigned short oLidarO;
    unsigned short oLidarN;
    char status;
    char statusO;
    char statusN;
    bool Assignment = false;
    char CRC;
    int K1 = 3;
    int K2 = 3;
    int K3 = 3;
    int K4 = 3;
    int K5 = 3;
    int K6 = 3;
    int K7 = 3;
    int K8 = 3;
};

struct airResult        //Detection status
{
    char status;
    uint32_t Xcoord;
    uint32_t Ycoord;
    char CRC;
};

struct fControl
{
    uint32_t fTime;
    uint8_t Coord;        // 0 for airplane, 1 for world
    uint8_t ControlMode;
    float OffsetX = 9999;
    float OffsetY = 9999;
    float OffsetZ;
    float OffsetW;
    char CRC;
};

struct TossStuct
{
    char numPipe;
    char tossStatus;
    char tossSuc;
    char CRC;
};

struct DetResult
{
    bool gRun = true;
    int index_img;
    vector<int> point;
    cv::Point p;
    cv::Mat loc;
    vector<UAVParam> uavparam;
    bool TargetLoss ;
    bool Reset;
    uint32_t Distance=999999999;
};

struct trackBox
{
    cv::Rect trackInitRect;
    cv::Mat trackInitMat;
    int kLock = 0;
    bool trueTraget = false;
    int lockConf = 10;
};

class pipelineData
{
public:  
    pipelineData(){}
    void myinit(string configPath){ //图片的路径就是参数
        ifstream readFile;  //输入流readFile
        readFile.open(configPath); //打开路径
        string temp; //定义temp用来占位
        readFile>>temp>>this->SendFlag; //this指针用来类内的指向，在同一个类中定义和指向
        readFile>>temp>>this->magicStop;
        readFile>>temp>>this->TargetLossHeight;
        readFile>>temp>>this->TossXOffset;
        readFile>>temp>>this->TossYOffset;
        readFile>>temp>>this->TakeOffD_1;
        readFile>>temp>>this->TakeOffD_2;
        readFile>>temp>>this->TossConf;
        readFile>>temp>>this->ReturnConf;
        readFile.close();
    }
    //Pipe Communication
    void recordairAttitude(airAttitude SairAttitude,int num,char* pathName);
    long getCurrentTime();
    void printairAttitude(airAttitude SairAttitude);
    int UART0_Open(int fd,char* port);
    void UART0_Close(int fd);
    int UART0_Set(int fd,int speed,int flow_ctrl,int databits,int stopbits,int parity);
    int UART0_Init(int fd, int speed,int flow_ctrl,int databits,int stopbits,int parity);
    unsigned short UART0_Recv(int fd, char *rcv_buf,int data_len);
    int UART0_Send(int fd, char *send_buf,int data_len);
    //################

    airAttitude SairAttitude;
    airResult SairResult;
    fControl SfControl;
    TossStuct SToss;
    DetResult SDetResult;
    trackBox StrackBox; //以上都是定义结构体类型的变量
    char ID; //定义ID，计次使用
    bool shutdown=false;
    char countCRC(char sbuf[], int num);
    void CopeSerialData(char ucData[],unsigned short usLength);
    void Send0x01();
    bool VisualInital=true;
    int len=0;
    int fd=0;
    void Update0x02();
    void Send0x02();
    void Reset(float k);
    bool vDetect=true;
    int sendKey=1;
    void Send0x03();
    void Send0x031();
    void Send0x04();
    void Update0x04(unsigned char* chrTemp);
    void SendStop();
    void Send0x05();
    int SendCount = 0;
    bool T = false;
    double Toss=99999999;
    float SendOffsetX = 0;
    float SendOffsetY = 0;
    void coutRec(unsigned char* c, int num, ofstream& of);
    void coutRecC(char* c, int num, ofstream& of);
    int ErrorRec = 0;
    int ErrorCRC = 0;
    void countCRCRec(unsigned char* c, int num);
    ofstream ofs_a{"/home/nvidia/jiqun/log.txt",std::ios::app};
    ofstream ofs_r{"/home/nvidia/jiqun/rec.txt",std::ios::app};
    ofstream ofs_s{"/home/nvidia/jiqun/send.txt",std::ios::app};
    bool DetectStart = false;
    bool TrackStrart = false;
    int findFlg = 5 ;
    cv::Mat DetectFrame;
    cv::Mat TrackFrame;
    cv::Mat ShowFrame;
    cv::Mat depFrame;
    Bbox detBoxBest;
    bool track_init = false;
    bool cam_start = true;
    vector<Bbox> SBox;
    rs2::pipeline_profile rs2;
    bool matReload = false;
    bool changeTossHeight = true;
    bool controlPipeline = true;
    bool resetWithSurface = false;
    UAVParam t;

    // Param Set，设置了一些初始的参数值，例如标志位，目标丢失高度，返航等
    int SendFlag = 29;
    float magicStop = 4.5;
    float TargetLossHeight = 6;
    float TossXOffset = 0.04;
    float TossYOffset = -0.07;
    float TakeOffD_1 = 2.5;
    float TakeOffD_2 = 1;
    float TossConf = 0.04;
    float ReturnConf = 2.0;
    bool ctl_update0X02 = false;
    bool ctl_update0X06 = true;
    void offset(double x,double y,double z,double w,char status);
    void UpdateHeight();
    void Updatemeter();
    bool initopen1=true;
    bool initopen2=false;
    void Update0x06();

};


#endif // PIPELINEDATA_H
