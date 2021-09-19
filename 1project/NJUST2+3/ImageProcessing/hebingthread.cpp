#include "hebingthread.h"

#ifdef FULLY_BUILD
#include "Compute/character.h"
#endif

HebingThread::HebingThread(QObject *parent) : QObject(parent)
{
    thread = new QThread;

    this->moveToThread(thread);
    thread->start();

    QTimer::singleShot(0, this, SLOT(init()));
}

void HebingThread::reload()
{
    cam_map= PP->cameraMap();
    tranMatShift->clear();
    tranMatShift=new vector<double>{58.68,692.13,69.89,-721.16,-618.66,407.32};
    huidu=0;huiduMAX=0;
    moreEach=0;moreLast=0;
    allMerge.clear();
    comPCDMaskSaved.clear();
    comPCDMasksave=1;
    comPCDsaved.clear();
    comPCDsave =1;
    eachPCdsave = 1;

    StringALL.clear();
    NUMALL.clear();
    MaskPicALL.clear();
    WorkPieceNums.clear();
    NUMTOTALLSAVE.clear();
    CameraImg.clear();
    //--------------

    shotTimes = PP->camera_shot_times();
    eachPCD.clear();
    eachPCD_n.clear();
    comPCD.clear();
    comPCDmask.clear();


    workpiece_index = 0;
    PcdAll.clear();

    mycamparam.clear();
    path_source0 = PP->compute_path().toStdString();
    txt_path0 = path_source0 + "/TXT/";
    camParam temp;
    for (int i = 0; i < cam_map.size(); i++)
    {
        string tempstr;
        tempstr = txt_path0 +to_string(cam_map[i])+".txt";
        ifstream fin;
        fin.open(tempstr);
        fin>>temp.A >>temp.B>>temp.C>>temp.D>>  temp.E>>temp.F;
        fin.close();
        mycamparam.push_back(temp);
    }
    //
//    qDebug()<<"hebin_reload_finish";
//    qDebug()<<"eachPCD"<<eachPCD.size();
//    qDebug()<<" a :"<<a.size();

}

void HebingThread::init()
{
    tranMatShift=new  vector<double>{58.68,692.13,69.89,-721.16,-618.66,407.32};
    workpiece_index = 0;
    //插入合并初始化代码
    shotTimes = PP->camera_shot_times();

    cam_map = PP->cameraMap();

#ifdef FULLY_BUILD
    CCC = new character;
    if(PP->check_training_model()){
        CCC->load_east_net();
    }
#endif


    /*
    str
    cammap
    */

    path_source0 = PP->compute_path().toStdString();
    txt_path0 = path_source0 + "/TXT/";
    camParam temp;
    for (int i = 0; i < cam_map.size(); i++)
    {
        string tempstr;
        tempstr = txt_path0 +to_string(cam_map[i])+".txt";
        ifstream fin;
        fin.open(tempstr);
        fin>>temp.A >>temp.B>>temp.C>>temp.D>>  temp.E>>temp.F;
        fin.close();
        mycamparam.push_back(temp);
    }


    cout<<"hebing init"<<endl;
}

void HebingThread::add_one_group(int group_index, std::vector<cv::Mat> GongjianTU, std::vector<cv::Mat> GongjianTU_n,
                                 std::vector<std::vector<std::string> > allstring, std::vector<std::vector<int> > stringmasknum, std::vector<int> NUMTOTALL)
{
    QMutexLocker m_lock(&mutex);

    std::vector<cv::Point3d> pcd;
    std::string name;


    qDebug()<<"HebingThread::add_one_group begin" << group_index;

    StringALL.insert(StringALL.end(), allstring.begin(), allstring.end());
    NUMALL.insert(NUMALL.end(), stringmasknum.begin(), stringmasknum.end());
    MaskPicALL.insert(MaskPicALL.end(), GongjianTU.begin(), GongjianTU.end());
    NUMTOTALLSAVE.insert(NUMTOTALLSAVE.end(), NUMTOTALL.begin(), NUMTOTALL.end());


    int camOrder, movOrder;
    Point3d tempPoint;
    int nl = GongjianTU[0].rows, nc = GongjianTU[0].cols;

    //int getPhoto = iPhoto;
    moreLast = moreEach;	moreEach = 0;
    int befPCDSize = eachPCD.size();

    //qDebug()<<"step2";
    double  X3d, Y3d, Z3d;
    int cam_num = cam_map.size();
    cout<<"1111111111111111111111111111111111111111111111cam_num"<<cam_num<<endl;
    for(int iii=0;iii<cam_num;iii++)
    {
        cout<<"22222222222222222222222222222222222cam_num"<<cam_map[iii]<<endl;
    }


    //二维点转三维点
    for ( int iPhoto=0; iPhoto < cam_map.size(); iPhoto++)
    {

        vector<Mat> channels, channels_U;

        Mat imageBlueChannel, imageBlueChannel_n;
        split(GongjianTU[iPhoto], channels);
        split(GongjianTU_n[iPhoto], channels_U);
        imageBlueChannel = channels.at(0);
        imageBlueChannel_n = channels_U.at(0);
        camOrder = iPhoto % cam_num;

        movOrder = group_index -1;
        for (int iJ = 0; iJ < nl; iJ = iJ + 5)
        {
            for (int iI = 0; iI < nc; iI = iI + 5)
            {

                if (imageBlueChannel.at<ushort>(iJ, iI) > 0.0)
                {
                    huidu = imageBlueChannel.at<ushort>(iJ, iI);

                     if(huidu>eachPCD.size())
                     {
                         eachPCD.resize(huidu);
                         eachPCD_n.resize(huidu);
                     }

                    X3d = mycamparam[camOrder].A *iI + mycamparam[camOrder].B*iJ +mycamparam[camOrder].C;
                    Y3d = mycamparam[camOrder].D *iI + mycamparam[camOrder].E*iJ +mycamparam[camOrder].F;
                    Z3d = 0;

                   double distance = tranMatShift->at(cam_map[camOrder]-1) - PP->Longmen_x_s(cam_map[camOrder])[movOrder];
                   //L_SHIFT = 0

                    X3d = X3d - distance;
                    if(cam_map[camOrder]==1){
                        X3d=X3d-(0.0002*PP->Longmen_x_s(cam_map[camOrder])[movOrder]-1.9868);
                        Y3d=Y3d-(0.00006*PP->Longmen_x_s(cam_map[camOrder])[movOrder]-1.4735);
                    }
                    if(cam_map[camOrder]==2){
                        X3d=X3d-(0.0004*PP->Longmen_x_s(cam_map[camOrder])[movOrder]-1.2917);
                        Y3d=Y3d-(0.00005*PP->Longmen_x_s(cam_map[camOrder])[movOrder]-1.3834);
                    }
                    if(cam_map[camOrder]==3){
                        X3d =X3d- (-0.00009*PP->Longmen_x_s(cam_map[camOrder])[movOrder]-1.7271);
                        Y3d =Y3d- (0.00005*PP->Longmen_x_s(cam_map[camOrder])[movOrder]-1.6969);
                    }

                    if(cam_map[camOrder]==4){
                        X3d=X3d-(-0.00004*PP->Longmen_x_s(cam_map[camOrder])[movOrder]-1.9802);
                        Y3d=Y3d-(0.00003*PP->Longmen_x_s(cam_map[camOrder])[movOrder]-0.9707);
                    }
                    if(cam_map[camOrder]==5){
                        X3d=X3d-(-0.00005*PP->Longmen_x_s(cam_map[camOrder])[movOrder]-0.8921);
                        Y3d=Y3d-(-0.0002*PP->Longmen_x_s(cam_map[camOrder])[movOrder]-1.8);

                    }
                    if(cam_map[camOrder]==6){
                        X3d =X3d- (0.0001*PP->Longmen_x_s(cam_map[camOrder])[movOrder]-1.3954);
                        Y3d =Y3d- (-0.00002*PP->Longmen_x_s(cam_map[camOrder])[movOrder]-1.6345);
                    }
                    tempPoint.x = X3d;
                    tempPoint.y = Y3d;
                    tempPoint.z = Z3d;
                    eachPCD[huidu - 1].pcd.push_back(tempPoint);

                }
            }
        }

        for (int iJ = 0; iJ < nl; iJ = iJ + 2)
        {
            for (int iI = 0; iI < nc; iI = iI + 2)
            {
                if (imageBlueChannel_n.at<ushort>(iJ, iI) > 0.0)
                {
                    huidu = imageBlueChannel_n.at<ushort>(iJ, iI);
                    if(huidu>eachPCD_n.size())
                    {
                        eachPCD_n.resize(huidu);
                    }
                    X3d = mycamparam[camOrder].A *iI + mycamparam[camOrder].B*iJ +mycamparam[camOrder].C;
                    Y3d = mycamparam[camOrder].D *iI + mycamparam[camOrder].E*iJ +mycamparam[camOrder].F;
                    Z3d = 0;

                    double distance = tranMatShift->at(cam_map[camOrder]-1) - PP->Longmen_x_s(cam_map[camOrder])[movOrder];
                    //L_SHIFT = 0

                     X3d = X3d - distance;
                     if(cam_map[camOrder]==1){
                         X3d=X3d-(0.0002*PP->Longmen_x_s(cam_map[camOrder])[movOrder]-1.9868);
                         Y3d=Y3d-(0.00006*PP->Longmen_x_s(cam_map[camOrder])[movOrder]-1.4735);
                     }
                     if(cam_map[camOrder]==2){
                         X3d=X3d-(0.0004*PP->Longmen_x_s(cam_map[camOrder])[movOrder]-1.2917);
                         Y3d=Y3d-(0.00005*PP->Longmen_x_s(cam_map[camOrder])[movOrder]-1.3834);
                     }
                     if(cam_map[camOrder]==3){
                         X3d =X3d- (-0.00009*PP->Longmen_x_s(cam_map[camOrder])[movOrder]-1.7271);
                         Y3d =Y3d- (0.00005*PP->Longmen_x_s(cam_map[camOrder])[movOrder]-1.6969);
                     }

                     if(cam_map[camOrder]==4){
                         X3d=X3d-(-0.00004*PP->Longmen_x_s(cam_map[camOrder])[movOrder]-1.9802);
                         Y3d=Y3d-(0.00003*PP->Longmen_x_s(cam_map[camOrder])[movOrder]-0.9707);
                     }
                     if(cam_map[camOrder]==5){
                         X3d=X3d-(-0.00005*PP->Longmen_x_s(cam_map[camOrder])[movOrder]-0.8921);
                         Y3d=Y3d-(-0.0002*PP->Longmen_x_s(cam_map[camOrder])[movOrder]-1.8);

                     }
                     if(cam_map[camOrder]==6){
                         X3d =X3d- (0.0001*PP->Longmen_x_s(cam_map[camOrder])[movOrder]-1.3954);
                         Y3d =Y3d- (-0.00002*PP->Longmen_x_s(cam_map[camOrder])[movOrder]-1.6345);
                     }

                    tempPoint.x = X3d;
                    tempPoint.y = Y3d;
                    tempPoint.z = Z3d;
                    eachPCD_n[huidu - 1].pcd.push_back(tempPoint);
                }

            }
        }

    }


    moreEach = eachPCD.size() - befPCDSize;

   // qDebug()<<"step2";
    //2.2合并工件
    int maxDis = 10;
    int maxCount = 5;
    double kImage = 1;
    int stepFirst3d = 10;
    int stepSecond3d = 10;

  // qDebug()<<"step2.1";

    int combineStep = eachPCD.size() - moreEach - moreLast;

    qDebug()<<"step2.2";
     //qDebug()<<"step4";
    vector<int> beforeMerge, afterMerge;
    for (int i = 0; i < moreEach + moreLast; i++)
    {
        beforeMerge.push_back(i);
        afterMerge.push_back(i);
    }
    //savImg();
   // qDebug()<<"step4.1";
//    int a_del_1 = 1;


    //连接判断
    for (int n1 = combineStep; n1 < eachPCD.size(); n1++)
    {
        //qDebug()<<"stepFOr4.2";
        for (int n2 = n1 + 1; n2 < eachPCD.size(); n2++)
        //for (int n2 =combineStep; n2 < eachPCD.size(); n2++)
        {
            if(n1 == n2)
            {
                continue;
            }
            int mergeCount = 0;
            double x1, y1, x2, y2, xy1_2Distance;
            bool mergeFlage = 0;

            //if (abs(newol[n1]->points[0].x - newol[n2]->points[0].x)>nl*kImage)
            //	continue;
            if(eachPCD[n1].pcd.size()>0&& eachPCD[n2].pcd.size()>0){
                if (abs(eachPCD[n1].pcd[0].y - eachPCD[n2].pcd[0].y) > nc * 1.5 )
                {
                    continue;
                }
            }


            for (int in1 = 0; in1 < eachPCD[n1].pcd.size(); in1 = in1 + stepFirst3d)
            {
                for (int in2 = 0; in2 < eachPCD[n2].pcd.size(); in2 = in2 + stepSecond3d)
                {

                    x1 = eachPCD[n1].pcd[in1].x;
                    y1 = eachPCD[n1].pcd[in1].y;
                    x2 = eachPCD[n2].pcd[in2].x;
                    y2 = eachPCD[n2].pcd[in2].y;
                    xy1_2Distance = abs(x1 - x2) + abs(y1 - y2);
                    if (xy1_2Distance < maxDis)
                    {
                        mergeCount++;
                    }
                    if (mergeCount > maxCount)
                    {
                        mergeFlage = 1;
                        break;
                    }
                }
                if (mergeFlage)
                {
                    break;
                }
            }

            if (mergeFlage)
            {
                int min = afterMerge[n1 - combineStep];
                int max = afterMerge[n2 - combineStep];
                if (afterMerge[n2 - combineStep] < afterMerge[n1 - combineStep])
                {
                    min = afterMerge[n2 - combineStep];
                    max = afterMerge[n1 - combineStep];
                }
                for (int ih = 0; ih < afterMerge.size(); ih++)
                {
                    if (afterMerge[ih] == max)
                    {
                        afterMerge[ih] = min;
                    }
                }
            }
        }
    }




    //编号刷新

    allMerge.resize(eachPCD.size());		//allMerge.size() = allmerge.size() + moreEach

    for (int i = 0; i < afterMerge.size(); i++)	//afterMerge.size() = moreEach + moreLast
    {
        afterMerge[i] += eachPCD.size();
    }
    for (int i = 0; i < moreLast; i++)
    {
        int changetemp = afterMerge[i];
        for (int j = 0; j < afterMerge.size(); j++)
        {
            if (afterMerge[j] == changetemp)
            {
                afterMerge[j] = allMerge[allMerge.size() - afterMerge.size() + i];
            }
        }
    }
    for (int i = 0; i < afterMerge.size(); i++)
    {
        allMerge[allMerge.size() - afterMerge.size() + i] = afterMerge[i];
    }
    // qDebug()<<"step6";
    vector<int> tempMerge;
    for (int ih = 0; ih < allMerge.size(); ih++)
    {
        bool pushFlag = 1;
        for (int ih1 = 0; ih1 < tempMerge.size(); ih1++)
        {
            if (tempMerge[ih1] == allMerge[ih])
            {
                pushFlag = 0;
                break;
            }
        }
        if (pushFlag)
        {
            tempMerge.push_back(allMerge[ih]);
        }
    }

    vector<int> returnAftMg;
    returnAftMg.resize(allMerge.size());
    for (int ii = 0; ii < tempMerge.size(); ii++)
    {

        for (int i = 0; i < allMerge.size(); i++)
        {
            if (tempMerge[ii] == allMerge[i])
            {
                returnAftMg[i] = ii + 1;	//注意这里表示的工件编号已经加了一个1，再索值表示的时候可能要减回来,意味着工件编号从1开始哟
            }
        }
    }
    allMerge = returnAftMg;
   for(int i=0;i<allMerge.size();i++)
   {
        cout<<allMerge[i]<<"  ";
   }
   cout<<endl;


    // 组合成combine
    // mask
   for (int i = 0; i < comPCD.size(); i++)
   {
       if(!(comPCDmask[i].num==-2))
       {
           comPCDmask[i].num = -1;
       }
   }
   if(group_index < shotTimes)
   {
       for (int i = 0; i < moreLast; i++)
       {
           if (allMerge[eachPCD.size() - moreEach -moreLast+ i] > comPCDmask.size())
           {
               comPCDmask.resize(allMerge[eachPCD.size() - moreEach-moreLast + i]);
           }
           //int bigNum_temp = allMerge[eachPCD.size() - moreEach + i] - 1;
           comPCDmask[allMerge[eachPCD.size() - moreEach -moreLast+ i] - 1].pcd.insert(comPCDmask[allMerge[eachPCD.size() - moreEach -moreLast+ i] - 1].pcd.end(), eachPCD[eachPCD.size() - moreEach -moreLast+ i].pcd.begin(), eachPCD[eachPCD.size() - moreEach -moreLast+ i].pcd.end());
           comPCDmask[allMerge[eachPCD.size() - moreEach -moreLast+ i] - 1].num = 1;
           comPCDmask[allMerge[eachPCD.size() - moreEach -moreLast+ i] - 1].source.push_back(eachPCD.size() - moreEach -moreLast+ i + 1);
       }

   }else{
       for (int i = 0; i < moreLast+moreEach; i++)
       {
           if (allMerge[eachPCD.size() - moreEach -moreLast+ i] > comPCDmask.size())
           {
               comPCDmask.resize(allMerge[eachPCD.size() - moreEach-moreLast + i]);
           }
           //int bigNum_temp = allMerge[eachPCD.size() - moreEach + i] - 1;
           comPCDmask[allMerge[eachPCD.size() - moreEach -moreLast+ i] - 1].pcd.insert(comPCDmask[allMerge[eachPCD.size() - moreEach -moreLast+ i] - 1].pcd.end(), eachPCD[eachPCD.size() - moreEach -moreLast+ i].pcd.begin(), eachPCD[eachPCD.size() - moreEach -moreLast+ i].pcd.end());
           comPCDmask[allMerge[eachPCD.size() - moreEach -moreLast+ i] - 1].source.push_back(eachPCD.size() - moreEach -moreLast+ i + 1);
       }
   }

    // n
    for (int i = 0; i < comPCD.size(); i++)
    {
        if(!(comPCD[i].num==-2))
        {
            comPCD[i].num = -1;
        }
    }
    if(group_index < shotTimes)
    {
        for (int i = 0; i < moreLast; i++)
        {
            if (allMerge[eachPCD.size() - moreEach -moreLast+ i] > comPCD.size())
            {
                comPCD.resize(allMerge[eachPCD.size() - moreEach-moreLast + i]);
            }
            //int bigNum_temp = allMerge[eachPCD.size() - moreEach + i] - 1;
            comPCD[allMerge[eachPCD.size() - moreEach -moreLast+ i] - 1].pcd.insert(comPCD[allMerge[eachPCD.size() - moreEach -moreLast+ i] - 1].pcd.end(), eachPCD_n[eachPCD.size() - moreEach -moreLast+ i].pcd.begin(), eachPCD_n[eachPCD.size() - moreEach -moreLast+ i].pcd.end());
            comPCD[allMerge[eachPCD.size() - moreEach -moreLast+ i] - 1].num = 1;
            comPCD[allMerge[eachPCD.size() - moreEach -moreLast+ i] - 1].source.push_back(eachPCD.size() - moreEach -moreLast+ i + 1);
        }

    }else{
        for (int i = 0; i < moreLast+moreEach; i++)
        {
            if (allMerge[eachPCD.size() - moreEach -moreLast+ i] > comPCD.size())
            {
                comPCD.resize(allMerge[eachPCD.size() - moreEach-moreLast + i]);
            }
            //int bigNum_temp = allMerge[eachPCD.size() - moreEach + i] - 1;
            comPCD[allMerge[eachPCD.size() - moreEach -moreLast+ i] - 1].pcd.insert(comPCD[allMerge[eachPCD.size() - moreEach -moreLast+ i] - 1].pcd.end(), eachPCD_n[eachPCD.size() - moreEach -moreLast+ i].pcd.begin(), eachPCD_n[eachPCD.size() - moreEach -moreLast+ i].pcd.end());
            comPCD[allMerge[eachPCD.size() - moreEach -moreLast+ i] - 1].source.push_back(eachPCD.size() - moreEach -moreLast+ i + 1);
        }
    }


    //发送
    for (int i = 0; i < comPCD.size(); i++)
    {
        if (comPCD[i].num == -1)
        {
            std::vector<cv::Point3d> pcd = comPCD[i].pcd;
            PcdAll.resize(comPCD.size());
            PcdAll[i]=comPCD[i].pcd;
             std::vector<cv::Point3d> pcdmask = comPCDmask[i].pcd;
            std::string name  = srcToStr(comPCD[i].source,StringALL,NUMALL);
            WorkPieceNums.resize(comPCD.size());
            WorkPieceNums[i]=comPCD[i].source;
            workpiece_index = i + 1;
            string tempTxtPath = "/home/jiangnan/桌面/temp1/"+ to_string(workpiece_index)+"txt";
            ofstream tempTxt1(tempTxtPath);
            for(int tempi = 0;tempi<pcd.size();tempi++){
                tempTxt1<<pcd[tempi].x<<"   "<<pcd[tempi].y<<"   "<<pcd[tempi].z<<endl;
            }
            tempTxt1.close();
            emit one_workpiece_ready(workpiece_index, pcd, name,pcdmask);
            comPCD[i].num = -2;

        }
    }
    
   //  qDebug()<<"step11";
    savCOMMaskImg();
    savCOMImg();


    qDebug()<<"HebingThread::add_one_group end" << group_index;
    //    emit one_workpiece_ready(workpiece_index, pcd, name);
}

void HebingThread::prepare_to_rematch(std::vector<int> index_list, std::vector<std::string> char_list)
{
    Q_ASSERT(index_list.size() == char_list.size());

    for (uint i = 0; i < index_list.size(); ++i) {
        emit send_to_rematch(index_list.at(i),
                             PcdGet(index_list.at(i)),
                             char_list.at(i));
    }
}
void HebingThread::savCOMMaskImg()
{
//    static vector<int> comPCDMaskSaved;
//    static int comPCDMasksave = 1;
    vector<PCDcombine> comPCDMask_t;

    comPCDMask_t = comPCDmask;
    for (int i = 0; i < comPCDmask.size(); i++)
    {
        bool flg1 = 0;
        if ((!(comPCDmask[i].num == -1)) && (!(comPCDmask[i].num == -2) ))
        {
            continue;
        }
        for (int j = 0; j < comPCDMaskSaved.size(); j++)
        {
            if (i == comPCDMaskSaved[j])
            {
                flg1 = 1;
                break;
            }
        }
        if (flg1)
        {
            continue;
        }


        comPCDMaskSaved.push_back(i);

        if(comPCDmask[i].pcd.size()<10)
        {
            continue;
        }

        vector<Point3d> save_1;

        save_1.insert(save_1.end(), comPCDmask[i].pcd.begin(), comPCDmask[i].pcd.end());

        double xmin = save_1[0].x;
        double xmax = save_1[0].x;
        double ymin = save_1[0].y;
        double ymax = save_1[0].y;
        double xnow = 0, ynow = 0;

        for (int i = 0; i < save_1.size(); i++)
        {
            xnow = save_1[i].x;
            ynow = save_1[i].y;
            if (xnow < xmin)xmin = xnow;
            if (xnow > xmax)xmax = xnow;
            if (ynow < ymin)ymin = ynow;
            if (ynow > ymax)ymax = ynow;
        }

        for (int i = 0; i < save_1.size(); i++)
        {
            save_1[i].x = save_1[i].x - xmin;
            save_1[i].y = save_1[i].y - ymin;
        }

        int tuxmin = 0;
        int tuxmax = xmax - xmin + 1;
        int tuymin = 0;
        int tuymax = ymax - ymin + 1;
        Mat src = Mat::zeros(tuymax, tuxmax, CV_8UC1);
        for (int i = 0; i < save_1.size(); i++)
        {

            src.at<uchar>(int(save_1[i].y), int(save_1[i].x)) = 252;
            if((int(save_1[i].y-1)>0) &&(int(save_1[i].x-1)>0))
            src.at<uchar>(int(save_1[i].y-1), int(save_1[i].x-1)) = 252;
             if((int(save_1[i].y-1)>0) &&(int(save_1[i].x)>0))
             src.at<uchar>(int(save_1[i].y-1), int(save_1[i].x)) = 252;
              if((int(save_1[i].y-1)>0) &&(int(save_1[i].x+1)<tuxmax))
              src.at<uchar>(int(save_1[i].y-1), int(save_1[i].x+1)) = 252;
               if((int(save_1[i].y)>0) &&(int(save_1[i].x-1)>0))
               src.at<uchar>(int(save_1[i].y), int(save_1[i].x-1)) = 252;
                if((int(save_1[i].y)>0) &&(int(save_1[i].x+1)<tuxmax))
               src.at<uchar>(int(save_1[i].y), int(save_1[i].x+1)) = 252;
                if((int(save_1[i].y+1)<tuymax) &&(int(save_1[i].x-1)>0))
               src.at<uchar>(int(save_1[i].y+1), int(save_1[i].x-1)) = 252;
                if((int(save_1[i].y+1)<tuymax) &&(int(save_1[i].x)>0))
               src.at<uchar>(int(save_1[i].y+1), int(save_1[i].x)) = 252;
                if((int(save_1[i].y+1)<tuymax) &&(int(save_1[i].x+1)<tuxmax))
               src.at<uchar>(int(save_1[i].y+1), int(save_1[i].x+1)) = 252;


        }
        Mat three_channel = Mat::zeros(src.rows, src.cols, CV_8UC3);
        Mat src_zeros = Mat::zeros(src.rows, src.cols, CV_8UC1);
        vector<Mat> channels;
        channels.push_back(src_zeros);
        channels.push_back(src);
        channels.push_back(src_zeros);
        merge(channels, three_channel);
        QString path1= PP->save_path() + "/" +PP->str_time() + "/COMBINE/";
        string  path2= path1.toStdString();
//        imwrite(PP->save_path().toStdString() + "/" + PP->str_time().toStdString() + "/hfc/" + Common::num2str56(comPCDMasksave) + "号工件匹配结果图" + ".jpg", three_channel);
        imwrite(path2 +"MASK-"+ Common::num2str56(comPCDMasksave) + "-hebin结果图" + ".jpg", three_channel);
        comPCDMasksave++;
    }
}


void HebingThread::savCOMImg()
{
//    static vector<int> comPCDsaved;
//    static int comPCDsave = 1;
    vector<PCDcombine> comPCD_t;

    comPCD_t = comPCD;
    for (int i = 0; i < comPCD.size(); i++)
    {
        bool flg1 = 0;
        if ((!(comPCD[i].num == -1)) && (!(comPCD[i].num == -2) ))
        {
            continue;
        }
        for (int j = 0; j < comPCDsaved.size(); j++)
        {
            if (i == comPCDsaved[j])
            {
                flg1 = 1;
                break;
            }
        }
        if (flg1)
        {
            continue;
        }


        comPCDsaved.push_back(i);
        if(comPCD[i].pcd.size()<10)
        {
            continue;
        }

        vector<Point3d> save_1;

        save_1.insert(save_1.end(), comPCD[i].pcd.begin(), comPCD[i].pcd.end());

        double xmin = save_1[0].x;
        double xmax = save_1[0].x;
        double ymin = save_1[0].y;
        double ymax = save_1[0].y;
        double xnow = 0, ynow = 0;

        for (int i = 0; i < save_1.size(); i++)
        {
            xnow = save_1[i].x;
            ynow = save_1[i].y;
            if (xnow < xmin)xmin = xnow;
            if (xnow > xmax)xmax = xnow;
            if (ynow < ymin)ymin = ynow;
            if (ynow > ymax)ymax = ynow;
        }

        for (int i = 0; i < save_1.size(); i++)
        {
            save_1[i].x = save_1[i].x - xmin;
            save_1[i].y = save_1[i].y - ymin;
        }

        int tuxmin = 0;
        int tuxmax = xmax - xmin + 1;
        int tuymin = 0;
        int tuymax = ymax - ymin + 1;
        Mat src = Mat::zeros(tuymax, tuxmax, CV_8UC1);
        for (int i = 0; i < save_1.size(); i++)
        {

            src.at<uchar>(int(save_1[i].y), int(save_1[i].x)) = 252;
            if((int(save_1[i].y-1)>0) &&(int(save_1[i].x-1)>0))
            src.at<uchar>(int(save_1[i].y-1), int(save_1[i].x-1)) = 252;
             if((int(save_1[i].y-1)>0) &&(int(save_1[i].x)>0))
             src.at<uchar>(int(save_1[i].y-1), int(save_1[i].x)) = 252;
              if((int(save_1[i].y-1)>0) &&(int(save_1[i].x+1)<tuxmax))
              src.at<uchar>(int(save_1[i].y-1), int(save_1[i].x+1)) = 252;
               if((int(save_1[i].y)>0) &&(int(save_1[i].x-1)>0))
               src.at<uchar>(int(save_1[i].y), int(save_1[i].x-1)) = 252;
                if((int(save_1[i].y)>0) &&(int(save_1[i].x+1)<tuxmax))
               src.at<uchar>(int(save_1[i].y), int(save_1[i].x+1)) = 252;
                if((int(save_1[i].y+1)<tuymax) &&(int(save_1[i].x-1)>0))
               src.at<uchar>(int(save_1[i].y+1), int(save_1[i].x-1)) = 252;
                if((int(save_1[i].y+1)<tuymax) &&(int(save_1[i].x)>0))
               src.at<uchar>(int(save_1[i].y+1), int(save_1[i].x)) = 252;
                if((int(save_1[i].y+1)<tuymax) &&(int(save_1[i].x+1)<tuxmax))
               src.at<uchar>(int(save_1[i].y+1), int(save_1[i].x+1)) = 252;


        }
        Mat three_channel = Mat::zeros(src.rows, src.cols, CV_8UC3);
        Mat src_zeros = Mat::zeros(src.rows, src.cols, CV_8UC1);
        vector<Mat> channels;
        channels.push_back(src_zeros);
        channels.push_back(src);
        channels.push_back(src_zeros);
        merge(channels, three_channel);
        QString path1= PP->save_path() + "/" + PP->str_time() + "/COMBINE/";
        string  path2= path1.toStdString();
//        imwrite(PP->save_path().toStdString() + "/" + PP->str_time().toStdString() + "/hfc/" + Common::num2str56(comPCDsave) + "号工件匹配结果图" + ".jpg", three_channel);
        imwrite(path2 + "singal-"+Common::num2str56(comPCDsave) + "号工件hebin结果图" + ".jpg", three_channel);
        comPCDsave++;
    }
}

void HebingThread::savImg()
{
//    static int eachPCdsave = 1;
    vector<Point3d> save_1;
    for (int i = 0; i < eachPCD.size(); i++)
    {
        save_1.insert(save_1.end(), eachPCD[i].pcd.begin(), eachPCD[i].pcd.end());
    }

    double xmin = save_1[0].x;
    double xmax = save_1[0].x;
    double ymin = save_1[0].y;
    double ymax = save_1[0].y;
    double xnow = 0, ynow = 0;

    for (int i = 0; i < save_1.size(); i++)
    {
        xnow = save_1[i].x;
        ynow = save_1[i].y;
        if (xnow < xmin)xmin = xnow;
        if (xnow > xmax)xmax = xnow;
        if (ynow < ymin)ymin = ynow;
        if (ynow > ymax)ymax = ynow;
    }

    for (int i = 0; i < save_1.size(); i++)
    {
        save_1[i].x = save_1[i].x - xmin;
        save_1[i].y = save_1[i].y - ymin;
    }

    int tuxmin = 0;
    int tuxmax = xmax - xmin + 1;
    int tuymin = 0;
    int tuymax = ymax - ymin + 1;
    Mat src = Mat::zeros(tuymax, tuxmax, CV_8UC1);
    for (int i = 0; i < save_1.size(); i++)
    {
        src.at<uchar>(int(save_1[i].y), int(save_1[i].x)) = 252;
    }
    Mat three_channel = Mat::zeros(src.rows, src.cols, CV_8UC3);
    Mat src_zeros = Mat::zeros(src.rows, src.cols, CV_8UC1);
    vector<Mat> channels;
    channels.push_back(src_zeros);
    channels.push_back(src);
    channels.push_back(src_zeros);
    merge(channels, three_channel);
    QString path1= PP->save_path() + "/" + PP->str_time() + "/COMBINE/";
    string  path2= path1.toStdString();
    imwrite(path2 + "all-"+Common::num2str56(eachPCdsave) + "号合并图" + ".jpg", three_channel);
    eachPCdsave++;
}

string HebingThread::srcToStr(vector<int> PCDnums, vector<vector<string>> allstring, vector<vector<int>> stringmasknum)
{
    string outstring = "none";
    for (int i = 0; i < PCDnums.size(); i++)
    {
        if (outstring != "none")
            break;
        for (int j = 0; j < stringmasknum.size(); j++)
        {
            if (outstring != "none")
                break;
            for (int k = 0; k < stringmasknum[j].size(); k++)
            {
                if (PCDnums[i] == stringmasknum[j][k])
                {
                    if (allstring.size() > j)
                    {
                        if (allstring[j].size() > k)
                        {
//                            string tmp1 = "CG";
//                            string tmp2 = "GC";
//                            const char*show1;
//                            const char*show2;
//                            show1 = strstr(allstring[j][k].c_str(), tmp1.c_str());
//                            show2 = strstr(allstring[j][k].c_str(), tmp2.c_str()); //判断字符是否正确
//                            if (show1 != NULL || show2 != NULL)
//                            {
                                outstring = allstring[j][k]; //赋值
                                break;
//                            }
                        }
                    }
                }
            }
        }
    }

    return outstring;
}


vector<Mat> HebingThread::show_pics(int num)
{
    num--;
//    qDebug()<<" num=    "<<num;
    vector<Mat> Camera_Imgoutput;
    if(num < 0 || num >= WorkPieceNums.size())
        return Camera_Imgoutput;


    vector<Mat> CameraImg = PP->CameraImg_PFA();

//    for(int m = 0; m < NUMTOTALLSAVE.size(); m++)
//    {
//        qDebug()<<"      NUMTOTALLSAVE[m]=    "<<NUMTOTALLSAVE[m];
//    }



    vector<Mat> re_dec_tmp;
    for(int i = 0; i < WorkPieceNums[num].size(); i++)
    {
        int WorkPiceNum = WorkPieceNums[num][i];
//        qDebug()<<" i=    "<<i<<"      WorkPiceNum=    "<<WorkPiceNum;
//        for(int m = 0; m < NUMTOTALLSAVE.size(); m++)
//        {
//            qDebug()<<"      NUMTOTALLSAVE[m]=    "<<NUMTOTALLSAVE[m];
//        }


        for(int m = 0; m < NUMTOTALLSAVE.size(); m++)
        {
            if(WorkPiceNum<= NUMTOTALLSAVE[0])
            {
                Mat reMask = MaskPicALL[m];

//                qDebug()<<" i=  "<<i<<"   WorkPiceNum=    "<<WorkPiceNum<<"m:     "<<m<<" MaskPicALL[m]:    "<< NUMTOTALLSAVE[m];

                Mat reMask0 = Mat::zeros(1200, 1920, CV_16UC1);
                for (int row = 0; row < reMask.rows; row++)
                {
                    for (int col = 0; col < reMask.cols; col++)
                    {

                        if (reMask.at<ushort>(row, col) == WorkPiceNum)
                        {
                            reMask0.at<ushort>(row, col) = 1;
                        }
                    }
                }
                reMask0.convertTo(reMask0, CV_8UC1, 255);
//                String ll =to_string(i);
//                imwrite("/home/jiangnan/图片/0519/"+ll+".bmp",reMask0);
                reMask = reMask0 / 255;
                Mat imgOrigin = CameraImg[m];
//                String ll =to_string(i);
//                imwrite("/home/jiangnan/图片/0519/"+ll+".bmp",imgOrigin);
                Mat reImage = Mat::zeros(imgOrigin.rows, imgOrigin.cols, CV_8UC3);
                Mat channels[3];
                split(imgOrigin, channels);
                channels[0] = channels[0].mul(reMask);
                channels[1] = channels[1].mul(reMask);
                channels[2] = channels[2].mul(reMask);
                merge(channels, 3, reImage);
                re_dec_tmp.push_back(reImage);
                break;
            }
            if((WorkPiceNum > NUMTOTALLSAVE[m])&&(WorkPiceNum <= NUMTOTALLSAVE[m+1]))
            {
//                qDebug()<<" i=  "<<i<<"   WorkPiceNum=    "<<WorkPiceNum<<"m:     "<<m+2<<" MaskPicALL[m]:    "<< NUMTOTALLSAVE[m+1];
//                qDebug()<<" i=  "<<i<<"   WorkPiceNum=    "<<WorkPiceNum;

                if(m+1==NUMTOTALLSAVE.size())
                    break;
                Mat reMask = MaskPicALL[m+1];
                Mat reMask0 = Mat::zeros(1200, 1920, CV_16UC1);
                for (int row = 0; row < reMask.rows; row++)
                {
                    for (int col = 0; col < reMask.cols; col++)
                    {

                        if (reMask.at<ushort>(row, col) == WorkPiceNum)
                        {
                            reMask0.at<ushort>(row, col) = 1;
                        }
                    }
                }
                reMask0.convertTo(reMask0, CV_8UC1, 255);
//                String ll =to_string(i);
//                imwrite("/home/jiangnan/图片/0519/"+ll+".bmp",reMask0);
                reMask = reMask0 / 255;
                Mat imgOrigin = CameraImg[m+1];
//                String ll =to_string(i);
//                imwrite("/home/jiangnan/图片/0519/"+ll+".bmp",imgOrigin);
                Mat reImage = Mat::zeros(imgOrigin.rows, imgOrigin.cols, CV_8UC3);
                Mat channels[3];
                split(imgOrigin, channels);
                channels[0] = channels[0].mul(reMask);
                channels[1] = channels[1].mul(reMask);
                channels[2] = channels[2].mul(reMask);
                merge(channels, 3, reImage);
                re_dec_tmp.push_back(reImage);
                break;
            }
        }

//        for(int m = 0; m < NUMALL.size(); m++)
//        {
//            for(int n = 0; n < NUMALL[m].size(); n++)
//            {
//                if(WorkPiceNum == NUMALL[m][n])
//                {
////                    qDebug()<<" m=    "<<m<<"      n=    "<<n;
//                    Mat reMask = MaskPicALL[m];
//                    Mat reMask0 = Mat::zeros(1200, 1920, CV_16UC1);
//                    for (int row = 0; row < reMask.rows; row++)
//                    {
//                        for (int col = 0; col < reMask.cols; col++)
//                        {

//                            if (reMask.at<ushort>(row, col) == WorkPiceNum)
//                            {
//                                reMask0.at<ushort>(row, col) = 1;
//                            }
//                        }
//                    }
//                    reMask0.convertTo(reMask0, CV_8UC1, 255);
//                    reMask = reMask0 / 255;
//                    Mat imgOrigin = CameraImg[m];
//                    Mat reImage = Mat::zeros(imgOrigin.rows, imgOrigin.cols, CV_8UC3);
//                    Mat channels[3];
//                    split(imgOrigin, channels);
//                    channels[0] = channels[0].mul(reMask);
//                    channels[1] = channels[1].mul(reMask);
//                    channels[2] = channels[2].mul(reMask);
//                    merge(channels, 3, reImage);
//                    re_dec_tmp.push_back(reImage);
//                    break;
//                }
//            }
//        }

    }

// qDebug()<<re_dec_tmp.size();
// for(int kk = 0; kk < re_dec_tmp.size(); kk++)
// {
//     Mat a = re_dec_tmp[kk];
//     qDebug()<<a.rows<<"     "<<a.cols;
//     String ll =to_string(kk);
//     imwrite("/home/jiangnan/图片/0519/"+ll+".bmp",a);
// }

#ifdef FULLY_BUILD

    Camera_Imgoutput = CCC->Eastoutput(re_dec_tmp);

#endif

    return Camera_Imgoutput;
}

vector<Point3d> HebingThread::PcdGet(int num)
{
    num--;
    vector<Point3d> PcdRematch;
    if(num < PcdAll.size())
        PcdRematch = PcdAll[num];
    return PcdRematch;

}
