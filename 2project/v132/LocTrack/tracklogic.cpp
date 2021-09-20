#include "tracklogic.h"

tracklogic::tracklogic()
{

}
//下面这两个更新函数是做什么的呢
void tracklogic::myLocTrackInit(pipelineData* plDin) {
    myCamParam.xsize =plDin->TrackFrame.cols*resizescale; myCamParam.ysize = plDin->TrackFrame.rows*resizescale; myCamParam.xrisze = 1*resizescale; myCamParam.yrisze = 1*resizescale; myCamParam.d = 0.003;
    myCamParam.f = 2.81;    myCamParam.rect=Rect(myCamParam.rect.x*resizescale,myCamParam.rect.y*resizescale,myCamParam.rect.width*resizescale,myCamParam.rect.height*resizescale);
    if(resizescale != 1){
        resize(myCamParam.image,myCamParam.image,Size(plDin->TrackFrame.cols*resizescale,plDin->TrackFrame.rows*resizescale));
    } //应该是得到图片或者矩形框的一些宽度高度包括重新塑造的参数信息
    myLocTracker->myinit(myUavParam,myCamParam); //初始化函数
}

void tracklogic::myLocTrackUpdate_1(pipelineData* plDin){
    if(resizescale != 1){
        resize(myCamParam.image,myCamParam.image,Size(plDin->TrackFrame.cols*resizescale,plDin->TrackFrame.rows*resizescale));
    }
    myLocTracker->myupdate_1(myUavParam,myCamParam);
}

void tracklogic::myLocTrackUpdate_2( ){
    myLocTracker->myupdate_2(myUavParam,myCamParam);
}

Rect tracklogic::getRoi(){
    Rect temp =  myLocTracker->getroi(); //定位
    if(resizescale != 1){
        temp =Rect(temp.x/resizescale,temp.y/resizescale,temp.width/resizescale,temp.height/resizescale);
    }
    return temp;
}

Mat tracklogic::getGPS(){
    return myLocTracker->getPos(); //定位得到的结果
}

float tracklogic::IOU(const ioubox& b1, const ioubox& b2) {
    float w = std::min(b1.m_left + b1.m_width, b2.m_left + b2.m_width)
            - std::max(b1.m_left, b2.m_left);  //宽
    float h = std::min(b1.m_top + b1.m_height, b2.m_top + b2.m_height)
            - std::max(b1.m_top, b2.m_top);  //高
    if (w <= 0 || h <= 0)
        return 0;
    return (w * h) / ((b1.m_height * b1.m_width) + (b2.m_height * b2.m_width) - (w * h));
}

bool tracklogicctl::initFlg = false;
//*************************************************************************
//*************************************************************************
void tracklogicctl::runTrack(pipelineData *plDin) {
    pipelineData* plD = plDin;
    tracklogic* tlc = new tracklogic();
    realsensehlp *rsh =new realsensehlp();
    Rect DrawRect;
    float font_scale = 1;
    printf("Track inital done!\n");
    plD->ofs_a<<"Track inital done!\n";
    plD->ofs_a.flush();
    cv::namedWindow("detection", cv::WINDOW_AUTOSIZE);
    //        cv::namedWindow("detection", cv::WINDOW_NORMAL);
    initFlg = true;
    while(plD->SDetResult.gRun) { //在这里设置了三个
        if(plD->TrackStrart == true && plD->DetectStart == false) {
            printf("Track start!\n");
            plD->ofs_a<<"Track start!\n";
            plD->ofs_a.flush();
            tlc->detB = plD->detBoxBest;
            int Cl;
            tlc->rs2 = plD->rs2;
            tlc->imageShow = plD->ShowFrame.clone();
            tlc->myCamParam.image = plD->TrackFrame.clone();
            tlc->depImage = plD->depFrame.clone();

            tlc->myCamParam.rect=Rect(plD->detBoxBest.bx0,plD->detBoxBest.by0,plD->detBoxBest.bw,plD->detBoxBest.bh);
            plD->DetectStart = true;
            plD->TrackStrart = false;
            plD->cam_start = true;
            if(plD->SBox.size()) {
                printf("XXX1\n");
                plD->ofs_a<<"XXX1\n";
                plD->ofs_a.flush();
                if(plD->track_init) {
                    tlc->myUavParam = plD->SDetResult.uavparam.back();
                    plD->SDetResult.uavparam.clear();
                    plD->SDetResult.uavparam.push_back(tlc->myUavParam);
                    if(0<plD->SDetResult.Distance && plD->SDetResult.Distance<3500) {
                        tlc->myUavParam.h = ((float) plD->SDetResult.Distance)/1000 + tlc->myUavParam.fh;
                    }
                    tlc->myLocTrackUpdate_1(plD);
                    tlc->myRoi=tlc->getRoi();
                    cv::rectangle(tlc->imageShow, cv::Point(tlc->myRoi.x, tlc->myRoi.y), cv::Point(tlc->myRoi.x+tlc->myRoi.width, tlc->myRoi.y+tlc->myRoi.height), cv::Scalar(0, 255, 255), 2);
                    float IOUt=-1;

                    ioubox boxtrack(tlc->myRoi.x,tlc->myRoi.y,tlc->myRoi.width,tlc->myRoi.height);
                    Point p(960,540);
                    Point p2(tlc->myRoi.x + tlc->myRoi.width*0.5, tlc->myRoi.y + tlc->myRoi.height*0.5);
                    plD->SDetResult.p.x=tlc->myRoi.x + tlc->myRoi.width*0.5;
                    plD->SDetResult.p.y= tlc->myRoi.y + tlc->myRoi.height*0.5;
                    line(tlc->imageShow,p,p2,Scalar(0,255,0),2);
                    for(int i=0; i<plD->SBox.size(); i++) {
                        tlc->b = plD->SBox[i];
                        ioubox boxnow(tlc->b.bx0,tlc->b.by0,tlc->b.bw,tlc->b.bh);
                        float IOUn=tlc->IOU(boxtrack,boxnow);
                        if(IOUt<IOUn) {
                            DrawRect=Rect(tlc->b.bx0,tlc->b.by0,tlc->b.bw,tlc->b.bh);
                            IOUt=IOUn;
                            Cl=tlc->b.bcl;
                        }
                    }
                    if(tlc->myUavParam.hLidar<7.5 && Cl==0 && plD->resetWithSurface) {
                        //                            plD->SendFlag = 4;
                        //                            plD->SendCount = 0;
                        plD->ofs_a<<"JJJJJJJJJJJJJJJJJJ\t"<<tlc->myUavParam.hLidar<<endl;
                        plD->ofs_a.flush();
                        plD->SDetResult.Reset=true;
                        plD->track_init=false;
                    }
                    else {
                        plD->SDetResult.Reset=false;
                    }

                    tlc->myCamParam.rect=Rect(DrawRect.x*tlc->resizescale,DrawRect.y*tlc->resizescale,DrawRect.width*tlc->resizescale,DrawRect.height*tlc->resizescale);
                    qDebug()<< "if  :tlc->myCamParam.rect.x: "<<tlc->myCamParam.rect.x<<"        tlc->myCamParam.rect.y: "<<tlc->myCamParam.rect.y<<"        tlc->myCamParam.rect.height: "<<tlc->myCamParam.rect.height<<"      tlc->myCamParam.rect.width:"<<tlc->myCamParam.rect.width<<endl;
                    tlc->myLocTrackUpdate_2();
                    cout<<"Found Box: "<<tlc->myCamParam.rect.x<<" "<<tlc->myCamParam.rect.y<<" "<<tlc->myCamParam.rect.x + tlc->myCamParam.rect.width<<" "<<tlc->myCamParam.rect.y + tlc->myCamParam.rect.height;
                    plD->ofs_a<<"Found Box: "<<tlc->myCamParam.rect.x<<" "<<tlc->myCamParam.rect.y<<" "<<tlc->myCamParam.rect.x + tlc->myCamParam.rect.width<<" "<<tlc->myCamParam.rect.y + tlc->myCamParam.rect.height<<endl;
                    plD->ofs_a.flush();
                    plD->SDetResult.loc= tlc->getGPS();
                    string loc2 =  "x: "+to_string(plD->SDetResult.loc.at<double>(0, 0))+"    y: "+ to_string(plD->SDetResult.loc.at<double>(1, 0))+"    z: "+ to_string(plD->SDetResult.loc.at<double>(2, 0));
                    cv::putText(tlc->imageShow, loc2, cv::Point(50, 50), cv::FONT_HERSHEY_SIMPLEX, font_scale, cv::Scalar(255, 255, 255), 1.5);
                    cout<<"Return Loc: "<<plD->SDetResult.loc.at<double>(0, 0)<<" "<<plD->SDetResult.loc.at<double>(1, 0)<<" "<<plD->SDetResult.loc.at<double>(2, 0)<<endl;
                    plD->ofs_a<<"Return Loc: "<<plD->SDetResult.loc.at<double>(0, 0)<<" "<<plD->SDetResult.loc.at<double>(1, 0)<<" "<<plD->SDetResult.loc.at<double>(2, 0)<<endl;
                    plD->ofs_a.flush();
                    plD->SDetResult.point.push_back(DrawRect.x);
                    plD->SDetResult.point.push_back(DrawRect.y);
                    plD->SDetResult.point.push_back(DrawRect.x+DrawRect.width);
                    plD->SDetResult.point.push_back(DrawRect.y+DrawRect.height);
                    printf("XXXX2\n");
                    plD->ofs_a<<"XXXX2\n";
                    plD->ofs_a.flush();
                    cout<<DrawRect.x<<" "<<DrawRect.y<<" "<<DrawRect.x+DrawRect.width<<" "<<DrawRect.y+DrawRect.height<<endl;
                    plD->ofs_a<<DrawRect.x<<" "<<DrawRect.y<<" "<<DrawRect.x+DrawRect.width<<" "<<DrawRect.y+DrawRect.height<<endl;
                    plD->ofs_a.flush();
                    //                        cv::rectangle(tlc->imageShow, cv::Point(DrawRect.x, DrawRect.y), cv::Point(DrawRect.x+DrawRect.width, DrawRect.y+DrawRect.height), cv::Scalar(255, 255, 255), 2);
                    if(IOUt<0.5 && IOUt>0.1) {
                        cv::rectangle(tlc->imageShow, cv::Point(tlc->myRoi.x, tlc->myRoi.y), cv::Point(tlc->myRoi.x+tlc->myRoi.width, tlc->myRoi.y+tlc->myRoi.height), cv::Scalar(255, 255, 255), 2);
                        //                            plD->trackInitRect = DrawRect;
                        plD->StrackBox.trackInitRect = DrawRect;
                        plD->StrackBox.trackInitMat = tlc->myCamParam.image.clone();
                        plD->StrackBox.kLock += 2;
                        if(plD->StrackBox.kLock>=plD->StrackBox.lockConf) {
                            plD->StrackBox.kLock -= 2;
                            //                                plD->StrackBox.kLock = plD->StrackBox.lockConf;
                            plD->StrackBox.trueTraget = true;
                        }
                        plD->track_init=false;
                    }
                    else if(IOUt>0.5) {
                        cv::rectangle(tlc->imageShow, cv::Point(DrawRect.x, DrawRect.y), cv::Point(DrawRect.x+DrawRect.width, DrawRect.y+DrawRect.height), cv::Scalar(255, 255, 255), 2);
                        //                            if(plD->StrackBox.kLock<plD->StrackBox.lockConf) {
                        plD->StrackBox.kLock += 2;
                        //                            }
                        if(plD->StrackBox.kLock>=plD->StrackBox.lockConf) {
                            plD->StrackBox.kLock -= 1;
                            //                                plD->StrackBox.kLock = plD->StrackBox.lockConf;
                            plD->StrackBox.trueTraget = true;
                        }
                    }
                    else {
                        if(plD->StrackBox.trueTraget) {
                            cv::rectangle(tlc->imageShow, cv::Point(tlc->myRoi.x, tlc->myRoi.y), cv::Point(tlc->myRoi.x+tlc->myRoi.width, tlc->myRoi.y+tlc->myRoi.height), cv::Scalar(255, 255, 255), 2);
                            plD->StrackBox.kLock -= 1;
                            if(plD->StrackBox.kLock<=0){
                                plD->StrackBox.kLock = 0;
                                plD->StrackBox.trueTraget = false;
                                plD->track_init=false;
                            }
                        }
                        else {
                            cv::rectangle(tlc->imageShow, cv::Point(tlc->myRoi.x, tlc->myRoi.y), cv::Point(tlc->myRoi.x+tlc->myRoi.width, tlc->myRoi.y+tlc->myRoi.height), cv::Scalar(255, 255, 255), 2);
                            plD->track_init=false;
                        }
                    }
                }
                else {
                    tlc->myUavParam = plD->SDetResult.uavparam.back();
                    plD->SDetResult.uavparam.clear();
                    plD->SDetResult.uavparam.push_back(tlc->myUavParam);
                    if(0<plD->SDetResult.Distance && plD->SDetResult.Distance<3500) {
                        tlc->myUavParam.h = ((float) plD->SDetResult.Distance)/1000 + tlc->myUavParam.fh;
                    }
                    printf("XXXX3\n");
                    plD->ofs_a<<"XXXX3\n";
                    plD->ofs_a.flush();
                    cout<<"Found Box: "<<tlc->myCamParam.rect.x<<" "<<tlc->myCamParam.rect.y<<" "<<tlc->myCamParam.rect.x + tlc->myCamParam.rect.width<<" "<<tlc->myCamParam.rect.y + tlc->myCamParam.rect.height<<endl;
                    plD->ofs_a<<"Found Box: "<<tlc->myCamParam.rect.x<<" "<<tlc->myCamParam.rect.y<<" "<<tlc->myCamParam.rect.x + tlc->myCamParam.rect.width<<" "<<tlc->myCamParam.rect.y + tlc->myCamParam.rect.height<<endl;
                    plD->ofs_a.flush();
                    if(plD->StrackBox.trueTraget && plD->SDetResult.Reset==false) {
                        tlc->myCamParam.rect = plD->StrackBox.trackInitRect;
                        tlc->myCamParam.image = plD->StrackBox.trackInitMat.clone();
                    }
                    else {
                        plD->StrackBox.kLock = 0;
                        plD->StrackBox.trueTraget = false;
                    }
                    tlc->myLocTrackInit(plD);
                    plD->track_init=true;
                    plD->SDetResult.Reset=false;
                    cv::rectangle(tlc->imageShow, cv::Point(tlc->myCamParam.rect.x, tlc->myCamParam.rect.y), cv::Point(tlc->myCamParam.rect.x + tlc->myCamParam.rect.width, tlc->myCamParam.rect.y + tlc->myCamParam.rect.height), cv::Scalar(255, 255, 255), 2);
                    plD->SDetResult.loc = tlc->getGPS();
                    string loc2 =  "x: "+to_string(plD->SDetResult.loc.at<double>(0, 0))+"    y: "+ to_string(plD->SDetResult.loc.at<double>(1, 0))+"    z: "+ to_string(plD->SDetResult.loc.at<double>(2, 0));
                    cv::putText(tlc->imageShow, loc2, cv::Point(50, 50), cv::FONT_HERSHEY_SIMPLEX, font_scale, cv::Scalar(255, 255, 255), 1.5);
                    cout<<"Return Loc: "<<plD->SDetResult.loc.at<double>(0, 0)<<" "<<plD->SDetResult.loc.at<double>(1, 0)<<" "<<plD->SDetResult.loc.at<double>(2, 0)<<endl;
                    plD->ofs_a<<"Return Loc: "<<plD->SDetResult.loc.at<double>(0, 0)<<" "<<plD->SDetResult.loc.at<double>(1, 0)<<" "<<plD->SDetResult.loc.at<double>(2, 0)<<endl;
                    plD->ofs_a.flush();
                    plD->SDetResult.p.x=(tlc->detB.bx0 + tlc->detB.bx1)*0.5;
                    plD->SDetResult.p.y= (tlc->detB.by0 + tlc->detB.by1)*0.5;
                    plD->SDetResult.point.push_back(tlc->detB.bx0);
                    plD->SDetResult.point.push_back(tlc->detB.by0);
                    plD->SDetResult.point.push_back(tlc->detB.bx1);
                    plD->SDetResult.point.push_back(tlc->detB.by1);
                }
            }
            else {
                tlc->myUavParam = plD->SDetResult.uavparam.back();
                plD->SDetResult.uavparam.clear();
                plD->SDetResult.uavparam.push_back(tlc->myUavParam);
                if(0<plD->SDetResult.Distance && plD->SDetResult.Distance<3500) {
                    tlc->myUavParam.h =((float) plD->SDetResult.Distance)/1000 + tlc->myUavParam.fh;
                }

                printf("XXXX4\n");
                plD->ofs_a<<"XXXX4\n";
                plD->ofs_a.flush();
                if(plD->track_init) {
                    tlc->myLocTrackUpdate_1(plD);
                    //                        tlc->myCamParam.rect=getRoi();


                    cout<< "else:   tlc->myCamParam.rect.x: "<<tlc->myCamParam.rect.x<<"        tlc->myCamParam.rect.y: "<<tlc->myCamParam.rect.y<<"        tlc->myCamParam.rect.height: "<<tlc->myCamParam.rect.height<<"      tlc->myCamParam.rect.width:"<<tlc->myCamParam.rect.width<<endl;
                    tlc->myLocTrackUpdate_2();
                    tlc->myRoi=tlc->getRoi();
                    cv::rectangle(tlc->imageShow, cv::Point(tlc->myRoi.x, tlc->myRoi.y), cv::Point(tlc->myRoi.x+tlc->myRoi.width, tlc->myRoi.y+tlc->myRoi.height), cv::Scalar(0, 255, 255), 2);
                    Point p(960,540);
                    Point p2(tlc->myRoi.x + tlc->myRoi.width*0.5, tlc->myRoi.y + tlc->myRoi.height*0.5);
                    plD->SDetResult.p.x=tlc->myRoi.x + tlc->myRoi.width*0.5;
                    plD->SDetResult.p.y= tlc->myRoi.y + tlc->myRoi.height*0.5;
                    line(tlc->imageShow,p,p2,Scalar(0,255,0),2);
                    plD->SDetResult.point.push_back(tlc->myRoi.x);
                    plD->SDetResult.point.push_back(tlc->myRoi.y);
                    plD->SDetResult.point.push_back(tlc->myRoi.x + tlc->myRoi.width);
                    plD->SDetResult.point.push_back(tlc->myRoi.y + tlc->myRoi.height);
                    qDebug()<<"Found Box: "<<tlc->myCamParam.rect.x<<" "<<tlc->myCamParam.rect.y<<" "<<tlc->myCamParam.rect.x + tlc->myCamParam.rect.width<<" "<<tlc->myCamParam.rect.y + tlc->myCamParam.rect.height;
                    plD->ofs_a<<"Found Box: "<<tlc->myCamParam.rect.x<<" "<<tlc->myCamParam.rect.y<<" "<<tlc->myCamParam.rect.x + tlc->myCamParam.rect.width<<" "<<tlc->myCamParam.rect.y + tlc->myCamParam.rect.height<<endl;
                    plD->ofs_a.flush();
                    plD->SDetResult.loc= tlc->getGPS();
                    string loc2 =  "x: "+to_string(plD->SDetResult.loc.at<double>(0, 0))+"    y: "+ to_string(plD->SDetResult.loc.at<double>(1, 0))+"    z: "+ to_string(plD->SDetResult.loc.at<double>(2, 0));
                    cv::putText(tlc->imageShow, loc2, cv::Point(50, 50), cv::FONT_HERSHEY_SIMPLEX, font_scale, cv::Scalar(255, 255, 255), 1.5);
                    cout<<"Return Loc: "<<plD->SDetResult.loc.at<double>(0, 0)<<" "<<plD->SDetResult.loc.at<double>(1, 0)<<" "<<plD->SDetResult.loc.at<double>(2, 0)<<endl;
                    plD->ofs_a<<"Return Loc: "<<plD->SDetResult.loc.at<double>(0, 0)<<" "<<plD->SDetResult.loc.at<double>(1, 0)<<" "<<plD->SDetResult.loc.at<double>(2, 0)<<endl;
                    plD->ofs_a.flush();
                    if(plD->StrackBox.trueTraget) {
                        cv::rectangle(tlc->imageShow, cv::Point(tlc->myRoi.x, tlc->myRoi.y), cv::Point(tlc->myRoi.x+tlc->myRoi.width, tlc->myRoi.y+tlc->myRoi.height), cv::Scalar(255, 255, 255), 2);
                        plD->StrackBox.kLock -= 2;
                        if(plD->StrackBox.kLock<=0){
                            plD->StrackBox.kLock = 0;
                            plD->StrackBox.trueTraget = false;
                            plD->track_init=false;
                        }
                    }
                    else {
                        plD->StrackBox.kLock = 0;
                        plD->track_init=false;
                    }
                }
                else {
                    std::cout<<"Target Loss!!!\n";
                    plD->ofs_a<<"Target Loss!!!\n";
                    plD->ofs_a.flush();
                    plD->SDetResult.TargetLoss=true;
                }
            }//////////
            printf("Track Done!\n");
            plD->ofs_a<<"Track Done!\n";
            plD->ofs_a.flush();
            if(tlc->myUavParam.hLidar < 8 && Cl == 1) {
                uint32_t distance = rsh->RoiDistanceInColor(
                            tlc->depImage,tlc->myCamParam.image,tlc->rs2,cv::Point(plD->SDetResult.point[0], plD->SDetResult.point[1]),cv::Point(plD->SDetResult.point[2], plD->SDetResult.point[3]));
                plD->SDetResult.Distance=distance;
                string loc2 =  "Distance: "+to_string(distance);
                cv::putText(tlc->imageShow, loc2, cv::Point(150, 150), cv::FONT_HERSHEY_SIMPLEX, font_scale, cv::Scalar(255, 255, 255), 1.5);
                if(plD->SDetResult.Distance>3000 && plD->SDetResult.Distance<5000) {
                    plD->magicStop = plD->SairAttitude.Height - ((float) plD->SDetResult.Distance)/1000 + 1.5;
                    plD->changeTossHeight = true;
                }
                if(plD->changeTossHeight && plD->SDetResult.Distance>0 && plD->SDetResult.Distance<3000) {
                    plD->magicStop = plD->SairAttitude.Height - ((float) plD->SDetResult.Distance)/1000 + 1.5;
                    plD->changeTossHeight = false;
                }
            }
            if(plD->SairAttitude.Height <= 6.0 && Cl == 0) {
                plD->SDetResult.TargetLoss=true;
            }

            if(plD->SBox.size()>0){
                plD->findFlg -=1;
            }
            if(plD->findFlg >0){
                plD->Update0x06();
                if(plD->controlPipeline) {
                    plD->Send0x02();
                }
                cout<<"finding target..."<<endl;
            }
            else{
                plD->Update0x02();
                if(plD->controlPipeline) {
                    plD->Send0x02();
                }
            }

            cv::imshow("detection", tlc->imageShow);
            cv::waitKey(1);
        }
    }
    printf("TT shutdowm!\n");
    plD->ofs_a<<"TT shutdowm!\n";
    plD->ofs_a.flush();
}
