#include "pipeline/pipelinedata.h"
//宏定义
#define FALSE  -1
#define TRUE   0

void pipelineData::recordairAttitude(airAttitude SairAttitude,int num,char* pathName)
{
    ofstream fout(pathName,std::ios::app);
//    cout<<pathName<<endl;

    if (fout)
    {
//        cout<<"successfully!";
        fout<<num<<std::endl;
        fout<<std::setiosflags(std::ios::fixed);
        fout<<std::setprecision(15)<<SairAttitude.aTime<<std::endl;
        fout<<std::setprecision(15)<<SairAttitude.Lon<<std::endl;
        fout<<std::setprecision(15)<<SairAttitude.Lat<<std::endl;
        fout<<std::setprecision(15)<<SairAttitude.Height<<std::endl;
        fout<<std::setprecision(15)<<SairAttitude.Roll<<std::endl;
        fout<<std::setprecision(15)<<SairAttitude.Pitch<<std::endl;
        fout<<std::setprecision(15)<<SairAttitude.Yaw<<std::endl;
        fout<<std::setprecision(15)<<SairAttitude.hLidar<<std::endl<<std::endl;
    }
}

long pipelineData::getCurrentTime()
{
   struct timeval tv;
   gettimeofday(&tv,NULL);
   return tv.tv_sec * 1000 + tv.tv_usec / 1000;
}
//*******************************************************************
void pipelineData::printairAttitude(airAttitude SairAttitude)
{
    std::cout<<std::setiosflags(std::ios::fixed);
    std::cout<<std::setprecision(15)<<SairAttitude.aTime<<std::endl;
    std::cout<<std::setprecision(15)<<SairAttitude.Lon<<std::endl;
    std::cout<<std::setprecision(15)<<SairAttitude.Lat<<std::endl;
    std::cout<<std::setprecision(15)<<SairAttitude.Height<<" m"<<std::endl;
    std::cout<<std::setprecision(15)<<SairAttitude.Roll<<" degree"<<std::endl;
    std::cout<<std::setprecision(15)<<SairAttitude.Pitch<<" degree"<<std::endl;
    std::cout<<std::setprecision(15)<<SairAttitude.Yaw<<" degree"<<std::endl;
    std::cout<<std::setprecision(15)<<SairAttitude.hLidar<<" m"<<std::endl;
} //打印出无人机当前状态量的函数

//============================ttyTHS2===============================
/*******************************************************************
* 名称：                  UART0_Open
* 功能：                打开串口并返回串口
*
* 设备文件描述
* 入口参数：        fd    :文件描述符     port :串口号(ttyS0,ttyS1,ttyS2)
* 出口参数：        正确返回为1，错误返回为0
*******************************************************************/
int pipelineData::UART0_Open(int fd,char* port)
{

         fd = open( port, O_RDWR|O_NOCTTY|O_NDELAY);
         if (FALSE == fd)
                {
                       perror("Can't Open Serial Port");
                       return(FALSE);
                }
     //恢复串口为阻塞状态
     if(fcntl(fd, F_SETFL, 0) < 0)
                {
                       printf("fcntl failed!\n");
                     return(FALSE);
                }
         else
                {
                  printf("fcntl=%d\n",fcntl(fd, F_SETFL,0));
                }
      //测试是否为终端设备
      if(0 == isatty(STDIN_FILENO))
                {
                       printf("standard input is not a terminal device\n");
                  return(FALSE);
                }
  else
                {
                     printf("isatty success!\n");
                }
  printf("fd->open=%d\n",fd);
  return fd;
}

/*******************************************************************
* 名称：                UART0_Close
* 功能：                关闭串口并返回串口设备文件描述
* 入口参数：        fd    :文件描述符     port :串口号(ttyS0,ttyS1,ttyS2)
* 出口参数：        void
*******************************************************************/

void pipelineData::UART0_Close(int fd)
{
    close(fd);
}

/*******************************************************************
* 名称：                UART0_Set
* 功能：                设置串口数据位，停止位和效验位
* 入口参数：        fd        串口文件描述符
*                              speed     串口速度
*                              flow_ctrl   数据流控制
*                           databits   数据位   取值为 7 或者8
*                           stopbits   停止位   取值为 1 或者2
*                           parity     效验类型 取值为N,E,O,,S
*出口参数：          正确返回为1，错误返回为0
*******************************************************************/
int pipelineData::UART0_Set(int fd,int speed,int flow_ctrl,int databits,int stopbits,int parity)
{

      int   i;
      int   status;
      int   speed_arr[] = { B460800, B230400, B115200, B19200, B9600, B4800, B2400, B1200, B300};
      int   name_arr[] = { 460800, 230400, 115200,  19200,  9600,  4800,  2400,  1200,  300};

      struct termios options;

    /*tcgetattr(fd,&options)得到与fd指向对象的相关参数，并将它们保存于options,该函数还可以测试配置是否正确，该串口是否可用等。若调用成功，函数返回值为0，若调用失败，函数返回值为1.
    */
    if  ( tcgetattr( fd,&options)  !=  0)
       {
          perror("SetupSerial 1");
          return(FALSE);
       }

    //设置串口输入波特率和输出波特率
    for ( i= 0;  i < sizeof(speed_arr) / sizeof(int);  i++)
                {
                     if  (speed == name_arr[i])
                            {
                                 cfsetispeed(&options, speed_arr[i]);
                                 cfsetospeed(&options, speed_arr[i]);
                            }
              }

    //修改控制模式，保证程序不会占用串口
    options.c_cflag |= CLOCAL;
    //修改控制模式，使得能够从串口中读取输入数据
    options.c_cflag |= CREAD;

    //设置数据流控制
    switch(flow_ctrl)
    {

       case 0 ://不使用流控制
              options.c_cflag &= ~CRTSCTS;
              break;

       case 1 ://使用硬件流控制
              options.c_cflag |= CRTSCTS;
              break;
       case 2 ://使用软件流控制
              options.c_cflag |= IXON | IXOFF | IXANY;
              break;
    }
    //设置数据位
    //屏蔽其他标志位
    options.c_cflag &= ~CSIZE;
    switch (databits)
    {
       case 5    :
                     options.c_cflag |= CS5;
                     break;
       case 6    :
                     options.c_cflag |= CS6;
                     break;
       case 7    :
                 options.c_cflag |= CS7;
                 break;
       case 8:
                 options.c_cflag |= CS8;
                 break;
       default:
                 fprintf(stderr,"Unsupported data size\n");
                 return (FALSE);
    }
    //设置校验位
    switch (parity)
    {
       case 'n':
       case 'N': //无奇偶校验位。
                 options.c_cflag &= ~PARENB;
                 options.c_iflag &= ~(INPCK|ICRNL|IXON);
                 break;
       case 'o':
       case 'O'://设置为奇校验
                 options.c_cflag |= (PARODD | PARENB);
                 options.c_iflag |= INPCK;
                 break;
       case 'e':
       case 'E'://设置为偶校验
                 options.c_cflag |= PARENB;
                 options.c_cflag &= ~PARODD;
                 options.c_iflag |= INPCK;
                 break;
       case 's':
       case 'S': //设置为空格
                 options.c_cflag &= ~PARENB;
                 options.c_cflag &= ~CSTOPB;
                 break;
        default:
                 fprintf(stderr,"Unsupported parity\n");
                 return (FALSE);
    }
    // 设置停止位
    switch (stopbits)
    {
       case 1:
                 options.c_cflag &= ~CSTOPB; break;
       case 2:
                 options.c_cflag |= CSTOPB; break;
       default:
                       fprintf(stderr,"Unsupported stop bits\n");
                       return (FALSE);
    }

  //修改输出模式，原始数据输出
  options.c_oflag &= ~OPOST;

  options.c_lflag &= ~(ICANON | ECHO | ECHOE | ISIG);//我加的
//options.c_lflag &= ~(ISIG | ICANON);

    //设置等待时间和最小接收字符
    options.c_cc[VTIME] = 1; /* 读取一个字符等待1*(1/10)s */
    options.c_cc[VMIN] = 1; /* 读取字符的最少个数为1 */

    //如果发生数据溢出，接收数据，但是不再读取 刷新收到的数据但是不读
    tcflush(fd,TCIFLUSH);

    //激活配置 (将修改后的termios数据设置到串口中）
    if (tcsetattr(fd,TCSANOW,&options) != 0)
           {
               perror("com set error!\n");
              return (FALSE);
           }
    return (TRUE);
}

/*******************************************************************
* 名称：                UART0_Init()
* 功能：                串口初始化
* 入口参数：        fd       :  文件描述符
*               speed  :  串口速度
*                              flow_ctrl  数据流控制
*               databits   数据位   取值为 7 或者8
*                           stopbits   停止位   取值为 1 或者2
*                           parity     效验类型 取值为N,E,O,,S
*
* 出口参数：        正确返回为1，错误返回为0
*******************************************************************/
int pipelineData::UART0_Init(int fd, int speed,int flow_ctrl,int databits,int stopbits,int parity)
{
    int err;
    //设置串口数据帧格式
    if (UART0_Set(fd,115200,0,8,1,'N') == FALSE)
       {
        return FALSE;
       }
    else
       {
               return  TRUE;
        }
}

/*******************************************************************
* 名称：                  UART0_Recv
* 功能：                接收串口数据
* 入口参数：        fd                  :文件描述符
*                              rcv_buf     :接收串口中数据存入rcv_buf缓冲区中
*                              data_len    :一帧数据的长度
* 出口参数：        正确返回为1，错误返回为0
*******************************************************************/
unsigned short pipelineData::UART0_Recv(int fd, char *rcv_buf,int data_len)
{
    unsigned short usLength,fs_sel;
    fd_set fs_read;

    struct timeval time;

    FD_ZERO(&fs_read);
    FD_SET(fd,&fs_read);

    time.tv_sec = 500/1000;
    time.tv_usec = 500 % 1000*1000;



    //使用select实现串口的多路通信
    fs_sel = select(fd+1,&fs_read,NULL,NULL,&time);
    if(fs_sel)
       {
              usLength = read(fd,rcv_buf,data_len);
          //printf("I am right!(version4) usLength = %d fs_sel = %d\n",usLength,fs_sel);

              return usLength;
       }
    else
       {
//          printf("No new data!\n");
              return FALSE;
       }
}

/********************************************************************
* 名称：                  UART0_Send
* 功能：                发送数据
* 入口参数：        fd                  :文件描述符
*                              send_buf    :存放串口发送数据
*                              data_len    :一帧数据的个数
* 出口参数：        正确返回为1，错误返回为0
*******************************************************************/
int pipelineData::UART0_Send(int fd, char *send_buf,int data_len)
{
    int len = 0;

    len = write(fd,send_buf,data_len);
    if (len == data_len )
              {
                     return len;
              }
    else
        {

                tcflush(fd,TCOFLUSH);
                return FALSE;
        }

}



void pipelineData::coutRec(unsigned char* c, int num, ofstream& of) {
    for(int i=0;i<num;i++) {
        cout<<setfill('0')<<setw(2)<<hex<<(unsigned int)(unsigned char)c[i]<<" ";
        of<<setfill('0')<<setw(2)<<hex<<(unsigned int)(unsigned char)c[i]<<" ";
        of.flush();
    }
    cout<<dec<<endl;
    of<<dec<<endl;
    of.flush();
}

void pipelineData::coutRecC(char* c, int num, ofstream& of) {
    for(int i=0;i<num;i++) {
        cout<<setfill('0')<<setw(2)<<hex<<(unsigned int)(unsigned char)c[i]<<" ";
        of<<setfill('0')<<setw(2)<<hex<<(unsigned int)(unsigned char)c[i]<<" ";
        of.flush();
    }
    cout<<dec<<endl;
    of<<dec<<endl;
    of.flush();
}

char pipelineData::countCRC(char sbuf[], int num)
{
//    static char k[200];
    char RCRC=0x00;
    for(int i=2;i<num;i++)
    {
        RCRC=RCRC+sbuf[i];
    }
    return RCRC;
}

void pipelineData::countCRCRec(unsigned char *c, int num)
{
    char RCRC=0x00;
    for(int i=2;i<num-1;i++) {
        RCRC=RCRC+c[i];
    }
    if(RCRC!=c[num-1]) {
        this->ErrorCRC++;
        ofs_a<<"CRC Error!!!"<<endl;
        ofs_a.flush();
    }
}

void pipelineData ::CopeSerialData(char ucData[],unsigned short usLength)
{
    unsigned char chrTemp[2000];
    unsigned short usRxLength = 0;
    std::cout<<std::setiosflags(std::ios::fixed);
    memcpy(chrTemp,ucData,usLength);
    usRxLength += usLength;
    while (usRxLength > 0)
    {
        if (chrTemp[0] !=0xA5 || chrTemp[1] !=0x5A)
        {
            usRxLength--;
            memcpy(&chrTemp[0],&chrTemp[1],usRxLength);
            continue;
        }
        if (chrTemp[2] ==0x00)
        {
            printf("SDK to tx2...\n");
            ofs_a<<"SDK to tx2...\n";
            ofs_a.flush();
        }
        else if (chrTemp[2] ==0x01)
        {
            printf("tx2 to SDK...\n");
            ofs_a<<"tx2 to SDK...\n";
            ofs_a.flush();
        }
        else
        {
            printf("Error! ID error!\n");
            ofs_a<<"Error! ID error!\n";
            ofs_a.flush();
            this->ErrorRec++;
            this->ErrorCRC++;
        }
        if (chrTemp[3] !=0x13)
        {
            printf("Error! Protocol error!\n");
            ofs_a<<"Error! Protocol error!\n";
            ofs_a.flush();
            this->coutRec(chrTemp,usRxLength,ofs_r);
            this->ErrorRec++;
            this->ErrorCRC++;
            usRxLength = 0;
            continue;
        }
        printf("%x ",chrTemp[0]);
        printf("%x ",chrTemp[1]);
        printf("%x ",chrTemp[2]);
        printf("%x ",chrTemp[3]);
        printf("%x\n",chrTemp[4]);
        switch(chrTemp[4])
        {
        case 0x01:
            if(chrTemp[5]==0x00 ) {
                this->ID=0x01;
                this->coutRec(chrTemp,8,ofs_r);
                usRxLength=0;
            }
            else {
                usRxLength=0;
            }
            break;
        case 0x02:
            this->coutRec(chrTemp,8,ofs_r);
            usRxLength=0;
            break;
        case 0x04:
            if(this->ID==0x02) {
                this->Update0x04(chrTemp);
                memcpy(&this->SairAttitude.aTime,&chrTemp[7],4);
                memcpy(&this->SairAttitude.oLidar,&chrTemp[45],2);
                memcpy(&this->SairAttitude.CRC,&chrTemp[48],1);
                this->coutRec(chrTemp,49,ofs_r);
                this->countCRCRec(chrTemp,49);
//                std::cout<<std::setprecision(15)<<this->SairAttitude.aTime<<std::endl;
//                std::cout<<std::setprecision(15)<<this->SairAttitude.Lon<<std::endl;
//                std::cout<<std::setprecision(15)<<this->SairAttitude.Lat<<std::endl;
//                std::cout<<std::setprecision(15)<<this->SairAttitude.Height<<" m"<<std::endl;
//                std::cout<<std::setprecision(15)<<this->SairAttitude.Roll<<" degree"<<std::endl;
//                std::cout<<std::setprecision(15)<<this->SairAttitude.Pitch<<" degree"<<std::endl;
//                std::cout<<std::setprecision(15)<<this->SairAttitude.Yaw<<" degree"<<std::endl;
//                std::cout<<std::setprecision(15)<<this->SairAttitude.hLidar<<" m"<<std::endl;
                ofs_a<<std::setprecision(15)<<this->SairAttitude.aTime<<std::endl;
                ofs_a<<std::setprecision(15)<<this->SairAttitude.Lon<<std::endl;
                ofs_a<<std::setprecision(15)<<this->SairAttitude.Lat<<std::endl;
                ofs_a<<std::setprecision(15)<<this->SairAttitude.Height<<" m"<<std::endl;
                ofs_a<<std::setprecision(15)<<this->SairAttitude.Roll<<" degree"<<std::endl;
                ofs_a<<std::setprecision(15)<<this->SairAttitude.Pitch<<" degree"<<std::endl;
                ofs_a<<std::setprecision(15)<<this->SairAttitude.Yaw<<" degree"<<std::endl;
                ofs_a<<std::setprecision(15)<<this->SairAttitude.hLidar<<" m"<<std::endl;
                this->t.fh=100;
                this->t.l=this->SairAttitude.Lon;
                this->t.m=this->SairAttitude.Lat;
                this->t.h=this->SairAttitude.Height+this->t.fh;
                this->t.gr=this->SairAttitude.Roll;
                this->t.gp=this->SairAttitude.Pitch-90;
                this->t.gy=this->SairAttitude.Yaw;
//                this->t.hLidar=this->SairAttitude.hLidar/100;
                this->t.hLidar=this->SairAttitude.Height;
                this->SDetResult.uavparam.push_back(this->t);
                usRxLength=0;
                if(this->SairAttitude.Height>6) {
                    this->SendFlag=29;
                }
                else {
                    this->SendFlag=4;
                }
            }
            else {
                if(usRxLength>=49) {
                    this->Update0x04(chrTemp);
                    memcpy(&this->SairAttitude.aTime,&chrTemp[7],4);
                    memcpy(&this->SairAttitude.oLidar,&chrTemp[45],2);
                    memcpy(&this->SairAttitude.CRC,&chrTemp[48],1);
                    this->coutRec(chrTemp,49,ofs_r);
                    this->countCRCRec(chrTemp,49);
//                    std::cout<<std::setprecision(15)<<this->SairAttitude.aTime<<std::endl;
//                    std::cout<<std::setprecision(15)<<this->SairAttitude.Lon<<std::endl;
//                    std::cout<<std::setprecision(15)<<this->SairAttitude.Lat<<std::endl;
//                    std::cout<<std::setprecision(15)<<this->SairAttitude.Height<<" m"<<std::endl;
//                    std::cout<<std::setprecision(15)<<this->SairAttitude.Roll<<" degree"<<std::endl;
//                    std::cout<<std::setprecision(15)<<this->SairAttitude.Pitch<<" degree"<<std::endl;
//                    std::cout<<std::setprecision(15)<<this->SairAttitude.Yaw<<" degree"<<std::endl;
//                    std::cout<<std::setprecision(15)<<this->SairAttitude.hLidar<<" m"<<std::endl;
                    ofs_a<<std::setprecision(15)<<this->SairAttitude.aTime<<std::endl;
                    ofs_a<<std::setprecision(15)<<this->SairAttitude.Lon<<std::endl;
                    ofs_a<<std::setprecision(15)<<this->SairAttitude.Lat<<std::endl;
                    ofs_a<<std::setprecision(15)<<this->SairAttitude.Height<<" m"<<std::endl;
                    ofs_a<<std::setprecision(15)<<this->SairAttitude.Roll<<" degree"<<std::endl;
                    ofs_a<<std::setprecision(15)<<this->SairAttitude.Pitch<<" degree"<<std::endl;
                    ofs_a<<std::setprecision(15)<<this->SairAttitude.Yaw<<" degree"<<std::endl;
                    ofs_a<<std::setprecision(15)<<this->SairAttitude.hLidar<<" m"<<std::endl;
                    usRxLength=usRxLength-49;
                    memcpy(&chrTemp[0],&chrTemp[49],usRxLength);
                }
                else {
                    usRxLength=0;
                }
            }
            break;
        case 0x05:
            memcpy(&this->SairResult.status,&chrTemp[7],1);
            memcpy(&this->SairResult.Xcoord,&chrTemp[8],4);
            memcpy(&this->SairResult.Ycoord,&chrTemp[12],4);
            memcpy(&this->SairResult.CRC,&chrTemp[13],1);
            usRxLength=usRxLength-14;
            memcpy(&chrTemp[0],&chrTemp[14],usRxLength);
            break;
        case 0x06:
            memcpy(&this->SfControl.fTime,&chrTemp[7],4);
            memcpy(&this->SfControl.Coord,&chrTemp[11],1);
            memcpy(&this->SfControl.ControlMode,&chrTemp[12],1);
            memcpy(&this->SfControl.OffsetX,&chrTemp[13],4);
            memcpy(&this->SfControl.OffsetY,&chrTemp[17],4);
            memcpy(&this->SfControl.OffsetZ,&chrTemp[21],4);
            memcpy(&this->SfControl.OffsetW,&chrTemp[25],4);
            memcpy(&this->SfControl.CRC,&chrTemp[29],1);
            usRxLength=usRxLength-30;
            memcpy(&chrTemp[0],&chrTemp[30],usRxLength);
            break;
        case 0x07:
//            memcpy(&this->SToss.numPipe,&chrTemp[7],1);
//            memcpy(&this->SToss.tossStatus,&chrTemp[8],1);
//            memcpy(&this->SToss.CRC,&chrTemp[9],1);
            memcpy(&this->SToss.tossSuc,&chrTemp[7],1);
            this->coutRec(chrTemp,9,ofs_r);
            usRxLength = usRxLength - 9;
            break;

        default:
            this->coutRec(chrTemp,usRxLength,ofs_r);
            usRxLength=0;
            printf("Unknow char!\n");
            ofs_a<<"Unknow char!\n";
            ofs_a.flush();
            break;
        }
    }
}

void pipelineData::Update0x04(unsigned char* chrTemp){
    memcpy(&this->SairAttitude.LonN,&chrTemp[11],8);
    memcpy(&this->SairAttitude.LatN,&chrTemp[19],8);
    memcpy(&this->SairAttitude.HeightN,&chrTemp[27],4);
    memcpy(&this->SairAttitude.RollN,&chrTemp[31],4);
    memcpy(&this->SairAttitude.PitchN,&chrTemp[35],4);
    memcpy(&this->SairAttitude.YawN,&chrTemp[39],4);
    memcpy(&this->SairAttitude.hLidarN,&chrTemp[43],2);
    memcpy(&this->SairAttitude.statusN,&chrTemp[47],1);
    if(this->SairAttitude.Assignment) {
        if(abs(this->SairAttitude.LonN - this->SairAttitude.LonO)<0.001) {
            memcpy(&this->SairAttitude.Lon,&this->SairAttitude.LonN,8);
            memcpy(&this->SairAttitude.LonO,&this->SairAttitude.LonN,8);
            this->SairAttitude.K1 = 3;
        }
        else {
            memcpy(&this->SairAttitude.Lon,&this->SairAttitude.LonO,8);
            this->SairAttitude.K1--;
            this->ErrorRec++;
        }

        if(abs(this->SairAttitude.LatN - this->SairAttitude.LatO)<0.001) {
            memcpy(&this->SairAttitude.Lat,&this->SairAttitude.LatN,8);
            memcpy(&this->SairAttitude.LatO,&this->SairAttitude.LatN,8);
            this->SairAttitude.K2 = 3;
        }
        else {
            memcpy(&this->SairAttitude.Lat,&this->SairAttitude.LatO,8);
            this->SairAttitude.K2--;
            this->ErrorRec++;
        }

        if(abs(this->SairAttitude.HeightN - this->SairAttitude.HeightO)<1) {
            memcpy(&this->SairAttitude.Height,&this->SairAttitude.HeightN,4);
            memcpy(&this->SairAttitude.HeightO,&this->SairAttitude.HeightN,4);
            this->SairAttitude.K3 = 3;
        }
        else {
            memcpy(&this->SairAttitude.Height,&this->SairAttitude.HeightO,4);
            this->SairAttitude.K3--;
            this->ErrorRec++;
        }

        if(abs(this->SairAttitude.RollN - this->SairAttitude.RollO) < 20 && this->SairAttitude.RollN<180 && this->SairAttitude.RollN>-180) {
            memcpy(&this->SairAttitude.Roll,&this->SairAttitude.RollN,4);
            memcpy(&this->SairAttitude.RollO,&this->SairAttitude.RollN,4);
            this->SairAttitude.K4 = 3;
        }
        else {
            memcpy(&this->SairAttitude.Roll,&this->SairAttitude.RollO,4);
            this->SairAttitude.K4--;
            this->ErrorRec++;
        }

        if(abs(this->SairAttitude.PitchN - this->SairAttitude.PitchO) < 20 && this->SairAttitude.PitchN<180 && this->SairAttitude.PitchN>-180) {
            memcpy(&this->SairAttitude.Pitch,&this->SairAttitude.PitchN,4);
            memcpy(&this->SairAttitude.PitchO,&this->SairAttitude.PitchN,4);
            this->SairAttitude.K5 = 3;
        }
        else {
            memcpy(&this->SairAttitude.Pitch,&this->SairAttitude.PitchO,4);
            this->SairAttitude.K5--;
            this->ErrorRec++;
        }

        if(abs(this->SairAttitude.YawN - this->SairAttitude.YawO) < 20 && this->SairAttitude.YawN<180 && this->SairAttitude.YawN>-180) {
            memcpy(&this->SairAttitude.Yaw,&this->SairAttitude.YawN,4);
            memcpy(&this->SairAttitude.YawO,&this->SairAttitude.YawN,4);
            this->SairAttitude.K6 = 3;
        }
        else {
            memcpy(&this->SairAttitude.Yaw,&this->SairAttitude.YawO,4);
            this->SairAttitude.K6--;
            this->ErrorRec++;
        }

        if(abs(this->SairAttitude.hLidarN - this->SairAttitude.hLidarO)<100 && this->SairAttitude.hLidarN>0) {
            memcpy(&this->SairAttitude.hLidar,&this->SairAttitude.hLidarN,2);
            memcpy(&this->SairAttitude.hLidarO,&this->SairAttitude.hLidarN,2);
            this->SairAttitude.K7 = 3;
        }
        else {
            memcpy(&this->SairAttitude.hLidarO,&this->SairAttitude.hLidarN,2);
            this->SairAttitude.K7--;
            this->ErrorRec++;
        }

        if(this->SairAttitude.statusN==0x01 || this->SairAttitude.statusN==0x00) {
            memcpy(&this->SairAttitude.status,&this->SairAttitude.statusN,1);
            memcpy(&this->SairAttitude.statusO,&this->SairAttitude.statusN,1);
            this->SairAttitude.K8 = 3;
        }
        else {
            memcpy(&this->SairAttitude.status,&this->SairAttitude.statusO,1);
            this->SairAttitude.K8--;
            this->ErrorRec++;
        }

        if(this->SairAttitude.K1<=0) {
            memcpy(&this->SairAttitude.Lon,&chrTemp[11],8);
            memcpy(&this->SairAttitude.LonO,&chrTemp[11],8);
            this->SairAttitude.K1 = 3;
        }
        if(this->SairAttitude.K2<=0) {
            memcpy(&this->SairAttitude.Lat,&chrTemp[19],8);
            memcpy(&this->SairAttitude.LatO,&chrTemp[19],8);
            this->SairAttitude.K2 = 3;
        }
        if(this->SairAttitude.K3<=0) {
            memcpy(&this->SairAttitude.Height,&chrTemp[27],4);
            memcpy(&this->SairAttitude.HeightO,&chrTemp[27],4);
            this->SairAttitude.K3 = 3;
        }
        if(this->SairAttitude.K4<=0) {
            memcpy(&this->SairAttitude.Roll,&chrTemp[31],4);
            memcpy(&this->SairAttitude.RollO,&chrTemp[31],4);
            this->SairAttitude.K4 = 3;
        }
        if(this->SairAttitude.K5<=0) {
            memcpy(&this->SairAttitude.Pitch,&chrTemp[35],4);
            memcpy(&this->SairAttitude.PitchO,&chrTemp[35],4);
            this->SairAttitude.K5 = 3;
        }
        if(this->SairAttitude.K6<=0) {
            memcpy(&this->SairAttitude.Yaw,&chrTemp[39],4);
            memcpy(&this->SairAttitude.YawO,&chrTemp[39],4);
            this->SairAttitude.K6 = 3;
        }
        if(this->SairAttitude.K7<=0) {
            memcpy(&this->SairAttitude.hLidar,&chrTemp[43],2);
            memcpy(&this->SairAttitude.hLidarO,&chrTemp[43],2);
            this->SairAttitude.K7 = 3;
        }
        if(this->SairAttitude.K8<=0) {
            memcpy(&this->SairAttitude.status,&chrTemp[47],1);
            memcpy(&this->SairAttitude.statusO,&chrTemp[47],1);
            this->SairAttitude.K8 = 3;
        }
    }
    else {
        memcpy(&this->SairAttitude.Lon,&chrTemp[11],8);
        memcpy(&this->SairAttitude.LonO,&chrTemp[11],8);
        memcpy(&this->SairAttitude.Lat,&chrTemp[19],8);
        memcpy(&this->SairAttitude.LatO,&chrTemp[19],8);
        memcpy(&this->SairAttitude.Height,&chrTemp[27],4);
        memcpy(&this->SairAttitude.HeightO,&chrTemp[27],4);
        memcpy(&this->SairAttitude.Roll,&chrTemp[31],4);
        memcpy(&this->SairAttitude.RollO,&chrTemp[31],4);
        memcpy(&this->SairAttitude.Pitch,&chrTemp[35],4);
        memcpy(&this->SairAttitude.PitchO,&chrTemp[35],4);
        memcpy(&this->SairAttitude.Yaw,&chrTemp[39],4);
        memcpy(&this->SairAttitude.YawO,&chrTemp[39],4);
        memcpy(&this->SairAttitude.hLidar,&chrTemp[43],2);
        memcpy(&this->SairAttitude.hLidarO,&chrTemp[43],2);
        memcpy(&this->SairAttitude.status,&chrTemp[47],1);
        memcpy(&this->SairAttitude.statusO,&chrTemp[47],1);
        this->SairAttitude.Assignment = true;
    }
}

void pipelineData::Send0x01()
{
    char sendbuf[9];
    sendbuf[0]=0xA5;
    sendbuf[1]=0x5A;
    sendbuf[2]=0x01;
    sendbuf[3]=0x13;
    sendbuf[4]=0x01;
    sendbuf[5]=0x01;
    sendbuf[6]=0x00;
    printf("11\n");
    ofs_a<<"11\n";
    ofs_a.flush();

//    if(this->VisualInital)
//    {
    sendbuf[7]=0x0F;
//    }
//    else {
//        sendbuf[7]=0xF0;
//    }
    sendbuf[8]=this->countCRC(sendbuf,8);
    this->len=UART0_Send(this->fd,sendbuf,9);
    this->coutRecC(sendbuf,9,ofs_s);
}
      //机载视觉启动成功

void pipelineData::offset(double x,double y,double z,double w,char status){
    this->SairResult.status=status;
    this->SfControl.OffsetX=x;
    this->SfControl.OffsetY=y;
    this->SfControl.OffsetZ=z;
    this->SfControl.OffsetW=w;}
void pipelineData::UpdateHeight(){
    double Dvalue=0;
    if(this->SairAttitude.Height<=60){
        Dvalue = abs(this->SairAttitude.Height - 60);
        if(Dvalue>=1){
            offset(0,0,this->SairAttitude.Height+1,0,0x01);
            QThread::msleep(1);
        }
        else{
            this->initopen1=false;
            this->initopen2=true;
        }
    }
    else{
        Dvalue = abs(this->SairAttitude.Height - 60);
        if(Dvalue>=1){
            offset(0,0,this->SairAttitude.Height-1,0,0x01);
            QThread::msleep(1);
        }
        else{
            this->initopen1=false;
            this->initopen2=true;
        }
    }
}

void pipelineData::Update0x06()
{
    double h1,h2,h3,h4,h5;

    if(this->initopen1){
        UpdateHeight();
    }

    if(this->initopen2){
        static int Dsum=0;
        this->T = false;
        Dsum++;
        int model = Dsum / 100;
        cout<<"model"<<endl;
        if(this->SDetResult.TargetLoss == false){
            switch (model) {
            case 0:
                h1 =this->SairAttitude.Height;
                offset(0, this->SfControl.OffsetY+1,h1,0,0x01);
                QThread::msleep(1);
                break;
            case 1:
                h2 =this->SairAttitude.Height;
                offset(this->SfControl.OffsetX+1,0,h2,0,0x01);
                QThread::msleep(1);
                break;

            case 2:
                h3 =this->SairAttitude.Height;
                offset(0,this->SfControl.OffsetY-1,h3,0,0x01);
                QThread::msleep(1);
                break;

            case 3:
                h4 =this->SairAttitude.Height;
                offset(this->SfControl.OffsetX-1,0,h4,0,0x01);
                QThread::msleep(1);
                break;

            case 4:
                h5 =this->SairAttitude.Height;
                if(h5>5){
                    offset(0,0,h5-0.5,0,0x01);
                }
                if(h5<5){
                    offset(0,0,h5+0.5,0,0x01);
                }
                break;
            default:
                QThread::msleep(1);
                break;
            }
        }
  }
}

void pipelineData::Update0x02()
{
//    double Y;
    this->T = false;
    if(this->SDetResult.TargetLoss) {
        if(this->SairAttitude.Height < this->TargetLossHeight) {
            this->SairResult.status=0x02;
            this->SfControl.OffsetX=0;
            this->SfControl.OffsetY=0;
            this->SfControl.OffsetZ=this->SairAttitude.Height+0.5;
            this->SfControl.OffsetW=0;
        }
        else {
            this->SairResult.status=0x02;
            this->SfControl.OffsetX=0;
            this->SfControl.OffsetY=0;
            this->SfControl.OffsetZ=this->SairAttitude.Height-1.5;
            this->SfControl.OffsetW=0;
        }
    } //高度小于六米的时候才会进if
    else
    {
        cout<<"Update X: "<<this->SDetResult.loc.at<double>(0, 0)<<endl;
        ofs_a<<"Update X: "<<this->SDetResult.loc.at<double>(0, 0)<<endl;
        ofs_a.flush();
        cout<<"Update Y: "<<this->SDetResult.loc.at<double>(1, 0)<<endl;
        ofs_a<<"Update Y: "<<this->SDetResult.loc.at<double>(1, 0)<<endl;
        ofs_a.flush();
        this->SairResult.status=0x01;
        this->SairResult.Xcoord=0;
        this->SairResult.Ycoord=0;
        if(this->SDetResult.loc.at<double>(0, 0)<-0.5)
        {
            this->SfControl.OffsetX=-0.5;
        }
        else if(this->SDetResult.loc.at<double>(0, 0)>0.5)
        {
            this->SfControl.OffsetX=0.5;
        }
        else
        {
            this->SfControl.OffsetX=this->SDetResult.loc.at<double>(0, 0) + this->TossXOffset;
        }
        cout<<"X: "<<this->SfControl.OffsetX<<" "<<this->SfControl.OffsetY<<endl;
        ofs_a<<"X: "<<this->SfControl.OffsetX<<" "<<this->SfControl.OffsetY<<endl;
        ofs_a.flush();

        if(this->SDetResult.loc.at<double>(1, 0)<-0.5)
        {
            this->SfControl.OffsetY=-0.5;
        }
        else if(this->SDetResult.loc.at<double>(1, 0)>0.5)
        {
            this->SfControl.OffsetY=0.5;
        }
        else
        {

            if(this->SairAttitude.Height<7) {
                this->SfControl.OffsetY=this->SDetResult.loc.at<double>(1, 0) + this->TossYOffset;
            }
            else if(this->SairAttitude.Height<8 && this->SairAttitude.Height>7) {
                float shift = abs( this->SairAttitude.Height - 8)*this->TossYOffset;
                this->SfControl.OffsetY=this->SDetResult.loc.at<double>(1, 0) + shift;
            }
            else {
                this->SfControl.OffsetY=this->SDetResult.loc.at<double>(1, 0);
            }
        }
        cout<<"X: "<<this->SfControl.OffsetX<<" "<<this->SfControl.OffsetY<<endl;
        ofs_a<<"X: "<<this->SfControl.OffsetX<<" "<<this->SfControl.OffsetY<<endl;
        ofs_a.flush();

        if(this->SairAttitude.Height>10)
        {
            this->SfControl.OffsetZ=this->SairAttitude.Height - TakeOffD_1;
        }
        else
        {
            this->SfControl.OffsetZ=this->SairAttitude.Height - TakeOffD_2;
        }
        std::cout<<"Update\n";
        ofs_a<<"Update\n";
        ofs_a.flush();

        this->SfControl.OffsetW=0;
        if(this->SfControl.OffsetX*this->SfControl.OffsetY==0) {
            this->T = false;
        }
        else {
            cout<<"Unlock\n";
            ofs_a<<"Unlock\n";
            ofs_a.flush();
            cout<<"X: "<<this->SfControl.OffsetX<<" "<<this->SfControl.OffsetY<<endl;
            ofs_a<<"X: "<<this->SfControl.OffsetX<<" "<<this->SfControl.OffsetY<<endl;
            ofs_a.flush();
            double q = this->SfControl.OffsetX ;
            double w = this->SfControl.OffsetY ;
            this->Toss = sqrt(q*q + w*w);
            cout<<"TTTTTTT"<<this->Toss<<endl;
            ofs_a<<"TTTTTTT\t"<<this->Toss<<endl;
            ofs_a.flush();
            this->T = true;
        }
    }
}

void pipelineData::Reset(float k)
{
    float i = 0;
    float A;
    if(k>1){
        A = this->SairAttitude.Yaw*(k-1)/k;
    }
    else if(k==1) {
        A = 0;
    }
    else {
        A = this->SairAttitude.Yaw;
    }

    cout<<"Reset Yaw: "<<k<<endl;
    ofs_a<<"Reset Yaw: "<<k<<endl;
    ofs_a.flush();
    cout<<"Yaw: "<<this->SairAttitude.Yaw<<" N:"<<A<<endl;
    ofs_a<<"Yaw: "<<this->SairAttitude.Yaw<<" N:"<<A<<endl;
    ofs_a.flush();
    char controlbuf[30];
    controlbuf[0]=0xA5;
    controlbuf[1]=0x5A;
    controlbuf[2]=0x01;
    controlbuf[3]=0x13;
    controlbuf[4]=0x06;
    controlbuf[5]=0x16;
    controlbuf[6]=0x00;
    memcpy(&controlbuf[7],&this->sendKey,4);
    controlbuf[11]=0x00;
    controlbuf[12]=0x01;
    memcpy(&controlbuf[13],&i,4);
    memcpy(&controlbuf[17],&i,4);
    memcpy(&controlbuf[21],&this->SairAttitude.Height,4);
    memcpy(&controlbuf[25],&A,4);
    controlbuf[29]=this->countCRC(controlbuf,29);
    this->len=UART0_Send(this->fd,controlbuf,30);
    this->coutRecC(controlbuf,30,ofs_s);
    this->sendKey=this->sendKey+1;
} //恢复原位，调整姿态

void pipelineData::SendStop() {
    float i = 0;
    char controlbuf[30];
    controlbuf[0]=0xA5;
    controlbuf[1]=0x5A;
    controlbuf[2]=0x01;
    controlbuf[3]=0x13;
    controlbuf[4]=0x06;
    controlbuf[5]=0x16;
    controlbuf[6]=0x00;
    memcpy(&controlbuf[7],&this->sendKey,4);
    controlbuf[11]=0x00;
    controlbuf[12]=0x00;
    memcpy(&controlbuf[13],&i,4);
    memcpy(&controlbuf[17],&i,4);
    memcpy(&controlbuf[21],&this->SairAttitude.Height,4);
    memcpy(&controlbuf[25],&this->SairAttitude.Yaw,4);
    controlbuf[29]=this->countCRC(controlbuf,29);
    this->len=UART0_Send(this->fd,controlbuf,30);
    this->coutRecC(controlbuf,30,ofs_s);
    this->sendKey=this->sendKey+1;
} //悬停

void pipelineData::Send0x05(){
    char cambuf[17];
    cambuf[0]=0xA5;
    cambuf[1]=0X5A;
    cambuf[2]=0x01;
    cambuf[3]=0x13;
    cambuf[4]=0x05;
    cambuf[5]=0x09;
    cambuf[6]=0x00;
    this->SairResult.status=0x01;
    this->SairResult.Xcoord=0;
    this->SairResult.Ycoord=0;
    memcpy(&cambuf[7],&this->SairResult.status,1);
    memcpy(&cambuf[8],&this->SairResult.Xcoord,4);
    memcpy(&cambuf[12],&this->SairResult.Ycoord,4);
    cambuf[16]=this->countCRC(cambuf,16);
    this->len=UART0_Send(this->fd,cambuf,17);
    this->coutRecC(cambuf,17,ofs_s);
} //视觉控制系统目标检测结果输出

void pipelineData::Send0x02()
{
    char cambuf[17];
    char controlbuf[30];
    cambuf[0]=0xA5;
    cambuf[1]=0X5A;
    cambuf[2]=0x01;
    cambuf[3]=0x13;
    cambuf[4]=0x05;
    cambuf[5]=0x09;
    cambuf[6]=0x00;
    if(this->vDetect)
    {
        this->SairResult.status=0x01;
    }
    else
    {
        this->SairResult.status=0x02;
    }
    memcpy(&cambuf[7],&this->SairResult.status,1);
    memcpy(&cambuf[8],&this->SairResult.Xcoord,4);
    memcpy(&cambuf[12],&this->SairResult.Ycoord,4);
    cambuf[16]=this->countCRC(cambuf,16);
    controlbuf[0]=0xA5;
    controlbuf[1]=0x5A;
    controlbuf[2]=0x01;
    controlbuf[3]=0x13;
    controlbuf[4]=0x06;
    controlbuf[5]=0x16;
    controlbuf[6]=0x00;
    memcpy(&controlbuf[7],&this->sendKey,4);
    controlbuf[11]=0x00;
    controlbuf[12]=0x01;
    memcpy(&controlbuf[13],&this->SfControl.OffsetX,4);
    memcpy(&controlbuf[17],&this->SfControl.OffsetY,4);
    memcpy(&controlbuf[21],&this->SfControl.OffsetZ,4);
    memcpy(&controlbuf[25],&this->SfControl.OffsetW,4);
    controlbuf[29]=this->countCRC(controlbuf,29);
    if(this->SendCount >= this->SendFlag) {
        this->len=UART0_Send(this->fd,cambuf,17);
        this->coutRecC(cambuf,17,ofs_s);
        if(this->SendOffsetX*this->SfControl.OffsetX<0 || this->SendOffsetY*this->SfControl.OffsetY<0){
            this->SendStop();
            this->SendOffsetX = this->SfControl.OffsetX;
            this->SendOffsetY = this->SfControl.OffsetY;
            this->SendCount = this->SendFlag - 3;
        }
        else {
            this->len=UART0_Send(this->fd,controlbuf,30);
            this->coutRecC(controlbuf,30,ofs_s);
            this->SendCount = 0;
            this->sendKey=this->sendKey+1;
        }
    }
    this->SendCount++;
    cout<<this->SendCount<<endl;
//    this->len=UART0_Send(this->fd,cambuf,17);
//    this->len=UART0_Send(this->fd,controlbuf,30);
//    this->sendKey=this->sendKey+1;
} //无人机平台控制

void pipelineData::Send0x03()
{
    char sendbuf[10];
    sendbuf[0]=0xA5;
    sendbuf[1]=0x5A;
    sendbuf[2]=0x01;
    sendbuf[3]=0x13;
    sendbuf[4]=0x07;
    sendbuf[5]=0x02;
    sendbuf[6]=0x00;
    sendbuf[7]=0x01;
    sendbuf[8]=0x01;
//    sendbuf[9]=this->countCRC(sendbuf,9);
    sendbuf[9]=0x1f;
    this->len=UART0_Send(this->fd,sendbuf,10);
    this->coutRecC(sendbuf,10,ofs_s);
    printf("Toss!!!!\n");
    ofs_a<<"Toss!!!!\n";
    ofs_a.flush();
}  //抛投系统

void pipelineData::Send0x031()
{
    char sendbuf[10];
    sendbuf[0]=0xA5;
    sendbuf[1]=0x5A;
    sendbuf[2]=0x01;
    sendbuf[3]=0x13;
    sendbuf[4]=0x07;
    sendbuf[5]=0x02;
    sendbuf[6]=0x00;
    sendbuf[7]=0x01;
    sendbuf[8]=0x00;
    sendbuf[9]=0x1e;
    this->len=UART0_Send(this->fd,sendbuf,10);
    this->coutRecC(sendbuf,10,ofs_s);
    printf("Toss Close!!\n");
    ofs_a<<"Toss Close!!\n";
    ofs_a.flush();
} //抛投

void pipelineData::Send0x04()
{
    char sendbuf[9];
    sendbuf[0]=0xA5;
    sendbuf[1]=0x5A;
    sendbuf[2]=0x01;
    sendbuf[3]=0x13;
    sendbuf[4]=0x08;
    sendbuf[5]=0x01;
    sendbuf[6]=0x00;
    sendbuf[7]=0x0F;
    sendbuf[8]=this->countCRC(sendbuf,8);
    this->len=UART0_Send(this->fd,sendbuf,9);
    this->coutRecC(sendbuf,9,ofs_s);
} //返航






