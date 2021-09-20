#include"common.h"
#include"local/localdata.h"
#include"Detect/detect.h"
#include"LocTrack/LocTrack.h"
#include"PartRealSense/realsensectl.h"
#include"porttalk/porttalk.h"



int main()
{
    //data
    TargetLib* targetAll = new TargetLib;

    //func
    location* LocationFunctions = new location;

    runsupportfunctions *RunSupportFunctions = new runsupportfunctions();
    projectdefaultpath defaultAllPath= RunSupportFunctions->getDefaultPath();

    //object
    targetidmap* TargetIDMap = new targetidmap;
    TargetIDMap->setFlashPower(0.1);

    detect *Detect=new detect;
    Detect->myinit(defaultAllPath.netPath);

    portCommunicate *PortCommunicate=new portCommunicate;
    PortCommunicate->myinit();

    realsensectl *RealSense =new realsensectl;
    RealSense->myinit();

    //init ?
    while(1){
        if(Detect->initFlg &&  PortCommunicate->initFlg && RealSense->initFlg){
            break;
        }
        QThread::msleep(10);
    }

    //run
    while (1) {

        //flash
        vector<Mat> curImage = RealSense->oneShot();
        portRecieveInf curPortInf= PortCommunicate->getCurrenInf();
        DetectData curDetectInf = Detect->getCurrentDetectInf();

        //makesure port is normal to skip no gps situation
        //show is needly,so we con't take it out at this while(1)
        if(curPortInf.recieveFlg==false) {
            if(curImage[0].rows==0){
                continue;
            }
            imshow("1",curImage[0]);
            waitKey(20);
        }

        else{    //begin
            if(curDetectInf.flashFlg){  //detect is ready to get new img?

                bool DetectFrameIsNotEmpty = !curDetectInf.DetectFrame.empty();
                if(DetectFrameIsNotEmpty ){  //detect have img ? makesure not the first frame
                    vector<camParam> vecCamParam=RunSupportFunctions->Bbox2camParam(curDetectInf.detV);
                    UAVParam uavParam = RunSupportFunctions->portInf2uavParam(curPortInf);
                    vector<Mat> locResult;
                    for(int i =0;i<vecCamParam.size();i++){
                        Mat gps = LocationFunctions->computeGps(uavParam,vecCamParam[i]);
                        locResult.push_back(gps);
                    }

                    //Map
                    vector<Point2d> vecPoints = RunSupportFunctions->mat2Point2d(locResult);
                    vector<int> vecID;
                    if(vecPoints.size()!=0){
                        vecID= TargetIDMap->compare(vecPoints);
                    }

                    //Update
                    targetAll = RunSupportFunctions->updateTargetMap(targetAll,vecID,curDetectInf.detV);

                    //show
                    Mat img = RunSupportFunctions->drawVecs(curDetectInf.DetectFrame,curDetectInf.detV);
                    img = RunSupportFunctions->putText(img,curDetectInf.detV,vecID);
                    vector<Point> Points = LocationFunctions->computeBackPoints(uavParam);
                    img = RunSupportFunctions->drawLine(img,Points);


                    imshow("1",img);
                    waitKey(10);
                }

                PortCommunicate->recordSelfPortInf(1,"./1.txt");
                Detect->setNewData(curImage[0].clone(),false);
            }
        }
    }

    //
    cout<<"IT IS HARD TO MAKE,HAHA";

    return 0;
}
