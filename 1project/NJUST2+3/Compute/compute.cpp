#include "compute.h"
#include "Server/parameter.h"
#include "common.h"


#include <QTime>

#define TEST 1

Compute::Compute()
{

}

void Compute::init()
{
    qDebug()<<"Compute::init!";

#if !TEST
    //    S_DecCnt = 0;//记录处理图像数=拍照数*相机数

    WP = new Weld_Processing;
    Md = new Mask_Detector;
    CCC = new character;
    PM = new PointCloud_match;
    POP = new Points_Processing;
    WOP = new Weld_Only_Processing;
#endif
}

void Compute::reload()
{
    qDebug()<<"Compute::reload!";

#if !TEST

    //network reload--------------------------------------
    WP->Net4();
    Md->Net3();
     WOP->Net2();
    POP->Net1();

    CCC->load_ssd();
    CCC->load_east_net();




    //params reload
    Mask_String.erase(Mask_String.begin(),Mask_String.end());
    Numbered_hanfengs.erase(Numbered_hanfengs.begin(),Numbered_hanfengs.end());
    Numbered_Masks.erase(Numbered_Masks.begin(),Numbered_Masks.end());
    getGongJian.erase(getGongJian.begin(),getGongJian.end());
    Dec_Strings.erase(Dec_Strings.begin(),Dec_Strings.end());
    Dec_Points.erase(Dec_Points.begin(),Dec_Points.end());

    PointCloud_3d.erase(PointCloud_3d.begin(),PointCloud_3d.end());
    matchSucceedOrNot.erase(matchSucceedOrNot.begin(),matchSucceedOrNot.end());
    point4dShow.erase(point4dShow.begin(),point4dShow.end());
    camIndexPoint.erase(camIndexPoint.begin(),camIndexPoint.end());
    jsonIndexPoint.erase(jsonIndexPoint.begin(),jsonIndexPoint.end());

    log_path=QString("/Comupute_Log_%1.txt").arg(QTime::currentTime().toString());

    deal_one_group_count=0;
    kkkkk=0;

    QFile file(PP->save_path() + "/" + PP->str_time() + log_path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Append))
        return;

    QTextStream out(&file);
    out<<"A new detection start!\n\n\n";

    file.flush();
    file.close();

#endif
}

QVector<QVector4D> Compute::point_cloud()
{
    QVector<QVector4D> mesh;

#if TEST
    //测试
    QFile file("/home/zdit/work/point4ds_txt2.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return mesh;

    int j = 0;

    while (!file.atEnd()) {
        QList<QByteArray> list = file.readLine().simplified().split(' ');
        Q_ASSERT(list.count() >= 4);
        j++;
        if(j % 5 != 0){
            continue;
        }
        QVector4D v;

        v.setX(list.at(0).toFloat());
        v.setY(list.at(1).toFloat());
        v.setZ(list.at(2).toFloat());
        v.setW(list.at(3).toFloat());
        mesh.append(v);
    }
#else
    for (uint i = 0; i < point4dShow.size(); ++i) {
        QVector4D v;
        v.setX(point4dShow.at(i).x);
        v.setY(point4dShow.at(i).y);
        v.setZ(point4dShow.at(i).z);
        v.setW(point4dShow.at(i).indexGongjian);
        mesh.append(v);
    }
#endif
    return mesh;
}

QStringList Compute::character_list()
{
    QStringList str_list;

#if TEST
    //测试
    str_list << "AAA" << "BBB" << "CCC" << "0"
             << "0" << "FFF" << "GGG";
#else
    for (uint i = 0; i < Mask_String.size(); ++i) {
        str_list.append(QString::fromStdString(Mask_String.at(i)));
    }
#endif
    return str_list;
}

void Compute::set_character_list(QStringList list)
{
    Mask_String.clear();
    foreach (QString s, list) {
        Mask_String.push_back(s.toStdString());
    }
}

QVector<int> Compute::result_list()
{
    QVector<int> result;

#if TEST
    //测试
    for (int i = 0; i < 7; ++i) {
        result.append(1);
    }
    result[4] = 0;
#else
    for (uint i = 0; i < matchSucceedOrNot.size(); ++i) {
        result.append(matchSucceedOrNot.at(i));
    }
#endif
    return result;
}

void Compute::set_result_list(QVector<int> list)
{
    matchSucceedOrNot.clear();
    matchSucceedOrNot = list.toStdVector();
}

void Compute::deal_one_group(vector<Mat> DecImg)
{
    qDebug()<<"start deal_one_group"<<deal_one_group_count+1<<endl;
#if !TEST

    QFile file(PP->save_path() + "/" + PP->str_time() + log_path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Append))
        return;

    QTextStream out(&file);

    deal_one_group_count++;
    out<<"the group of pictures :      "<<deal_one_group_count<<"   is in detection!";
    qDebug()<<"the group of pictures :      "<<deal_one_group_count<<"   is in detection!";


    vector<int>  cameraMap;
    cameraMap=PP->cameraMap();
    //焊缝外轮廓，mask以及字符串检测====================================================================================================
    //==============================================================================================================================
    //字符检测前对单次拍摄图片取ROI，DecImg中为原图----------------
    vector<Mat> origiRoi;
    vector<Point2f> centerpointout;
    vector<string> alltextout;
    origiRoi=takeROI(DecImg, cameraMap);
    qDebug()<<"DecImg.size()"<<DecImg.size();

    out<<"      DecImg.size():      "<<DecImg.size()<<"\n";
    qDebug()<<"      DecImg.size():      "<<DecImg.size()<<"\n";

    for(uint i=0;i<DecImg.size();i++)
    {
        Mat imgIn=DecImg[i];

        out<<"the number of proup :   "<<i+1<<"   is in detection!\n";
        qDebug()<<"the number of proup :   "<<i+1<<"   is in detection!\n";
        out<<"DecIMG.row:   "<<DecImg[i].rows<<"       ";
        qDebug()<<"DecIMG.row:   "<<DecImg[i].rows<<"       ";
        out<<"DecIMG.col:   "<<DecImg[i].cols<<"\n";
        qDebug()<<"DecIMG.col:   "<<DecImg[i].cols<<"\n";




        //mask，焊缝外轮廓检测-------------------------------------

        double tWeldDeteStart = (double)getTickCount();
        Mat weldDete = WP->Img_detector(imgIn);
        out<<"WELD detect done!\n";
        qDebug()<<"WELD detect done!\n";

        double tWeldDeteEnd = (double)getTickCount();
        qDebug()<<"WELD detect Time:"<<(tWeldDeteEnd - tWeldDeteStart) * 1000.0 / (getTickFrequency())<<"ms\n";

        double tMaskDeteStart = (double)getTickCount();
        Mat maskDete = Md->Img_detector(imgIn);
        out<<"MASK detect done!\n";
        qDebug()<<"MASK detect done!\n";

        double tMaskDeteEnd = (double)getTickCount();
        qDebug()<<"Mask detect Time:"<<(tMaskDeteEnd - tMaskDeteStart) * 1000.0 / (getTickFrequency())<<"ms\n";

        //依次存入mask，焊缝外轮廓检测图----------------------------
        weldDeteOut.push_back(weldDete);
        out<<"WELD saved!\n";
        qDebug()<<"WELD saved!\n";
        maskDeteOut.push_back(maskDete);
        out<<"MASK saved!\n";
        qDebug()<<"MASK saved!\n";

        //字符串检测----------------------------------------------
        qDebug()<<"Chara_Detec start!\n";

        //imwrite("/home/fenglin/图片/tmp000/yuantu.jpg",origiRoi[i]);
        double tCharcDeteStart = (double)getTickCount();
        CCC->Chara_Detec(origiRoi[i], alltextout, centerpointout);
        out<<"Chara_Detec done!\n";
        qDebug()<<"Chara_Detec done!\n";


        double tCharcDeteEnd = (double)getTickCount();
        qDebug()<<"Chara detect Time:"<<(tCharcDeteEnd - tCharcDeteStart) * 1000.0 / (getTickFrequency())<<"ms\n";

//        testzbar();




        //依次存入每张图检测到的字符串及对应坐标-----------------------
        qDebug()<<"Dec_Strings start!\n";
        Dec_Strings.push_back(alltextout);
        out<<"Dec_Strings saved!\n";
        qDebug()<<"Dec_Strings saved!\n";
        Dec_Points.push_back(centerpointout);
        out<<"Dec_Points saved!\n";
        qDebug()<<"Dec_Points saved!\n";
    }


    out<<"THIS GROUP HAS DONE!!!\n\n";
    qDebug()<<"THIS GROUP HAS DONE!!!\n\n";


    file.flush();
    file.close();
#endif
}

void Compute::deal_hebing()
{
#if !TEST

    QString pathSave=PP->save_path()+"/"+PP->str_time();
    QDir dir;
    dir.mkpath(pathSave);
    QString pathSave_deResults=pathSave+"/detected_results";
    dir.mkpath(pathSave_deResults);
    QString pathSave_WELD=pathSave_deResults+"/WELD";
    dir.mkpath(pathSave_WELD);
    QString pathSave_MASK=pathSave_deResults+"/MASK";
    dir.mkpath(pathSave_MASK);
    QString pathSave_hfc=pathSave+"/hfc";
    dir.mkpath(pathSave_hfc);
    QString pathSave_pointsMatch=pathSave+"/pointsMatch";
    dir.mkpath(pathSave_pointsMatch);
    QString pathSave_points_OriginPics=pathSave_pointsMatch+"/OriginPics";
    dir.mkpath(pathSave_points_OriginPics);
    //    QString pathSave_points_detected_PointsPics=pathSave_pointsMatch+"/pointsDetectedPics";
    //    dir.mkpath(pathSave_points_detected_PointsPics);
    //    QString pathSave_points_weldDetectedPics=pathSave_pointsMatch+"/weldDetectedPics";
    //    dir.mkpath(pathSave_points_weldDetectedPics);
    QString pathSave_points_pointsMatchPics=pathSave_pointsMatch+"/pointsMatchPics";
    dir.mkpath(pathSave_points_pointsMatchPics);
    QString pathSave_points_weldMatchPics=pathSave_pointsMatch+"/weldMatchPics";
    dir.mkpath(pathSave_points_weldMatchPics);




    QFile file(PP->save_path() + "/" + PP->str_time() + log_path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Append))
        return;

    QTextStream out(&file);
    out<<"Deal_hebing start!\n";
    qDebug()<<"Deal_hebing start!\n";
    out<<"Deal_hebing start!\n";
    out<<"Deal_hebing start!\n";




    int MaxBianhao=0;
    vector<Mat> weldDeteRoiOut;
    vector<Mat> maskDeteRoiOut;
    vector<Point2f> centerpointout;
    vector<string> alltextout;
    vector<int>  cameraMap;
    cameraMap=PP->cameraMap();
    double L_shift = PP->l_shift();               //机器人坐标偏移量
    int USE_CAM_NUM=cameraMap.size();
    vector<double> Longmen_x_s=PP->Longmen_x_s();
    //验证马工接口-----------------------------------------------------------------------------------------
    vector<double> Longmen_x_s1=PP->Longmen_x_s(1);
    vector<double> Longmen_x_s2=PP->Longmen_x_s(2);
    vector<double> Longmen_x_s3=PP->Longmen_x_s(3);
    out<<"check Longmen_x_s(1)(2)(3):     \n";
    for(int qqq=0;qqq<Longmen_x_s1.size();qqq++)
    {
        out<<"Longmen_x_s1:       \n";
        double www=Longmen_x_s1[qqq];
        out<<www<<"\n";
    }
    for(int qqq=0;qqq<Longmen_x_s2.size();qqq++)
    {
        out<<"Longmen_x_s2:       \n";
        double www=Longmen_x_s2[qqq];
        out<<www<<"\n";
    }
    for(int qqq=0;qqq<Longmen_x_s3.size();qqq++)
    {
        out<<"Longmen_x_s3:       \n";
        double www=Longmen_x_s3[qqq];
        out<<www<<"\n";
    }




    //Time

    double timeStart,timeEnd;
    timeStart = (double)clock();



    //验证马工接口-----------------------------------------------------------------------------------------
    int MAX_CAM_CRACK =PP->camera_shot_times();
    vector<Mat> CameraImg_PFA=PP->CameraImg_PFA();
    gongjian_hebing GH;
    //对焊缝外轮廓以及mask检测图片取ROI，weldDeteOut中为检测原图-----
    weldDeteRoiOut = takeROI(weldDeteOut, cameraMap);
    maskDeteRoiOut = takeROI(maskDeteOut, cameraMap);
    weldDeteOut.erase(weldDeteOut.begin(),weldDeteOut.end());
    maskDeteOut.erase(maskDeteOut.begin(),maskDeteOut.end());
    out<<"MASK+WELD ROI has done!\n";

    timeEnd = (double)clock();

    qDebug()<<"pre process time:"<<(timeEnd - timeStart)/CLOCKS_PER_SEC<<"s \n";

    //MASK&焊缝编号-------------------------------------------------------
    //=========================================================================================
    //weldDeteRoiOut, maskDeteRoiOut, Dec_Strings, Dec_Points依次为检测得到的焊缝、mask、字符串、坐标输入
    //Numbered_hanfengs：带编号的焊缝图，其中编号为合并后的新编号， Numbered_Masks：带编号的mask图，其中编号为合并前的编号
    //getGongJian为原始工件合并后依次对应的新编号，MaxBianhao为合并后最大的工件编号
    //// Mask_String为不同ID的工件依次对应的字符串（供UI调用）
    ///point4dShow为输出的四维点云（供UI调用）
    ///
    timeStart = (double)clock();
    //=================================================================================
    GH.numberAndMerge(weldDeteRoiOut, maskDeteRoiOut, Dec_Strings, Dec_Points,
                      Numbered_hanfengs, Numbered_Masks, Mask_String,Longmen_x_s, L_shift, getGongJian,
                      USE_CAM_NUM, MAX_CAM_CRACK, MaxBianhao, cameraMap, point4dShow);

    out<<"NumberAndMerge has done!\n";
    timeEnd = (double)clock();
    qDebug()<<"NumberAndMerge:"<<(timeEnd - timeStart)/CLOCKS_PER_SEC<<"s\n";


    qDebug()<<"合并后最大的工件编号"<<MaxBianhao;
    qDebug()<<"原始工件合并后依次对应的新编号"<<getGongJian;

    //点云数据赋值：工件-图片-点云-----
    timeStart = (double)clock();
    PointCloud_3d = PM->PointCloud_3d_GivenData(MaxBianhao, Numbered_hanfengs, getGongJian);
    out<<"PointCloud_3d_GivenData has done!\n";
    out<<"PointCloud_3d.size():     "<<PointCloud_3d.size()<<"\n\n";
    //    cout<<"PointCloud_3d.size()"<<PointCloud_3d.size()<<endl;
    //matchSucceedOrNot.resize(MaxBianhao);     //matchSucceedOrNot存储匹配结果，1：成功、0：失败
    matchSucceedOrNot.resize(MaxBianhao,0);     //matchSucceedOrNot存储匹配结果，1：成功、0：失败

    timeEnd = (double)clock();
    qDebug()<<"PointCloud_3d_GivenData time:"<<(timeEnd - timeStart)/CLOCKS_PER_SEC<<"s\n";

    out<<"Work Piece start re_detect_character!!!\n";
    //工件合并后增加一轮字符识别过程
    double tCCStart,tCCEnd;

    int reCCNum = 0;
    for (int j=0; j < Mask_String.size(); j++)
    {
        if (Mask_String[j] == "0")      //没有检测出字符就重新检测
        {
            //            cout<<"kaishi ercipipei11111"<<endl;
            out<<j<<"   Work Piece need re_detect_character!\n";
            vector<Mat> reDetectPictures = PM->ShowPictures(j, Numbered_Masks, CameraImg_PFA);      //给出重新检测小图，已经裁剪旋转
            vector<vector<string>> reDec_Strings;
            for (int k = 0; k < reDetectPictures.size(); k++)       //输入工件小图，输出二次识别的字符结果
            {
                tCCStart = (double)clock();
                CCC->Chara_Detec(reDetectPictures[k], alltextout, centerpointout);
                reDec_Strings.push_back(alltextout);
                reCCNum++;
                tCCEnd = (double)clock();
                qDebug()<<reCCNum<<"二次识别 time:"<<(tCCEnd - tCCStart)/CLOCKS_PER_SEC<<"s\n";
                //alltextout.clear();
            }
            Mask_String[j]= PM->maskStringRegiven(reDec_Strings);
            out<<j<<"   Work Piece re_detect_character has done!\n";
        }
        //        cout<<"kaishi ercipipei2222222222"<<endl;

    }
    out<<"Work Piece re_detect_character has done!!!\n\n";
    for (int ms=0;ms<Mask_String.size();ms++)
    {
        cout <<"二次识别num"<<ms<<"bianhao:              "<< Mask_String[ms] << endl;
    }


   qDebug()<<"re Character num:"<<reCCNum<<'\n';

    out<<"Deal_hebing end!\n";
    out<<"Deal_hebing end!\n";
    out<<"Deal_hebing end!\n\n\n";


    file.flush();
    file.close();

#endif
}

void Compute::deal_Match()
{
    //    QFile file(PP->save_path() + "/" + PP->str_time() + log_path);
    //    if (!file.open(QIODevice::WriteOnly | QIODevice::Append))
    //        return;

    //    QTextStream out(&file);
    //    out<<"deal_Match\n";
    //    out<<"deal_Match\n";
    //    out<<"deal_Match\n\n\n";



    vector<vector<Point3d>> weldData;
    int send_ID;
    //    qDebug()<<"deal_Match:"<<matchSucceedOrNot;
    //    for (uint i = 0; i < Mask_String.size(); ++i) {
    //        qDebug()<<QString::fromStdString(Mask_String.at(i));
    //    }
    vector<int>  cameraMap;
    cameraMap=PP->cameraMap();
    double L_shift = PP->l_shift();               //机器人坐标偏移量
    // qDebug()<<"deal_Match:"<<matchSucceedOrNot;
    // for (uint i = 0; i < Mask_String.size(); ++i) {
    //     qDebug()<<QString::fromStdString(Mask_String.at(i));
    // }
    //    qDebug()<<"deal_Match:"<<Mask_String;

    //    PointCloud_match PM;
    vector<double> Longmen_x_s=PP->Longmen_x_s();
    vector<Mat> CameraImg_PFA=PP->CameraImg_PFA();

    //进行工件实际点云和标准点云匹配-----

#if !TEST
    for (int i = 0; i < PointCloud_3d.size(); i++)
    {
        //        //发送开始匹配信号
        //        emit start_match(i+1);

        cout<<"Mask_String"<<i<<Mask_String[i]<<endl;
        int match_or_not = 0;
        if (Mask_String[i] == "-1")     //zi fu jiao yan shi shan chu le
        {
            //cout<<"deleted so continue"<<endl;
            continue;
        }
        int pointCloudCheck = PM->PointCloud_check( PointCloud_3d[i], Mask_String[i]);     //判定该点云是否足够大且有名称
        if (pointCloudCheck == 0)
        {
            matchSucceedOrNot[i] = match_or_not;
            continue;
        }

        //发送开始匹配信号
        emit start_match(i+1);

        //获取工件标准点云路径-----
        char Path[200];
        memset(Path,'\0',200);
        string Path_tmp = PM->Get_Path(Mask_String[i]);     //根据工件字符，给出工件的标准模型的路径
        strcpy(Path,Path_tmp.c_str());
        if (Path[0] == '\0')     //没有找到路径时，即没有此编码工件的标准模型，跳出此次循环
        {
            matchSucceedOrNot[i] = match_or_not;

            cout << "No Model and StartPointMatch!" << endl;
            //start_point_check(i+1);      //开始点检测
            kkkkk++;
            vector<int> pointsPicsNum;     //picsNum,to install originnal pics's num in all pics
            vector<int> workpieceNUMinPic;     //to install the num of the workpiece in the picture
            vector<Mat> pointsOriginPics = PM->ShowOriginalPictures(i, CameraImg_PFA,pointsPicsNum,workpieceNUMinPic);     //give oringinnal pics for later activities, pointsPicsNum is important
            vector<Mat> weldDetectedPics;
            vector<Mat> detected_PointsPics;
            for(int q=0;q<pointsOriginPics.size();q++)//kan yi xia jian ce jie guo tu
            {
                Mat img_origin=pointsOriginPics[q];
                Mat img_detected=POP->Img_detector(img_origin);
                Mat img_weld_detected=WOP->Img_detector(img_origin);
                detected_PointsPics.push_back(img_detected);
                weldDetectedPics.push_back(img_weld_detected);
                imwrite(PP->save_path().toStdString() + "/" + PP->str_time().toStdString() + "/pointsMatch/OriginPics/WorkPiece" +Common::num2str1(kkkkk)+"-"+Common::num2str1(q)+".jpg", pointsOriginPics[q]);
            }
            vector<Mat> pointsMatchPics= PM->ShowPointPictures(Numbered_Masks,  detected_PointsPics, pointsPicsNum,workpieceNUMinPic);
            vector<Mat> weldMatchPics=PM->ShowPointPictures(Numbered_Masks,  weldDetectedPics, pointsPicsNum,workpieceNUMinPic);

            for(int q=0;q<pointsMatchPics.size();q++)//kan yi xia jian ce jie guo tu
            {
                imwrite(PP->save_path().toStdString() + "/" + PP->str_time().toStdString() +"/pointsMatch/pointsMatchPics/WorkPiece" +Common::num2str1(kkkkk)+"-"+Common::num2str1(q)+".jpg", pointsMatchPics[q]);
            }
            for(int q=0;q<weldMatchPics.size();q++)//kan yi xia jian ce jie guo tu
            {
                imwrite(PP->save_path().toStdString() + "/" + PP->str_time().toStdString() +"/pointsMatch/weldMatchPics/WorkPiece" +Common::num2str1(kkkkk)+"-"+Common::num2str1(q)+".jpg", weldMatchPics[q]);
            }

            vector<vector<Point2d>> preLines;
            preLines =PM->pointmatch(weldData,i,pointsMatchPics,pointsPicsNum,cameraMap,weldMatchPics,Longmen_x_s,L_shift);

            //point_check_success(i+1);    //点检测成功信号
            cout << "匹配成功；" << endl;

//            send_ID=i;
//            //计算焊缝1
//            std::string strPart = "POINT-MATCH-SEND"+Mask_String[send_ID] + "_" + Common::num2str1(send_ID);
//            emit send_weldData(weldData, strPart);
//            weldData.clear();
//            match_or_not=9;
//            matchSucceedOrNot[i] = match_or_not;

//            cout << "No Model and continue" << endl;
            continue;
        }

        //将实际点云和标准点云模型进行匹配,提取焊缝数值赋值给weldData-----

        cout<<"检测工件:     "<<Mask_String[i]<<endl;
        match_or_not = PM->Match_and_GetWeldData(i, PointCloud_3d[i], Path, weldData, L_shift, Longmen_x_s, cameraMap, camIndexPoint, jsonIndexPoint);

#else
    for (int i = 0; i < 7; i++)
    {
        //发送开始匹配信号
        emit start_match(i+1);
        QThread::sleep(4);               //
        int match_or_not = i;
#endif

        if(match_or_not==1)
        {
            //发送匹配成功信号
            emit match_success(i+1);
            cout << "匹配成功；" << endl;
            send_ID=i;
            //计算焊缝1
            std::string strPart = Mask_String[send_ID] + "_" + Common::num2str1(send_ID);
            emit send_weldData(weldData, strPart);
            weldData.clear();
            match_or_not=9;
            matchSucceedOrNot[i] = match_or_not;
        }
#if 0
        else
        {
            vector<Mat> re_dec_Mat = PM.ShowPictures(i, Numbered_Masks, CameraImg_PFA);       //跟匹配失败的工件有关的图片

            for (int k = 0; k < re_dec_Mat.size(); k++)       //二次检测，输入工件小图，输出二次识别的字符结果
            {
                CCC->Chara_Detec(re_dec_Mat[k], centerpointout, alltextout);
                reDec_Strings.push_back(alltextout);
                alltextout.clear();
            }

            match_or_not = PM.Rematch_PointCloud(i, reDec_Strings, Mask_String[i], PointCloud_3d[i], weldData, L_shift, Longmen_x_s, cameraMap);

            if (match_or_not == 1)
            {
                matchSucceedOrNot[i] = match_or_not;
                cout << "匹配成功；" << endl;
                send_ID=i;
                emit SendWeldseamData();              //发送数据
                weldData.clear();
            }

            if (match_or_not == 0)
            {
                matchSucceedOrNot[i] = match_or_not;
                cout << "该工件多次匹配仍失败，请人工寻找原因！" << endl;
            }
        }
#endif


        else
        {
            cout << "SatndardMatch Failed and StartPointMatch!" << endl;
            //start_point_check(i+1);      //开始点检测
            kkkkk++;
            vector<int> pointsPicsNum;     //picsNum,to install originnal pics's num in all pics
            vector<int> workpieceNUMinPic;     //to install the num of the workpiece in the picture
            vector<Mat> pointsOriginPics = PM->ShowOriginalPictures(i, CameraImg_PFA,pointsPicsNum,workpieceNUMinPic);     //give oringinnal pics for later activities, pointsPicsNum is important
            vector<Mat> weldDetectedPics;
            vector<Mat> detected_PointsPics;
            for(int q=0;q<pointsOriginPics.size();q++)//kan yi xia jian ce jie guo tu
            {
                Mat img_origin=pointsOriginPics[q];
                Mat img_detected=POP->Img_detector(img_origin);
                Mat img_weld_detected=WOP->Img_detector(img_origin);
                detected_PointsPics.push_back(img_detected);
                weldDetectedPics.push_back(img_weld_detected);
                imwrite(PP->save_path().toStdString() + "/" + PP->str_time().toStdString() + "/pointsMatch/OriginPics/WorkPiece" +Common::num2str1(kkkkk)+"-"+Common::num2str1(q)+".jpg", pointsOriginPics[q]);
            }
            vector<Mat> pointsMatchPics= PM->ShowPointPictures(Numbered_Masks,  detected_PointsPics, pointsPicsNum,workpieceNUMinPic);
            vector<Mat> weldMatchPics=PM->ShowPointPictures(Numbered_Masks,  weldDetectedPics, pointsPicsNum,workpieceNUMinPic);

            for(int q=0;q<pointsMatchPics.size();q++)//kan yi xia jian ce jie guo tu
            {
                imwrite(PP->save_path().toStdString() + "/" + PP->str_time().toStdString() +"/pointsMatch/pointsMatchPics/WorkPiece" +Common::num2str1(kkkkk)+"-"+Common::num2str1(q)+".jpg", pointsMatchPics[q]);
            }
            for(int q=0;q<weldMatchPics.size();q++)//kan yi xia jian ce jie guo tu
            {
                imwrite(PP->save_path().toStdString() + "/" + PP->str_time().toStdString() +"/pointsMatch/weldMatchPics/WorkPiece" +Common::num2str1(kkkkk)+"-"+Common::num2str1(q)+".jpg", weldMatchPics[q]);
            }

            vector<vector<Point2d>> preLines;
            preLines =PM->pointmatch(weldData,i,pointsMatchPics,pointsPicsNum,cameraMap,weldMatchPics,Longmen_x_s,L_shift);

            //point_check_success(i+1);    //点检测成功信号
            cout << "匹配成功；" << endl;
        }


//        else
//        {
//            //发送匹配失败信号
//            emit match_failed(i+1);

//            //matchSucceedOrNot[i] = match_or_not;
//            cout << "\n匹配失败；" << endl;
//        }
    }

    cout << "\n检测完成" << endl;


    qDebug()<<"matchSucceedOrNot:"<<matchSucceedOrNot;

    //    out<<"deal_Match end!\n";
    //    out<<"deal_Match end!\n";
    //    out<<"deal_Match end!\n\n\n";


    //    file.flush();
    //    file.close();
}

vector<Mat> Compute::show_pics(int num)
{
#if TEST
    vector<Mat> v;
    for (int i = 0; i < 3; ++i) {
        QString str = PP->offline_path() + "/camera" + QString::number(i+1) + "-" + QString::number(num)
                + "----.bmp";
        cv::Mat PFA_img = cv::imread(str.toStdString());        //读取
        v.push_back(PFA_img);
    }
    return v;
#else
    //    return PointCloud_match::ShowPictures(int i, vector<Mat> Numbered_Masks, vector<Mat> CameraImg_PFA);
    vector<Mat> CameraImg_PFA=PP->CameraImg_PFA();
    vector<Mat> v = PM->ShowPicturesToSee(num, Numbered_Masks, CameraImg_PFA);
    vector<Mat> Camera_Imgoutput;
    Camera_Imgoutput = CCC->Eastoutput(v);
    return Camera_Imgoutput;
    //        return v;
#endif
}

void Compute::P_weld_send()
{
    //添加处理代码

}

vector<Mat> Compute::takeROI(vector<Mat> imgIn, vector<int> usedCamID)
{
    int MAX_CAM_NUM=PP->max_cam_num();
    int usedCanNum = usedCamID.size();
    Mat Roi = Mat::zeros(imgIn[0].rows, imgIn[0].cols, imgIn[0].type());
    vector<Mat> imgRoi;
    for (uint i = 0; i < imgIn.size(); i++)
    {
        Mat imgI = imgIn[i];
        int oneCrack_pos = i % usedCanNum;
        //        if ((oneCrack_pos == 0) && (usedCamID[oneCrack_pos] == 1))
        if ((oneCrack_pos == 0) && (PP->is_first_in_edge()))
        {
            //            Rect rect(1070, 100, 750, 1000);
            //            610,100,1110,1000---------------------new
            Rect rect(PP->ROI_first().x(), PP->ROI_first().y(), PP->ROI_first().width(), PP->ROI_first().height());
            Mat Tmp = imgI(rect);
            Mat src = Mat::zeros(imgI.rows, imgI.cols, imgIn[0].type());
            Mat ROI = src(rect);
            Tmp.copyTo(ROI);
            Roi = src.clone();
        }



        else if ((oneCrack_pos == usedCanNum - 1) && (PP->is_last_in_edge()))
        {
            //            Rect rect(100, 100, 700, 1000);
            //            100,100,1200,1000---------------------new
            Rect rect(PP->ROI_middle().x(), PP->ROI_middle().y(), PP->ROI_middle().width(), PP->ROI_middle().height());
            Mat Tmp = imgI(rect);
            Mat src = Mat::zeros(imgI.rows, imgI.cols, imgIn[0].type());
            Mat ROI = src(rect);
            Tmp.copyTo(ROI);
            Roi = src.clone();
        }

        else
        {
            //            Rect rect(100, 100, 1700, 900);
            //            250,100,1400,1000---------------------new
            Rect rect(PP->ROI_last().x(), PP->ROI_last().y(), PP->ROI_last().width(), PP->ROI_last().height());
            Mat Tmp = imgI(rect);
            Mat src = Mat::zeros(imgI.rows, imgI.cols, imgIn[0].type());
            Mat ROI = src(rect);
            Tmp.copyTo(ROI);
            Roi = src.clone();

        }
        imgRoi.push_back(Roi);

    }
    return imgRoi;
}




















