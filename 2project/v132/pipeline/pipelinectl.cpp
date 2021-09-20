#include "pipelinectl.h"

pipelineCtl::pipelineCtl()
{

}
bool pipelineCtl::initFlg = false; //飞控的标志位

void  pipelineCtl::Rec(pipelineData* plD) {
    unsigned short usLength=0;
    char rcv_buf[2000];
    while(!plD->shutdown) {
        usLength = plD->UART0_Recv(plD->fd, rcv_buf,2000);
        if (usLength>0&&usLength<65535)
        {
            plD->CopeSerialData(rcv_buf,usLength);
        }
    }
    printf("Rec shutdown!\n");
} //shutdown也是设置的一个布偶位，接收串口数据，

void  pipelineCtl::flush(){
    vector<string> txtName = {"rec","log","send"}; //建立三个txt
    for(int i = 0 ; i<txtName.size();i++){
        ofstream ofs("/home/nvidia/jiqun/"+txtName[i]+".txt",std::ios::out);
        ofs<<"Start writing!\n";
        ofs.close();
    }
    qDebug()<<"load Aircontrol ...";
    this->plD->ofs_a<<"load Aircontrol ..."<<endl;
    this->plD->ofs_a.flush();
    this->plD->ID = 0x00;
    if(!this->plD->ofs_a){
        printf("File open failes!\n");
    }
    this->plD->ofs_a<<"detection\n";
    this->plD->ofs_a.flush();
    std::cout<<"detection\n";
    int err;
    this->pipeCount = 0;
    this->firstDetect = 1;
    this->brkFLg = false;
    this->plD->fd = open("/dev/ttyTHS2",O_RDWR|O_NOCTTY|O_NDELAY);
    do{
        err = this->plD->UART0_Init(this->plD->fd,115200,0,8,1,'N');
        printf("Set Port Exactly!\n");
        this->plD->ofs_a<<"Set Port Exactly!\n";
        this->plD->ofs_a.flush();
    }while(-1 == err || -1 == this->plD->fd);
    printf("Inital done!\n");
    this->plD->ofs_a<<"Inital done!\n";
    this->plD->ofs_a.flush();
} //在终端看到的刷新应该都在这里

void pipelineCtl::send(pipelineData* plDin, realsensectl *rsc) {
    pipelineData* plD = plDin;
    switch (plD->ID) {
    case 0x01:
        plD->Send0x01(); //启动成功
        this->pipeCount=0;
        QThread::msleep(50);
        plD->SairAttitude.Height = 999999;
        plD->Send0x05(); //目标检测结果输出
        if(plD->VisualInital) {
            firstDetect = 1; //第一次探测
            cout<<"XXXXXXXXXXX\n";
            plD->ofs_a<<"XXXXXXXXXXX\n";
            plD->ofs_a.flush();
            plD->ID=0x05; //进入0X05
        }
        break;
    case 0x02:
        if(plD->SairAttitude.Yaw*plD->SairAttitude.Yaw >25) {
            plD->ID=0x01;
            break;
        } //如果偏航角距离标准值差距过大，就需要重新走一遍0X01，0X05，调整飞机姿态
        if(plD->SDetResult.gRun && plD->cam_start) { //如果检测和相机标志位打开，起初设置都为真
            vector<Mat> matVect = rsc->oneShot(); //拿图
            Mat cap = matVect[0].clone();
            Mat dep = matVect[1].clone(); //写
            plD->rs2 = rsc->rs2Shot();
            plD->DetectFrame = cap.clone();
            plD->TrackFrame = cap.clone();
            plD->depFrame = dep.clone(); //跟踪框，检测框
            if(firstDetect == 1) { //如果第一次探测的标志位为1
                plD->DetectStart = true;
                firstDetect = 0;
            }
            this->pipeCount++; //计数
            cout<<"OOOOOOOOO   "<<this->pipeCount<<endl;
            plD->ofs_a<<"OOOOOOOOO   "<<this->pipeCount<<endl;
            plD->ofs_a.flush();
            cout<<"TTTTTTT"<<plD->Toss<<endl;
            plD->ofs_a<<"TTTTTTT\t"<<plD->Toss<<endl;
            plD->ofs_a.flush();
            plD->cam_start = false; //相机bool位改为假
        }

        if(plD->matReload) {
            vector<Mat> matVect = rsc->oneShot();
            Mat cap = matVect[0].clone();
            Mat dep = matVect[1].clone();
            plD->rs2 = rsc->rs2Shot();
            plD->DetectFrame = cap.clone();
            plD->TrackFrame = cap.clone();
            plD->depFrame = dep.clone();
            plD->matReload = false;
        }

        if(plD->SfControl.OffsetX*plD->SfControl.OffsetY==0) {
            plD->T = false;
        }
        else {
            double q = plD->SfControl.OffsetX ;
            double w = plD->SfControl.OffsetY ;
            plD->Toss = sqrt(q*q + w*w);
            plD->T = true;
        } //计算距离抛投点的距离
        plD->controlPipeline = true; //航线控制bool位为真
        if(plD->Toss < plD->TossConf && plD->SairAttitude.Height < 5.0 && plD->SDetResult.Distance < 1600 && plD->SDetResult.Distance>0 && plD->T) {
            plD->controlPipeline = false;
            cout<<"CHANGE!!!!!!!!!!!!!!\n";
            plD->ofs_a<<"CHANGE!!!!!!!!!!!!!!\n";
            plD->ofs_a.flush();
            plD->SendStop();
            plD->Send0x03();
            plD->SDetResult.gRun=false;
            plD->ID=0x03;
        } //如果距离高度满足上述条件，直接进入0X03抛投

        if(plD->SairAttitude.Height < plD->ReturnConf || (plD->SDetResult.Distance < 900 && plD->SDetResult.Distance>0 && plD->SairAttitude.hLidar<5.0)) {
            plD->controlPipeline = false;
            plD->SendStop();
            plD->SDetResult.gRun=false;
            plD->ID=0x04;
        }  //如果满足上述条件，直接跳到0X04返航

        break;
    case 0x03:
        std::cout<<"detection end\n";
        plD->ofs_a<<"detection end\n";
        plD->ofs_a.flush();
        plD->Send0x03();
        sleep(2);
        plD->Send0x031();
        plD->ID=0x04;
        break;
    case 0x04:
        plD->Send0x031();
        sleep(1);
        while(plD->SairAttitude.status==0x01){
            plD->Send0x04(); //返航
            sleep(1);
        }
        plD->shutdown=true;
        break;
    case 0x05:
        //            plD->SendStop();
        sleep(1);
        for(float k=10; k>0; k--) {
            plD->Reset(k); //调整无人机姿态
            QThread::msleep(500);
            if(plD->ID == 0x01) {
                this->brkFLg=true;
                break;
            }
        }
        if(this->brkFLg == true) {
            break;
        }
        plD->SendStop(); //悬停
        plD->ID=0x02;  //进入0X02
        break;
    default:
        QThread::msleep(100);
        break;
    }
}
