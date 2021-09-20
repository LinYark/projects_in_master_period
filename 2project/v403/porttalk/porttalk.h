#ifndef PORTTALK_H
#define PORTTALK_H



#include"common.h"



class portCommunicateSource{
public:
    int UART0_Send(int fd, char *send_buf,int data_len);
    unsigned short UART0_Recv(int fd, char *rcv_buf,int data_len);
    int UART0_Init(int fd, int speed,int flow_ctrl,int databits,int stopbits,int parity);
    int UART0_Set(int fd,int speed,int flow_ctrl,int databits,int stopbits,int parity);
    void UART0_Close(int fd);
    int UART0_Open(int fd,char* port);
};


class portCommunicate
{
public:
    void recordSelfPortInf(int num,string pathName);
    portRecieveInf getCurrenInf(){
        portRecieveInf portInf = *(this->portInf);
        return portInf;
    }
    void myinit(){    //set portID
        bool errorFlg;
        portCommunicateSource portFunc;
        portID = open("/dev/ttyTHS2",O_RDWR|O_NOCTTY|O_NDELAY);
        do{
            errorFlg = portFunc.UART0_Init(portID,115200,0,8,1,'N');
            printf("Set Port Exactly!\n");
        }while(-1 == errorFlg || -1 == portID);


        thread t1(updateHide);
        t1.detach();
    }

    bool recieveFlg = false;



private:


public:
    static bool initFlg ;
private:
    static void updateHide();
    static void copyPortInformation(char ucData[],unsigned short Length);

    static int portID;
    static portRecieveInf *portInf;

};

#endif // PORTTALK_H
