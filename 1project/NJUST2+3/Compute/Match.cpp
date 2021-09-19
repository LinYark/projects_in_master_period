#include"Match.h"



string num2str2(int i)
{
    stringstream ss;
    ss << i;
    return ss.str();
}

PointCloud_match::PointCloud_match()
{
    path_source = PP->compute_path().toStdString();
    Path_gongjianmodel = path_source + "/Json_all/";
}



string  PointCloud_match::strToPath(string tmp)
{
    string gongjian_chara_name1 = tmp;
    string gongjian_chara_name2 = reverseString(tmp);
    string Path_zong1 = Path_gongjianmodel + gongjian_chara_name1 + ".json";
    string Path_zong2 = Path_gongjianmodel + gongjian_chara_name2 + ".json";
    //cout<<Path_zong1<<endl;
    //cout<<Path_zong2<<endl;
    int flg1 = 1, flg2 = 1;
    ifstream fin1;
    fin1.open(Path_zong1);
    if (!fin1)
    {
        flg1 = 0;
    }
    fin1.close();
    ifstream fin2;
    fin2.open(Path_zong2);
    if (!fin2)
    {
        flg2 = 0;
    }
    fin2.close();

    string Path0="none";

    if (flg1 == 1 && flg2 == 0)
    {
        Path0 = Path_zong1;
    }
    if (flg1 == 0 && flg2 == 1)
    {
        Path0 = Path_zong2;
    }
    return Path0;
}

string PointCloud_match::reverseString(string s)
{
    int charnum = s.length();
    string wordreverse;
    wordreverse = s;
    for (int i = 0; i < charnum; i++)
    {
        wordreverse[i] = s[charnum - i - 1];
    }
    return wordreverse;
}
float PointCloud_match::func_2(vector<Point3d> twoPoint, PointCloud::Ptr sourcelk, int pointNums,int  jumpPoint)
{
    Mat temp_1(1, 2, CV_32FC1);
    Mat tatgetMat, sourceMat, tatgettatgetMat, sourcesourceMat, A, B, AA, BB, DD;
    int n=0, m=0;
    double pieceX = (twoPoint[1].x - twoPoint[0].x)/pointNums;
    double pieceY =( twoPoint[1].y - twoPoint[0].y)/pointNums;

    for (int i = 0; i < pointNums; i++)
    {
        temp_1.at<float>(0, 0) = (int)twoPoint[0].x + i*pieceX;			temp_1.at<float>(0, 1) = (int)twoPoint[0].y + i*pieceY;
        tatgetMat.push_back(temp_1);
        n++;
    }
    A = tatgetMat;
    for (int i = 0; i < sourcelk->points.size(); i = i + jumpPoint)
    {
        temp_1.at<float>(0, 0) = (int)sourcelk->points[i].x;			temp_1.at<float>(0, 1) = (int)sourcelk->points[i].y;
        sourceMat.push_back(temp_1);
        m++;
    }
    B = sourceMat;
    tatgettatgetMat = tatgetMat.mul(tatgetMat);
    sourcesourceMat = sourceMat.mul(sourceMat);
    reduce(tatgettatgetMat, tatgettatgetMat, 1, cv::REDUCE_SUM);
    reduce(sourcesourceMat, sourcesourceMat, 1, cv::REDUCE_SUM);
    tatgettatgetMat = tatgettatgetMat.t();	//1xn
    sourcesourceMat = sourcesourceMat.t();	//1xm
    for (int i = 0; i < m; i++)
    {
        AA.push_back(tatgettatgetMat);	//mxn
    }
    for (int j = 0; j < n; j++)			//nxm
    {
        BB.push_back(sourcesourceMat);
    }
    AA = AA.t();
    DD = AA + BB - 2 * tatgetMat*(sourceMat.t());

    reduce(DD, DD, 1, cv::REDUCE_MIN);
    reduce(DD, DD, 0, cv::REDUCE_SUM);
    int returnnum = DD.at<float>(0, 0);
    return returnnum;
    /*cout << DD << endl;*/
}
vector<Point3d> PointCloud_match::Shi_Duan(vector<Point3d> fourPoint, PointCloud::Ptr sourcelk,
    int pointNums,//??????????
    double distance,
    int jumpPoint
    )
{
    vector<Point3d> twoPoint;
    twoPoint.resize(2);
    double disTemp_1 = pow((fourPoint[0].x - fourPoint[2].x), 2) + pow((fourPoint[0].y - fourPoint[2].y), 2);
    double disTemp_2 = pow((fourPoint[0].x - fourPoint[3].x), 2) + pow((fourPoint[0].y - fourPoint[3].y), 2);
    if (disTemp_1 < disTemp_2)
    {
        twoPoint[0].x = (fourPoint[0].x + fourPoint[2].x) / 2;
        twoPoint[0].y = (fourPoint[0].y + fourPoint[2].y) / 2;
        twoPoint[1].x = (fourPoint[1].x + fourPoint[3].x) / 2;
        twoPoint[1].y = (fourPoint[1].y + fourPoint[3].y) / 2;
    }
    else
    {
        twoPoint[0].x = (fourPoint[0].x + fourPoint[3].x) / 2;
        twoPoint[0].y = (fourPoint[0].y + fourPoint[3].y) / 2;
        twoPoint[1].x = (fourPoint[1].x + fourPoint[2].x) / 2;
        twoPoint[1].y = (fourPoint[1].y + fourPoint[2].y) / 2;
    }

    double k = (twoPoint[0].y - twoPoint[1].y) / (twoPoint[0].x - twoPoint[1].x);
    double kInv = -1 / k;

    double deltaX_P = pow((distance*distance / (kInv*kInv + 1)), 0.5);
    double deltaY_P = kInv*deltaX_P;
    double deltaX_N = -pow((distance*distance / (kInv*kInv + 1)), 0.5);
    double deltaY_N = kInv*deltaX_N;

    vector<vector<Point3d>> pushedPoint;
    pushedPoint.resize(2);
    Point3d tempP;
    tempP.x = twoPoint[0].x + deltaX_P;
    tempP.y = twoPoint[0].y + deltaY_P;
    pushedPoint[0].push_back(tempP);
    tempP.x = twoPoint[1].x + deltaX_P;
    tempP.y = twoPoint[1].y + deltaY_P;
    pushedPoint[0].push_back(tempP);

    tempP.x = twoPoint[0].x + deltaX_N;
    tempP.y = twoPoint[0].y + deltaY_N;
    pushedPoint[1].push_back(tempP);
    tempP.x = twoPoint[1].x + deltaX_N;
    tempP.y = twoPoint[1].y + deltaY_N;
    pushedPoint[1].push_back(tempP);

    Point3d centerPoint;
    centerPoint.x = (twoPoint[0].x + twoPoint[1].x) / 2;	centerPoint.y = (twoPoint[0].y + twoPoint[1].y) / 2;

    deltaX_P = pow((distance*distance / (k*k + 1)), 0.5);
    deltaY_P = k*deltaX_P;
    deltaX_N = -pow((distance*distance / (k*k + 1)), 0.5);
    deltaY_N = k*deltaX_N;

    for (int i = 0; i < pushedPoint.size(); i++)
    {
        for (int j = 0; j < pushedPoint[i].size(); j++)
        {
            disTemp_1 = pow((pushedPoint[i][j].x + deltaX_P - centerPoint.x), 2) + pow((pushedPoint[i][j].y + deltaY_P - centerPoint.y), 2);
            disTemp_2 = pow((pushedPoint[i][j].x + deltaX_N - centerPoint.x), 2) + pow((pushedPoint[i][j].y + deltaY_N - centerPoint.y), 2);
            if (disTemp_1 < disTemp_2)
            {
                pushedPoint[i][j].x = pushedPoint[i][j].x + deltaX_P;
                pushedPoint[i][j].y = pushedPoint[i][j].y + deltaY_P;
            }
            else
            {
                pushedPoint[i][j].x = pushedPoint[i][j].x + deltaX_N;
                pushedPoint[i][j].y = pushedPoint[i][j].y + deltaY_N;
            }
        }
    }
     vector<int> pushValue;
     //viewPair(sourcelk,sourcelk);

    for (int i = 0; i < pushedPoint.size(); i++)
    {
        int value = func_2(pushedPoint[i], sourcelk, pointNums, jumpPoint);
        pushValue.push_back(value);
    }
    vector<Point3d> return3line = fourPoint;
    if (pushValue[0] > pushValue[1])
    {
        return3line.push_back(pushedPoint[0][0]);
        return3line.push_back(pushedPoint[0][1]);
    }
    else {
        return3line.push_back(pushedPoint[1][0]);
        return3line.push_back(pushedPoint[1][1]);
    }

    return return3line;
}



vector<Point3d> PointCloud_match::Match(int workpiece_index, PointCloud::Ptr target_in, char* Path)
{
//    viewPair1(target_in);
    qDebug()<< "vector<Point3d> PointCloud_match::Match   ";
    LunKuoPiPei HYY;
    PiPei   ZhuanHuan;
    PointCloud::Ptr target00(new PointCloud);
    target00=target_in;
    vector<vector<Point3d>>	jsonpcd;
    vector<Point3d> source_lunkuo;
    vector<Point3d> source3d;
    vector<Point3d> source_stiffener;
    jsonpcd = json_all(Path);

    source3d = jsonpcd[0];
    source_lunkuo = jsonpcd[1];
    source_stiffener=jsonpcd[2];
    PointCloud::Ptr sourcejs(new PointCloud);
    PointCloud::Ptr sourcelk(new PointCloud);
    PointCloud::Ptr target(new PointCloud);
    PointCloud::Ptr source(new PointCloud);
    sourcelk->width = 1;
    sourcelk->height = source_lunkuo.size();
    sourcelk->points.resize(sourcelk->width * sourcelk->height);
    sourcejs->width = 1;
    sourcejs->height = source3d.size();
    sourcejs->points.resize(sourcejs->width * sourcejs->height);
    PointCloud::Ptr source_stiff(new PointCloud);
    source_stiff->width = 1;
    source_stiff->height = source_stiffener.size();
    source_stiff->points.resize(source_stiff->width * source_stiff->height);
    //////
    int source3dsize = source3d.size();
    double  XYZ[3], XYZ_min = 0;
    int XYZ_flag = 666;
    XYZ[0] = fabs(source3d[0].x - source3d[source3dsize / 4].x) +
        fabs(source3d[0].x - source3d[source3dsize * 2 / 4].x) +
        fabs(source3d[0].x - source3d[source3dsize * 3 / 4].x) +
        fabs(source3d[0].x - source3d[source3dsize - 1].x);
    XYZ[1] = fabs(source3d[0].y - source3d[source3dsize / 4].y) +
        fabs(source3d[0].y - source3d[source3dsize * 2 / 4].y) +
        fabs(source3d[0].y - source3d[source3dsize * 3 / 4].y) +
        fabs(source3d[0].y - source3d[source3dsize - 1].y);
    XYZ[2] = fabs(source3d[0].z - source3d[source3dsize / 4].z) +
        fabs(source3d[0].z - source3d[source3dsize * 2 / 4].z) +
        fabs(source3d[0].z - source3d[source3dsize * 3 / 4].z) +
        fabs(source3d[0].z - source3d[source3dsize - 1].z);
    for (int i = 0; i <= 2; i++) {
        if (i == 0) {
            XYZ_min = XYZ[0];
            XYZ_flag = 0;
        }
        else {
            if (XYZ_min > XYZ[i]) {
                XYZ_min = XYZ[i];
                XYZ_flag = i;
            }
        }
    }

    switch (XYZ_flag)
    {
    case 0:
        for (int ii = 0; ii < source3d.size(); ii++)
        {
            sourcejs->points[ii].x = source3d[ii].z;
            sourcejs->points[ii].y = source3d[ii].y;
            sourcejs->points[ii].z = source3d[ii].x;
            //sourcejs->points[ii].z =0;
        }
        for (int ii = 0; ii < source_stiffener.size(); ii++)
        {
            source_stiff->points[ii].x = source_stiffener[ii].z;
            source_stiff->points[ii].y = source_stiffener[ii].y;
            //                            //sourcejs->points[ii].z = source3d[ii].x;
            source_stiff->points[ii].z = source_stiffener[ii].x;
            //sourcejs->points[ii].z =0;
        }

        for (int ii = 0; ii < source_lunkuo.size(); ii++)
        {
            sourcelk->points[ii].x = source_lunkuo[ii].z;
            sourcelk->points[ii].y = source_lunkuo[ii].y;
            sourcelk->points[ii].z = source_lunkuo[ii].x;
        }
        break;
    case 1:
        for (int ii = 0; ii < source3d.size(); ii++)
        {
            sourcejs->points[ii].x = source3d[ii].x;
            sourcejs->points[ii].y = source3d[ii].z;
            sourcejs->points[ii].z = source3d[ii].y;
            //  sourcejs->points[ii].z =0;
        }
        for (int ii = 0; ii < source_stiffener.size(); ii++)
        {
            source_stiff->points[ii].x = source_stiffener[ii].x;
            source_stiff->points[ii].y = source_stiffener[ii].z;
            //                            //sourcejs->points[ii].z = source3d[ii].x;
            source_stiff->points[ii].z = source_stiffener[ii].y;
            //sourcejs->points[ii].z =0;
        }
        for (int ii = 0; ii < source_lunkuo.size(); ii++)
        {
            sourcelk->points[ii].x = source_lunkuo[ii].x;
            sourcelk->points[ii].y = source_lunkuo[ii].z;
            sourcelk->points[ii].z = source_lunkuo[ii].y;
        }
        break;
    case 2:
        for (int ii = 0; ii < source3d.size(); ii++)
        {
            sourcejs->points[ii].x = source3d[ii].x;
            sourcejs->points[ii].y = source3d[ii].y;
            sourcejs->points[ii].z = source3d[ii].z;
            // sourcejs->points[ii].z =0;
        }
        for (int ii = 0; ii < source_stiffener.size(); ii++)
        {
            source_stiff->points[ii].x = source_stiffener[ii].x;
            source_stiff->points[ii].y = source_stiffener[ii].y;
            //                            //sourcejs->points[ii].z = source3d[ii].x;
            source_stiff->points[ii].z = source_stiffener[ii].z;
            //sourcejs->points[ii].z =0;
        }
        for (int ii = 0; ii < source_lunkuo.size(); ii++)
        {
            sourcelk->points[ii].x = source_lunkuo[ii].x;
            sourcelk->points[ii].y = source_lunkuo[ii].y;
            sourcelk->points[ii].z = source_lunkuo[ii].z;
        }
        break;
    }
    //cout<<"------------345-----------"<<source3d.size()<<endl;
    direct_turn_matrix turn_matrix;
    turn_matrix = direct_turn(sourcelk);
    //cout<<"------------348-----------"<<source3d.size()<<endl;
    pcl::transformPointCloud(*sourcejs, *sourcejs, turn_matrix.transformation_matrix);
    pcl::transformPointCloud(*sourcejs, *sourcejs, turn_matrix.transformation_matrix2);
    pcl::transformPointCloud(*source_stiff, *source_stiff, turn_matrix.transformation_matrix);
    pcl::transformPointCloud(*source_stiff, *source_stiff, turn_matrix.transformation_matrix2);
   // cout<<"------------352-----------"<<source3d.size()<<endl;
    //viewPair(sourcejs，sourcelk);
    //viewPair(sourcelk,sourcejs);
    //pcl::io::savePCDFileASCII("/home/jiangnan/QT-PROGRAMS/PCD/sourcejs.pcd",*sourcejs);
    double z=0;
    for(int i=0;i<source_stiff->points.size();i++)
    {
        z+=source_stiff->points[i].z;
    }
    z=z/source_stiff->points.size();
    cout<<z<<endl;
    Eigen::Matrix4f transformation_matrix_temp11;
    int ff=0;
    //cout<<"------------364-----------"<<source3d.size()<<endl;
    if(z<sourcejs->points[0].z)
    {
        Eigen::AngleAxisf init_rotation(0.0, Eigen::Vector3f::UnitZ());    //??????????????????????????(?????
        Eigen::AngleAxisf init_rotation1(3.14, Eigen::Vector3f::UnitY());
        Eigen::AngleAxisf init_rotation2(0.0, Eigen::Vector3f::UnitX());
        Eigen::Translation3f init_translation(0, 0, 0);
        transformation_matrix_temp11 = (init_translation * init_rotation*init_rotation1*init_rotation2).matrix();

        pcl::transformPointCloud(*sourcejs, *sourcejs, transformation_matrix_temp11);
        pcl::transformPointCloud(*sourcelk, *sourcelk, transformation_matrix_temp11);
        pcl::transformPointCloud(*source_stiff, *source_stiff, transformation_matrix_temp11);
        ff=1;

    }
//cout<<"------------379-----------"<<source3d.size()<<endl;
    double downX = sourcejs->points[0].x;
    double downY = sourcejs->points[0].y;
    for (int i = 0; i < sourcejs->points.size(); i++)
    {
        sourcejs->points[i].x = sourcejs->points[i].x - downX + 1.0;
        sourcejs->points[i].y = sourcejs->points[i].y - downY + 1.0;
        sourcejs->points[i].z = 0;
    }
    vector<vector<Point3d>> doublesource3d;
    doublesource3d = json_hf(Path);
    int HF_all_num = doublesource3d.size();
    //int hfnumCheck = HF_all_num;
    double pipeidu;
    if (HF_all_num <= 2) {
        pipeidu = 1200;
    }
    else {
        pipeidu = 2500;
    }

    target = HYY.LunKuoPiPeiHY_11(target00);
    source = HYY.LunKuoPiPeiHY_11(sourcejs);
    int FLAG=1;
   // viewPair(target,source);
    //viewPair1(target);
    //pcl::io::savePCDFileASCII("target.pcd",)
    ZhuanHuan = HYY.LunKuoPiPeiHY_2(target, source);
    std::cout<<"flag: "<<ZhuanHuan.flag<<endl;
    FLAG=FLAG*ZhuanHuan.flag;
    if(FLAG==-1)
        cout<<"gongjian biaozhunjian fan"<<endl;
    //HFALL_.resize(HF_all_num);
    Eigen::Matrix4f transformation_matrix;
    Eigen::Matrix4f transformation_matrix_bef;
    Eigen::Matrix4f transformation_matrix_temp;
    Eigen::Matrix4f transformation_matrixmin;
    PointCloud::Ptr cloud_standerd_temp(new PointCloud);
    PointCloud::Ptr cloud_standerd_order(new PointCloud);


    pcl::copyPointCloud(*sourcejs, *cloud_standerd_temp);
    transformation_matrix_bef = ZhuanHuan.transformation_matrix0_ni00;
    transformation_matrix = transformation_matrix_bef;
    pcl::transformPointCloud(*cloud_standerd_temp, *cloud_standerd_temp, transformation_matrix);
    pcl::copyPointCloud(*cloud_standerd_temp, *cloud_standerd_order);
    save_pcl_to_img(target00, cloud_standerd_temp,workpiece_index,ZhuanHuan.PiPeiDu00);
    int pipeidumin = ZhuanHuan.PiPeiDu00;

    //viewPair(target,cloud_standerd_temp);

    if (FLAG==-1)
    {
        pcl::copyPointCloud(*sourcejs, *cloud_standerd_temp);

        transformation_matrix_temp = getMinRect(cloud_standerd_temp);
        transformation_matrix_temp = transformation_matrix_temp * transformation_matrix_bef;

        pcl::transformPointCloud(*cloud_standerd_temp, *cloud_standerd_temp, transformation_matrix_temp);


        target = HYY.LunKuoPiPeiHY_11(target00);
        source = HYY.LunKuoPiPeiHY_11(cloud_standerd_temp);
       // viewPair(target,source);
        ZhuanHuan = HYY.LunKuoPiPeiHY_2(target, source);
        FLAG=FLAG*ZhuanHuan.flag;
        if(FLAG==-1)
            cout<<"gongjian biaozhunjian fan getMinRect"<<endl;
        std::cout<<"flag: "<<ZhuanHuan.flag<<endl;
        transformation_matrix_temp = ZhuanHuan.transformation_matrix0_ni00 *  transformation_matrix_temp;
        transformation_matrix_bef = transformation_matrix_temp;
        transformation_matrix = transformation_matrix_temp;
        cout << "...";
        if (ZhuanHuan.PiPeiDu00 < pipeidumin && FLAG==1){
            pipeidumin = ZhuanHuan.PiPeiDu00;
            transformation_matrixmin = transformation_matrix;
        }
        pcl::copyPointCloud(*sourcejs, *cloud_standerd_temp);
        pcl::transformPointCloud(*cloud_standerd_temp, *cloud_standerd_temp, transformation_matrix);
        save_pcl_to_img(target00, cloud_standerd_temp,workpiece_index,ZhuanHuan.PiPeiDu00);
        //viewPair(target,cloud_standerd_temp);
    }

    if (ZhuanHuan.PiPeiDu00 > pipeidu||FLAG==-1)     //????????????1000???????????X???????????????
    {
        pcl::copyPointCloud(*sourcejs, *cloud_standerd_temp);
        FLAG=1;
        Eigen::AngleAxisf init_rotation(3.14, Eigen::Vector3f::UnitZ());    //??????????????????????????(?????
        Eigen::AngleAxisf init_rotation1(0.0, Eigen::Vector3f::UnitY());
        Eigen::AngleAxisf init_rotation2(0.0, Eigen::Vector3f::UnitX());
        Eigen::Translation3f init_translation(0, 0, 0);
        transformation_matrix_temp = (init_translation * init_rotation*init_rotation1*init_rotation2).matrix();

        pcl::transformPointCloud(*cloud_standerd_temp, *cloud_standerd_temp, transformation_matrix_temp);


        target = HYY.LunKuoPiPeiHY_11(target00);     //??source00??????????????????????
        source = HYY.LunKuoPiPeiHY_11(cloud_standerd_temp);      //??target00??????????????????????
        //viewPair(target,source);
        ZhuanHuan = HYY.LunKuoPiPeiHY_2(target, source);   //??source??target???????????result????????????????
        std::cout<<"flag: "<<ZhuanHuan.flag<<endl;
        FLAG=FLAG*ZhuanHuan.flag;
        if(FLAG==-1)
            cout<<"gongjian biaozhunjian fan"<<endl;
        cout << "...";

        transformation_matrix_temp = ZhuanHuan.transformation_matrix0_ni00 *transformation_matrix_temp;
        transformation_matrix_bef = transformation_matrix_temp;
        transformation_matrix = transformation_matrix_bef;
        if (ZhuanHuan.PiPeiDu00 < pipeidumin && FLAG==1){
            pipeidumin = ZhuanHuan.PiPeiDu00;
            transformation_matrixmin = transformation_matrix;
        }

        pcl::copyPointCloud(*sourcejs, *cloud_standerd_temp);
        pcl::transformPointCloud(*cloud_standerd_temp, *cloud_standerd_temp, transformation_matrix);
        pcl::copyPointCloud(*cloud_standerd_temp, *cloud_standerd_order);
        save_pcl_to_img(target00, cloud_standerd_temp,workpiece_index,ZhuanHuan.PiPeiDu00);
               //viewPair(target,cloud_standerd_temp);
    }
    if (FLAG==-1)     //????????????1000???????????X???????????????
    {
        pcl::copyPointCloud(*sourcejs, *cloud_standerd_temp);

        transformation_matrix_temp = getMinRect(cloud_standerd_temp);
        //transformation_matrix = (init_translation * init_rotation*init_rotation1*init_rotation2).matrix();
        transformation_matrix_temp = transformation_matrix_temp * transformation_matrix_bef;

        pcl::transformPointCloud(*cloud_standerd_temp, *cloud_standerd_temp, transformation_matrix_temp);


        target = HYY.LunKuoPiPeiHY_11(target00);     //??source00??????????????????????
        source = HYY.LunKuoPiPeiHY_11(cloud_standerd_temp);      //??target00??????????????????????
        //viewPair(target,source);
        ZhuanHuan = HYY.LunKuoPiPeiHY_2(target, source);   //??source??target???????????result????????????????
        FLAG=FLAG*ZhuanHuan.flag;
        if(FLAG==-1)
            cout<<"gongjian biaozhunjian fan getMinRect"<<endl;
        std::cout<<"flag: "<<ZhuanHuan.flag<<endl;
        transformation_matrix_temp = ZhuanHuan.transformation_matrix0_ni00 *  transformation_matrix_temp;
        transformation_matrix_bef = transformation_matrix_temp;
        transformation_matrix = transformation_matrix_temp;
        cout << "...";
        if (ZhuanHuan.PiPeiDu00 < pipeidumin && FLAG==1){
            pipeidumin = ZhuanHuan.PiPeiDu00;
            transformation_matrixmin = transformation_matrix;
        }
        pcl::copyPointCloud(*sourcejs, *cloud_standerd_temp);
        //pcl::transformPointCloud(*cloud_standerd_temp, *cloud_standerd_temp, transformation_matrix);
        pcl::transformPointCloud(*cloud_standerd_temp, *cloud_standerd_temp, transformation_matrix);
        //         viewPair(target,source);
        save_pcl_to_img(target00, cloud_standerd_temp,workpiece_index,ZhuanHuan.PiPeiDu00);
        //pcl::copyPointCloud(*cloud_standerd_temp, *cloud_standerd_order);
        //viewPair(target,cloud_standerd_temp);
    }

    if (ZhuanHuan.PiPeiDu00 > pipeidu||FLAG==-1)     //????????????1000???????????X???????????????
    {
        pcl::copyPointCloud(*sourcejs, *cloud_standerd_temp);
        FLAG=1;
        Eigen::AngleAxisf init_rotation(3.14/1.5, Eigen::Vector3f::UnitZ());    //??????????????????????????(?????
        Eigen::AngleAxisf init_rotation1(0.0, Eigen::Vector3f::UnitY());
        Eigen::AngleAxisf init_rotation2(0.0, Eigen::Vector3f::UnitX());
        Eigen::Translation3f init_translation(0, 0, 0);
        transformation_matrix_temp = (init_translation * init_rotation*init_rotation1*init_rotation2).matrix();

        pcl::transformPointCloud(*cloud_standerd_temp, *cloud_standerd_temp, transformation_matrix_temp);


        target = HYY.LunKuoPiPeiHY_11(target00);     //??source00??????????????????????
        source = HYY.LunKuoPiPeiHY_11(cloud_standerd_temp);      //??target00??????????????????????
        ZhuanHuan = HYY.LunKuoPiPeiHY_2(target, source);   //??source??target???????????result????????????????
        std::cout<<"flag: "<<ZhuanHuan.flag<<endl;
        FLAG=FLAG*ZhuanHuan.flag;
        if(FLAG==-1)
            cout<<"gongjian biaozhunjian fan"<<endl;
        cout << "...";


        transformation_matrix_temp = ZhuanHuan.transformation_matrix0_ni00 *transformation_matrix_temp;
        transformation_matrix_bef = transformation_matrix_temp;
        transformation_matrix = transformation_matrix_bef;
        if (ZhuanHuan.PiPeiDu00 < pipeidumin && FLAG==1){
            pipeidumin = ZhuanHuan.PiPeiDu00;
            transformation_matrixmin = transformation_matrix;
        }

        pcl::copyPointCloud(*sourcejs, *cloud_standerd_temp);
        //pcl::transformPointCloud(*cloud_standerd_temp, *cloud_standerd_temp, transformation_matrix);
        pcl::transformPointCloud(*cloud_standerd_temp, *cloud_standerd_temp, transformation_matrix);
        pcl::copyPointCloud(*cloud_standerd_temp, *cloud_standerd_order);
        save_pcl_to_img(target00, cloud_standerd_temp,workpiece_index,ZhuanHuan.PiPeiDu00);
        //       viewPair(target,cloud_standerd_temp);
    }
    if (FLAG==-1)     //????????????1000???????????X???????????????
    {
        pcl::copyPointCloud(*sourcejs, *cloud_standerd_temp);

        transformation_matrix_temp = getMinRect(cloud_standerd_temp);
        //transformation_matrix = (init_translation * init_rotation*init_rotation1*init_rotation2).matrix();
        transformation_matrix_temp = transformation_matrix_temp * transformation_matrix_bef;

        pcl::transformPointCloud(*cloud_standerd_temp, *cloud_standerd_temp, transformation_matrix_temp);


        target = HYY.LunKuoPiPeiHY_11(target00);     //??source00??????????????????????
        source = HYY.LunKuoPiPeiHY_11(cloud_standerd_temp);      //??target00??????????????????????
        ZhuanHuan = HYY.LunKuoPiPeiHY_2(target, source);   //??source??target???????????result????????????????
        std::cout<<"flag: "<<ZhuanHuan.flag<<endl;
        FLAG=FLAG*ZhuanHuan.flag;
        if(FLAG==-1)
            cout<<"gongjian biaozhunjian fan getMinRect"<<endl;
        transformation_matrix_temp = ZhuanHuan.transformation_matrix0_ni00 *  transformation_matrix_temp;
        transformation_matrix_bef = transformation_matrix_temp;
        transformation_matrix = transformation_matrix_temp;
        cout << "...";
        if (ZhuanHuan.PiPeiDu00 < pipeidumin && FLAG==1){
            pipeidumin = ZhuanHuan.PiPeiDu00;
            transformation_matrixmin = transformation_matrix;
        }
        pcl::copyPointCloud(*sourcejs, *cloud_standerd_temp);
        //pcl::transformPointCloud(*cloud_standerd_temp, *cloud_standerd_temp, transformation_matrix);
        pcl::transformPointCloud(*cloud_standerd_temp, *cloud_standerd_temp, transformation_matrix);
        //         viewPair(target,source);
        save_pcl_to_img(target00, cloud_standerd_temp,workpiece_index,ZhuanHuan.PiPeiDu00);
        //pcl::copyPointCloud(*cloud_standerd_temp, *cloud_standerd_order);
    }

    if (ZhuanHuan.PiPeiDu00 > pipeidu||FLAG==-1)     //????????????1000???????????X???????????????
    {
        pcl::copyPointCloud(*sourcejs, *cloud_standerd_temp);

        FLAG=1;
        Eigen::AngleAxisf init_rotation(3.14/2, Eigen::Vector3f::UnitZ());    //??????????????????????????(?????
        Eigen::AngleAxisf init_rotation1(0.0, Eigen::Vector3f::UnitY());
        Eigen::AngleAxisf init_rotation2(0.0, Eigen::Vector3f::UnitX());

        Eigen::Translation3f init_translation(0, 0, 0);
        transformation_matrix_temp = (init_translation * init_rotation*init_rotation1*init_rotation2).matrix();

        pcl::transformPointCloud(*cloud_standerd_temp, *cloud_standerd_temp, transformation_matrix_temp);


        target = HYY.LunKuoPiPeiHY_11(target00);     //??source00??????????????????????
        source = HYY.LunKuoPiPeiHY_11(cloud_standerd_temp);      //??target00??????????????????????
        ZhuanHuan = HYY.LunKuoPiPeiHY_2(target, source);   //??source??target???????????result????????????????
        std::cout<<"flag: "<<ZhuanHuan.flag<<endl;
        FLAG=FLAG*ZhuanHuan.flag;
        if(FLAG==-1)
            cout<<"gongjian biaozhunjian fan"<<endl;
        cout << "...";


        transformation_matrix_temp = ZhuanHuan.transformation_matrix0_ni00 *transformation_matrix_temp;
        transformation_matrix_bef = transformation_matrix_temp;
        transformation_matrix = transformation_matrix_bef;
        if (ZhuanHuan.PiPeiDu00 < pipeidumin && FLAG==1){
            pipeidumin = ZhuanHuan.PiPeiDu00;
            transformation_matrixmin = transformation_matrix;
        }

        pcl::copyPointCloud(*sourcejs, *cloud_standerd_temp);
        //pcl::transformPointCloud(*cloud_standerd_temp, *cloud_standerd_temp, transformation_matrix);
        pcl::transformPointCloud(*cloud_standerd_temp, *cloud_standerd_temp, transformation_matrix);
        pcl::copyPointCloud(*cloud_standerd_temp, *cloud_standerd_order);
        save_pcl_to_img(target00, cloud_standerd_temp,workpiece_index,ZhuanHuan.PiPeiDu00);
        //       viewPair(target,cloud_standerd_temp);
    }
    if (FLAG==-1)     //????????????1000???????????X???????????????
    {
        pcl::copyPointCloud(*sourcejs, *cloud_standerd_temp);

        transformation_matrix_temp = getMinRect(cloud_standerd_temp);
        //transformation_matrix = (init_translation * init_rotation*init_rotation1*init_rotation2).matrix();
        transformation_matrix_temp = transformation_matrix_temp * transformation_matrix_bef;

        pcl::transformPointCloud(*cloud_standerd_temp, *cloud_standerd_temp, transformation_matrix_temp);


        target = HYY.LunKuoPiPeiHY_11(target00);     //??source00??????????????????????
        source = HYY.LunKuoPiPeiHY_11(cloud_standerd_temp);      //??target00??????????????????????
        ZhuanHuan = HYY.LunKuoPiPeiHY_2(target, source);   //??source??target???????????result????????????????
        std::cout<<"flag: "<<ZhuanHuan.flag<<endl;
        FLAG=FLAG*ZhuanHuan.flag;
        if(FLAG==-1)
            cout<<"gongjian biaozhunjian fan getMinRect"<<endl;
        transformation_matrix_temp = ZhuanHuan.transformation_matrix0_ni00 *  transformation_matrix_temp;
        transformation_matrix_bef = transformation_matrix_temp;
        transformation_matrix = transformation_matrix_temp;
        cout << "...";
        if (ZhuanHuan.PiPeiDu00 < pipeidumin && FLAG==1){
            pipeidumin = ZhuanHuan.PiPeiDu00;
            transformation_matrixmin = transformation_matrix;
        }
        pcl::copyPointCloud(*sourcejs, *cloud_standerd_temp);
        //pcl::transformPointCloud(*cloud_standerd_temp, *cloud_standerd_temp, transformation_matrix);
        pcl::transformPointCloud(*cloud_standerd_temp, *cloud_standerd_temp, transformation_matrix);
        //         viewPair(target,source);
        save_pcl_to_img(target00, cloud_standerd_temp,workpiece_index,ZhuanHuan.PiPeiDu00);
        //pcl::copyPointCloud(*cloud_standerd_temp, *cloud_standerd_order);
    }
    if (ZhuanHuan.PiPeiDu00 > pipeidu||FLAG==-1)     //????????????1000???????????X???????????????
    {
        pcl::copyPointCloud(*sourcejs, *cloud_standerd_temp);

        FLAG=1;
        Eigen::AngleAxisf init_rotation(3.14/3, Eigen::Vector3f::UnitZ());    //??????????????????????????(?????
        Eigen::AngleAxisf init_rotation1(0.0, Eigen::Vector3f::UnitY());
        Eigen::AngleAxisf init_rotation2(0.0, Eigen::Vector3f::UnitX());
        Eigen::Translation3f init_translation(0, 0, 0);
        transformation_matrix_temp = (init_translation * init_rotation*init_rotation1*init_rotation2).matrix();
        pcl::transformPointCloud(*cloud_standerd_temp, *cloud_standerd_temp, transformation_matrix_temp);

        target = HYY.LunKuoPiPeiHY_11(target00);     //??source00??????????????????????
        source = HYY.LunKuoPiPeiHY_11(cloud_standerd_temp);      //??target00??????????????????????
        ZhuanHuan = HYY.LunKuoPiPeiHY_2(target, source);   //??source??target???????????result????????????????
        std::cout<<"flag: "<<ZhuanHuan.flag<<endl;
        FLAG=FLAG*ZhuanHuan.flag;
        if(FLAG==-1)
            cout<<"gongjian biaozhunjian fan"<<endl;
        cout << "...";

        transformation_matrix_temp = ZhuanHuan.transformation_matrix0_ni00 *transformation_matrix_temp;
        transformation_matrix_bef = transformation_matrix_temp;
        transformation_matrix = transformation_matrix_bef;
        if (ZhuanHuan.PiPeiDu00 < pipeidumin && FLAG==1){
            pipeidumin = ZhuanHuan.PiPeiDu00;
            transformation_matrixmin = transformation_matrix;
        }

        pcl::copyPointCloud(*sourcejs, *cloud_standerd_temp);
        //pcl::transformPointCloud(*cloud_standerd_temp, *cloud_standerd_temp, transformation_matrix);
        pcl::transformPointCloud(*cloud_standerd_temp, *cloud_standerd_temp, transformation_matrix);
        pcl::copyPointCloud(*cloud_standerd_temp, *cloud_standerd_order);
        save_pcl_to_img(target00, cloud_standerd_temp,workpiece_index,ZhuanHuan.PiPeiDu00);
        //       viewPair(target,cloud_standerd_temp);
    }
    if (FLAG==-1)     //????????????1000???????????X???????????????
    {
        pcl::copyPointCloud(*sourcejs, *cloud_standerd_temp);
        transformation_matrix_temp = getMinRect(cloud_standerd_temp);
        transformation_matrix_temp = transformation_matrix_temp * transformation_matrix_bef;
        pcl::transformPointCloud(*cloud_standerd_temp, *cloud_standerd_temp, transformation_matrix_temp);

        target = HYY.LunKuoPiPeiHY_11(target00);     //??source00??????????????????????
        source = HYY.LunKuoPiPeiHY_11(cloud_standerd_temp);      //??target00??????????????????????
        ZhuanHuan = HYY.LunKuoPiPeiHY_2(target, source);   //??source??target???????????result????????????????
        std::cout<<"flag: "<<ZhuanHuan.flag<<endl;
        FLAG=FLAG*ZhuanHuan.flag;
        if(FLAG==-1)
            cout<<"gongjian biaozhunjian fan getMinRect"<<endl;
        transformation_matrix_temp = ZhuanHuan.transformation_matrix0_ni00 *  transformation_matrix_temp;
        transformation_matrix_bef = transformation_matrix_temp;
        transformation_matrix = transformation_matrix_temp;
        cout << "...";
        if (ZhuanHuan.PiPeiDu00 < pipeidumin && FLAG==1){
            pipeidumin = ZhuanHuan.PiPeiDu00;
            transformation_matrixmin = transformation_matrix;
        }
        pcl::copyPointCloud(*sourcejs, *cloud_standerd_temp);
        pcl::transformPointCloud(*cloud_standerd_temp, *cloud_standerd_temp, transformation_matrix);
        save_pcl_to_img(target00, cloud_standerd_temp,workpiece_index,ZhuanHuan.PiPeiDu00);
    }
    if (ZhuanHuan.PiPeiDu00 > pipeidu||FLAG==-1)     //????????????1000???????????X???????????????
    {
        pcl::copyPointCloud(*sourcejs, *cloud_standerd_temp);

        FLAG=1;
        Eigen::AngleAxisf init_rotation(3.14/4, Eigen::Vector3f::UnitZ());    //??????????????????????????(?????
        Eigen::AngleAxisf init_rotation1(0.0, Eigen::Vector3f::UnitY());
        Eigen::AngleAxisf init_rotation2(0.0, Eigen::Vector3f::UnitX());
        Eigen::Translation3f init_translation(0, 0, 0);
        transformation_matrix_temp = (init_translation * init_rotation*init_rotation1*init_rotation2).matrix();
        pcl::transformPointCloud(*cloud_standerd_temp, *cloud_standerd_temp, transformation_matrix_temp);

        target = HYY.LunKuoPiPeiHY_11(target00);     //??source00??????????????????????
        source = HYY.LunKuoPiPeiHY_11(cloud_standerd_temp);      //??target00??????????????????????
        ZhuanHuan = HYY.LunKuoPiPeiHY_2(target, source);   //??source??target???????????result????????????????
        std::cout<<"flag: "<<ZhuanHuan.flag<<endl;
        FLAG=FLAG*ZhuanHuan.flag;
        if(FLAG==-1)
            cout<<"gongjian biaozhunjian fan"<<endl;
        cout << "...";

        transformation_matrix_temp = ZhuanHuan.transformation_matrix0_ni00 *transformation_matrix_temp;
        transformation_matrix_bef = transformation_matrix_temp;
        transformation_matrix = transformation_matrix_bef;
        if (ZhuanHuan.PiPeiDu00 < pipeidumin && FLAG==1){
            pipeidumin = ZhuanHuan.PiPeiDu00;
            transformation_matrixmin = transformation_matrix;
        }

        pcl::copyPointCloud(*sourcejs, *cloud_standerd_temp);
        //pcl::transformPointCloud(*cloud_standerd_temp, *cloud_standerd_temp, transformation_matrix);
        pcl::transformPointCloud(*cloud_standerd_temp, *cloud_standerd_temp, transformation_matrix);
        pcl::copyPointCloud(*cloud_standerd_temp, *cloud_standerd_order);
        save_pcl_to_img(target00, cloud_standerd_temp,workpiece_index,ZhuanHuan.PiPeiDu00);
        //       viewPair(target,cloud_standerd_temp);
    }
    if (FLAG==-1)     //????????????1000???????????X???????????????
    {
        pcl::copyPointCloud(*sourcejs, *cloud_standerd_temp);
        transformation_matrix_temp = getMinRect(cloud_standerd_temp);
        transformation_matrix_temp = transformation_matrix_temp * transformation_matrix_bef;
        pcl::transformPointCloud(*cloud_standerd_temp, *cloud_standerd_temp, transformation_matrix_temp);

        target = HYY.LunKuoPiPeiHY_11(target00);     //??source00??????????????????????
        source = HYY.LunKuoPiPeiHY_11(cloud_standerd_temp);      //??target00??????????????????????
        ZhuanHuan = HYY.LunKuoPiPeiHY_2(target, source);   //??source??target???????????result????????????????
        std::cout<<"flag: "<<ZhuanHuan.flag<<endl;
        FLAG=FLAG*ZhuanHuan.flag;
        if(FLAG==-1)
            cout<<"gongjian biaozhunjian fan getMinRect"<<endl;
        transformation_matrix_temp = ZhuanHuan.transformation_matrix0_ni00 *  transformation_matrix_temp;
        transformation_matrix_bef = transformation_matrix_temp;
        transformation_matrix = transformation_matrix_temp;
        cout << "...";
        if (ZhuanHuan.PiPeiDu00 < pipeidumin && FLAG==1){
            pipeidumin = ZhuanHuan.PiPeiDu00;
            transformation_matrixmin = transformation_matrix;
        }
        pcl::copyPointCloud(*sourcejs, *cloud_standerd_temp);
        pcl::transformPointCloud(*cloud_standerd_temp, *cloud_standerd_temp, transformation_matrix);
        save_pcl_to_img(target00, cloud_standerd_temp,workpiece_index,ZhuanHuan.PiPeiDu00);
    }



    vector<Point3d> returnHF;
    //pcl::transformPointCloud(*sourcelk,*sourcelk, turn_matrix.transformation_matrix);
    //pcl::transformPointCloud(*sourcelk, *sourcelk, turn_matrix.transformation_matrix2);

    for(int i =0 ;i<sourcelk->points.size();i++)
    {
        sourcelk->points[i].x = sourcelk->points[i].x - downX + 1.0;
        sourcelk->points[i].y = sourcelk->points[i].y - downY + 1.0;
        sourcelk->points[i].z = 0;
    }




    //viewPair(target00,sourcelk);
    //viewPair(target00,cloud_standerd_temp);

    PointCloud::Ptr HFnum1(new PointCloud);

    if (ZhuanHuan.PiPeiDu00 <= pipeidu)
    {
        pcl::transformPointCloud(*sourcelk, *sourcelk, transformation_matrix);
        save_pcl_to_img(target00, cloud_standerd_temp,workpiece_index,ZhuanHuan.PiPeiDu00);
        int a =ZhuanHuan.PiPeiDu00;
        Eigen::Vector4f centroid;					// 质心
            pcl::compute3DCentroid(*sourcelk, centroid);
             for(int i =0 ;i<cloud_standerd_temp->points.size();i++)
        {
            cloud_standerd_temp->points[i].x = cloud_standerd_temp->points[i].x - centroid[0];
            cloud_standerd_temp->points[i].y = cloud_standerd_temp->points[i].y - centroid[1];
            cloud_standerd_temp->points[i].z = cloud_standerd_temp->points[i].z ;
        }


            Eigen::AngleAxisf init_rotation(3.14, Eigen::Vector3f::UnitZ());    //??????????????????????????(?????
            Eigen::AngleAxisf init_rotation1(0.0, Eigen::Vector3f::UnitY());
            Eigen::AngleAxisf init_rotation2(0.0, Eigen::Vector3f::UnitX());
            Eigen::Translation3f init_translation(0, 0, 0);
            transformation_matrix_temp = (init_translation * init_rotation*init_rotation1*init_rotation2).matrix();
            pcl::transformPointCloud(*cloud_standerd_temp, *cloud_standerd_temp, transformation_matrix_temp);
            for(int i =0 ;i<cloud_standerd_temp->points.size();i++)
        {
            cloud_standerd_temp->points[i].x = cloud_standerd_temp->points[i].x + centroid[0];
            cloud_standerd_temp->points[i].y = cloud_standerd_temp->points[i].y + centroid[1];
            cloud_standerd_temp->points[i].z = cloud_standerd_temp->points[i].z ;
        }
            target = HYY.LunKuoPiPeiHY_11(target00);     //??source00??????????????????????
            source = HYY.LunKuoPiPeiHY_11(cloud_standerd_temp);      //??target00??????????????????????
            ZhuanHuan = HYY.LunKuoPiPeiHY_2(target, source);
            pcl::transformPointCloud(*cloud_standerd_temp, *cloud_standerd_temp, ZhuanHuan.transformation_matrix0_ni00);
            save_pcl_to_img(target00, cloud_standerd_temp,workpiece_index,ZhuanHuan.PiPeiDu00);
            //pcl::io::savePCDFileASCII("/home/jiangnan/source.pcd",*cloud_standerd_temp);
            //pcl::io::savePCDFileASCII("/home/jiangnan/target.pcd",*target00);
            int b = ZhuanHuan.PiPeiDu00;
             if (ZhuanHuan.PiPeiDu00 <= pipeidu){
                 if (b < a){
                     transformation_matrixmin=ZhuanHuan.transformation_matrix0_ni00;
                     cout <<Path<<":  "<< workpiece_index << "th  Match xuanzhuan succeed" << endl;

                 }
                 else{
                    transformation_matrix = transformation_matrix;
                    cout <<Path<<":  "<< workpiece_index << "th  Match succeed" << endl;
                 }
             }

        int  put_or_not = 0;
        pcl::copyPointCloud(*sourcejs, *cloud_standerd_temp);
        pcl::transformPointCloud(*cloud_standerd_temp, *cloud_standerd_temp, transformation_matrix);

              //viewPair(target,cloud_standerd_temp);

        for (uint n = 0; n < HF_all_num; n++)
        {

            HFnum1->width = 1;
            HFnum1->height = doublesource3d[n].size();
            HFnum1->points.resize(HFnum1->width * HFnum1->height);
            switch (XYZ_flag)
            {
            case 0:
                for (int ii = 0; ii < doublesource3d[n].size(); ii++)
                {
                    HFnum1->points[ii].x = doublesource3d[n][ii].z;
                    HFnum1->points[ii].y = doublesource3d[n][ii].y;
                    HFnum1->points[ii].z = doublesource3d[n][ii].x;

                }
                break;
            case 1:
                for (int ii = 0; ii < doublesource3d[n].size(); ii++)
                {
                    HFnum1->points[ii].x = doublesource3d[n][ii].x;
                    HFnum1->points[ii].y = doublesource3d[n][ii].z;
                    HFnum1->points[ii].z = doublesource3d[n][ii].y;
                }
                break;
            case 2:
                for (int ii = 0; ii < doublesource3d[n].size(); ii++)
                {
                    HFnum1->points[ii].x = doublesource3d[n][ii].x;
                    HFnum1->points[ii].y = doublesource3d[n][ii].y;
                    HFnum1->points[ii].z = doublesource3d[n][ii].z;
                }
                break;
            }

            pcl::transformPointCloud(*HFnum1, *HFnum1, turn_matrix.transformation_matrix);
            pcl::transformPointCloud(*HFnum1, *HFnum1, turn_matrix.transformation_matrix2);
            if(ff==1)
            {
                pcl::transformPointCloud(*HFnum1, *HFnum1, transformation_matrix_temp11);
            }


            for (int i = 0; i < HFnum1->points.size(); i++)
            {
                HFnum1->points[i].x = HFnum1->points[i].x - downX + 1.0;
                HFnum1->points[i].y = HFnum1->points[i].y - downY + 1.0;
                HFnum1->points[i].z = 0;
            }


            if (b<a){
                           pcl::transformPointCloud(*HFnum1, *HFnum1, transformation_matrix);
                           for(int i =0 ;i<HFnum1->points.size();i++)
                               {
                                  HFnum1->points[i].x = HFnum1->points[i].x - centroid[0];
                                  HFnum1->points[i].y = HFnum1->points[i].y - centroid[1];
                                  HFnum1->points[i].z = HFnum1->points[i].z ;
                               }
                           pcl::transformPointCloud(*HFnum1, *HFnum1, transformation_matrix_temp);
                           for(int i =0 ;i<HFnum1->points.size();i++)
                               {
                                  HFnum1->points[i].x = HFnum1->points[i].x + centroid[0];
                                  HFnum1->points[i].y = HFnum1->points[i].y + centroid[1];
                                  HFnum1->points[i].z = HFnum1->points[i].z ;
                               }
                           pcl::transformPointCloud(*HFnum1, *HFnum1, transformation_matrixmin);
                           }
                           else{
                               pcl::transformPointCloud(*HFnum1, *HFnum1, transformation_matrix);
                           }

            //viewPair(target00,cloud_standerd_temp);
            //viewPair(target00,HFnum1);
            //viewPair(sourcelk,HFnum1);
            PointCloud::Ptr target(new PointCloud);


            Point3d temp1;


            temp1.x = HFnum1->points[0].x;
            temp1.y = HFnum1->points[0].y;
            returnHF.push_back(temp1);
            cout << "(" << temp1.x << "," << temp1.y << ")\t";
            temp1.x = HFnum1->points[HFnum1->points.size() - 1].x;
            temp1.y = HFnum1->points[HFnum1->points.size() - 1].y;
            returnHF.push_back(temp1);
            cout << "(" <<temp1.x << "," << temp1.y << ")" << endl;
        }
        vector<vector<Point3d>> HFALL_Three;
        HFALL_Three.resize(returnHF.size()*0.75);
        int ii=0;
        //viewPair(sourcelk,target00);
        for(int i =0;i<returnHF.size();i=i+4)
        {
            Point3d Point0,Point1,Point2,Point3;
            Point0.x=returnHF[i].x;
            Point0.y=returnHF[i].y;
            Point0.z=0;
            Point1.x=returnHF[i+1].x;
            Point1.y=returnHF[i+1].y;
            Point1.z=0;
            Point2.x=returnHF[i+2].x;
            Point2.y=returnHF[i+2].y;
            Point2.z=0;
            Point3.x=returnHF[i+3].x;
            Point3.y=returnHF[i+3].y;
            Point3.z=0;
            vector<Point3d> fourPoint={Point0,Point1,Point2,Point3};

            vector<Point3d> Six_Point=Shi_Duan(fourPoint,sourcelk,20,80, 10 );

            HFALL_Three[ii].resize(2);
            HFALL_Three[ii+1].resize(2);
            HFALL_Three[ii+2].resize(2);
            HFALL_Three[ii][0]=Six_Point[0];
            HFALL_Three[ii][1]=Six_Point[1];
            HFALL_Three[ii+1][0]=Six_Point[2];
            HFALL_Three[ii+1][1]=Six_Point[3];
            HFALL_Three[ii+2][0]=Six_Point[4];
            HFALL_Three[ii+2][1]=Six_Point[5];
            ii=ii+3;
            qDebug()<<"Start Point Detect:"<<'/n';
            for(int j =0;j<Six_Point.size();j++)
            {
                 qDebug()<<Six_Point[j].x<<" "<<Six_Point[j].y<<"      ";
            }

        }
        returnHF.clear();
        //returnHF.resize(2*HFALL_Three.size());
        for(int ii=0;ii<HFALL_Three.size();ii=ii+3)
        {


            /*returnHF[ii].x=HFALL_Three[ii][0];
            returnHF[ii].y=HFALL_Three[ii][1];
            returnHF[ii+1].x=HFALL_Three[ii+1][0];
            returnHF[ii+1].y=HFALL_Three[ii+1][1];
            returnHF[ii+2].x=HFALL_Three[ii+2][0];
            returnHF[ii+2].y=HFALL_Three[ii+2][1];*/

            returnHF.push_back(HFALL_Three[ii][0]);
            returnHF.push_back(HFALL_Three[ii][1]);
            returnHF.push_back(HFALL_Three[ii+1][0]);
            returnHF.push_back(HFALL_Three[ii+1][1]);
            //returnHF.push_back(HFALL_Three[ii+2][0]);
            //returnHF.push_back(HFALL_Three[ii+2][1]);

        }


        qDebug()<< "return returnHF        ";
        qDebug()<< "returnHF.size()        "<<returnHF.size();


        return returnHF;
    }
    if (ZhuanHuan.PiPeiDu00 > pipeidu )
    {
        cout << "Num:" << workpiece_index << "th  Match fail" << endl;
        cout << "minpipeidu:" << pipeidumin << endl;
        pcl::copyPointCloud(*sourcejs, *cloud_standerd_temp);
        pcl::transformPointCloud(*cloud_standerd_temp, *cloud_standerd_temp, transformation_matrixmin);
        pcl::transformPointCloud(*sourcelk, *sourcelk, transformation_matrixmin);
        save_pcl_to_img1(target00, cloud_standerd_temp);
        if( pipei_result == 0){
            cout <<Path<<":  "<< workpiece_index << "th  ImageMatch succeed" << endl;

            pcl::copyPointCloud(*sourcejs, *cloud_standerd_temp);
            pcl::transformPointCloud(*cloud_standerd_temp, *cloud_standerd_temp, transformation_matrixmin);

                  //viewPair(target,cloud_standerd_temp);

            for (uint n = 0; n < HF_all_num; n++)
            {

                HFnum1->width = 1;
                HFnum1->height = doublesource3d[n].size();
                HFnum1->points.resize(HFnum1->width * HFnum1->height);
                switch (XYZ_flag)
                {
                case 0:
                    for (int ii = 0; ii < doublesource3d[n].size(); ii++)
                    {
                        HFnum1->points[ii].x = doublesource3d[n][ii].z;
                        HFnum1->points[ii].y = doublesource3d[n][ii].y;
                        HFnum1->points[ii].z = doublesource3d[n][ii].x;

                    }
                    break;
                case 1:
                    for (int ii = 0; ii < doublesource3d[n].size(); ii++)
                    {
                        HFnum1->points[ii].x = doublesource3d[n][ii].x;
                        HFnum1->points[ii].y = doublesource3d[n][ii].z;
                        HFnum1->points[ii].z = doublesource3d[n][ii].y;
                    }
                    break;
                case 2:
                    for (int ii = 0; ii < doublesource3d[n].size(); ii++)
                    {
                        HFnum1->points[ii].x = doublesource3d[n][ii].x;
                        HFnum1->points[ii].y = doublesource3d[n][ii].y;
                        HFnum1->points[ii].z = doublesource3d[n][ii].z;
                    }
                    break;
                }

                pcl::transformPointCloud(*HFnum1, *HFnum1, turn_matrix.transformation_matrix);
                pcl::transformPointCloud(*HFnum1, *HFnum1, turn_matrix.transformation_matrix2);
                if(ff==1)
                {
                    pcl::transformPointCloud(*HFnum1, *HFnum1, transformation_matrix_temp11);
                }


                for (int i = 0; i < HFnum1->points.size(); i++)
                {
                    HFnum1->points[i].x = HFnum1->points[i].x - downX + 1.0;
                    HFnum1->points[i].y = HFnum1->points[i].y - downY + 1.0;
                    HFnum1->points[i].z = 0;
                }


                pcl::transformPointCloud(*HFnum1, *HFnum1, transformation_matrixmin);

               // viewPair(target00,cloud_standerd_temp);
                //viewPair(target00,HFnum1);
                //viewPair(sourcelk,HFnum1);
                PointCloud::Ptr target1(new PointCloud);
                pcl::copyPointCloud(*target00, *target1);

                PointCloud::Ptr hf(new PointCloud);
                pcl::copyPointCloud(*HFnum1, *hf);
                double xmin = target1->points[0].x;
                double xmax = target1->points[0].x;
                double ymin = target1->points[0].y;
                double ymax = target1->points[0].y;
                double xnow = 0, ynow = 0;
                int sizetarget1 = target1->points.size();
                int sizehf = hf->points.size();

                for (int i = 0; i < sizetarget1; i++)
                   {
                       xnow = target1->points[i].x;
                       ynow = target1->points[i].y;
                       if (xnow < xmin)xmin = xnow;
                       if (xnow > xmax)xmax = xnow;
                       if (ynow < ymin)ymin = ynow;
                       if (ynow > ymax)ymax = ynow;
                               }

                               for (int i = 0; i < sizehf; i++)
                               {
                                   xnow = hf->points[i].x;
                                   ynow = hf->points[i].y;
                                   if (xnow < xmin)xmin = xnow;
                                   if (xnow > xmax)xmax = xnow;
                                   if (ynow < ymin)ymin = ynow;
                                   if (ynow > ymax)ymax = ynow;
                               }


                               for (int i = 0; i < sizetarget1; i++)
                               {
                                   target1->points[i].x = target1->points[i].x - xmin;
                                   target1->points[i].y = target1->points[i].y - ymin;
                                   target1->points[i].z=0;
                               }


                               for (int i = 0; i < sizehf; i++)
                               {
                                   hf->points[i].x = hf->points[i].x - xmin;
                                   hf->points[i].y = hf->points[i].y - ymin;
                                   hf->points[i].z=0;
                               }
                               pcl::KdTreeFLANN<pcl::PointXYZ> kdtree;
                               float radius=5;
                               pcl::PointXYZ searchPoint;
                               vector<int> pointIdxRadiusSearch;
                               vector<float> pointRadiusSquareDistance;
                               int flag1=0;
                               kdtree.setInputCloud(target1);
                               float mindis1;
                               for(int i=0;i<sizehf;i++)
                               {
                                   searchPoint.x=hf->points[i].x;
                                   searchPoint.y=hf->points[i].y;
                                   searchPoint.z=hf->points[i].z;
                                   if(kdtree.radiusSearch(searchPoint,radius,pointIdxRadiusSearch,pointRadiusSquareDistance)>0)
                                   {

                                           mindis1+=pointRadiusSquareDistance[0];

                                   }
                                   else
                                   {
                                       flag1++;  //未匹配个数

                                   }
                               }
                                   mindis1=mindis1/(sizetarget1-flag1);
                                   cout<<"mindis: "<<mindis1<<endl;
                                   //cout<<"sizetarget: "<<sizetarget<<endl;
                                   cout<<"flag1: "<<flag1<<endl;
                                   cout<<"sizetarget/5: "<<sizetarget1<<endl;
                                       if(flag1<sizetarget1/4)
                                       {

                                              cout<<"焊缝匹配成功"<<endl;
                                              int  put_or_not = 0;
                                             // FILE *fp = NULL;
                                             // fp = fopen(F_PATH，“a+”);
                                             // if(NULL==fp) return -1;
                                             // fprintf(fp,"%s\n",flag1,"%s\n",sizetarget1/7);
                                             // fclose(fp);
                                             // fp = NULL;
                                              Point3d temp1;
                                              temp1.x = HFnum1->points[0].x;
                                              temp1.y = HFnum1->points[0].y;
                                              returnHF.push_back(temp1);
                                              cout << "(" << temp1.x << "," << temp1.y << ")\t";
                                              temp1.x = HFnum1->points[HFnum1->points.size() - 1].x;
                                              temp1.y = HFnum1->points[HFnum1->points.size() - 1].y;
                                              returnHF.push_back(temp1);
                                              cout << "(" <<temp1.x << "," << temp1.y << ")" << endl;

                                       }
                                       else{
                                           cout<<"焊缝匹配失败"<<endl;
                                           returnHF.resize(0);
                                           return returnHF;
                                       }


            }
            vector<vector<Point3d>> HFALL_Three;
            HFALL_Three.resize(returnHF.size()*0.75);
            int ii=0;
            //viewPair(sourcelk,target00);
            for(int i =0;i<returnHF.size();i=i+4)
            {
                Point3d Point0,Point1,Point2,Point3;
                Point0.x=returnHF[i].x;
                Point0.y=returnHF[i].y;
                Point0.z=0;
                Point1.x=returnHF[i+1].x;
                Point1.y=returnHF[i+1].y;
                Point1.z=0;
                Point2.x=returnHF[i+2].x;
                Point2.y=returnHF[i+2].y;
                Point2.z=0;
                Point3.x=returnHF[i+3].x;
                Point3.y=returnHF[i+3].y;
                Point3.z=0;
                vector<Point3d> fourPoint={Point0,Point1,Point2,Point3};

                vector<Point3d> Six_Point=Shi_Duan(fourPoint,sourcelk,20,80, 10 );

                HFALL_Three[ii].resize(2);
                HFALL_Three[ii+1].resize(2);
                HFALL_Three[ii+2].resize(2);
                HFALL_Three[ii][0]=Six_Point[0];
                HFALL_Three[ii][1]=Six_Point[1];
                HFALL_Three[ii+1][0]=Six_Point[2];
                HFALL_Three[ii+1][1]=Six_Point[3];
                HFALL_Three[ii+2][0]=Six_Point[4];
                HFALL_Three[ii+2][1]=Six_Point[5];
                ii=ii+3;
                qDebug()<<"Start Point Detect:"<<'/n';
                for(int j =0;j<Six_Point.size();j++)
                {
                     qDebug()<<Six_Point[j].x<<" "<<Six_Point[j].y<<"      ";
                }

            }
            returnHF.clear();
            //returnHF.resize(2*HFALL_Three.size());
            for(int ii=0;ii<HFALL_Three.size();ii=ii+3)
            {


                /*returnHF[ii].x=HFALL_Three[ii][0];
                returnHF[ii].y=HFALL_Three[ii][1];
                returnHF[ii+1].x=HFALL_Three[ii+1][0];
                returnHF[ii+1].y=HFALL_Three[ii+1][1];
                returnHF[ii+2].x=HFALL_Three[ii+2][0];
                returnHF[ii+2].y=HFALL_Three[ii+2][1];*/

                returnHF.push_back(HFALL_Three[ii][0]);
                returnHF.push_back(HFALL_Three[ii][1]);
                returnHF.push_back(HFALL_Three[ii+1][0]);
                returnHF.push_back(HFALL_Three[ii+1][1]);
                //returnHF.push_back(HFALL_Three[ii+2][0]);
                //returnHF.push_back(HFALL_Three[ii+2][1]);

            }


            qDebug()<< "return returnHF        ";
            qDebug()<< "returnHF.size()        "<<returnHF.size();


            return returnHF;

        }
        else{
            returnHF.resize(0);
            return returnHF;
        }
    }
}

PointCloud::Ptr PointCloud_match::p3dToPCD(vector<cv::Point3d> p3d)
{
    PointCloud::Ptr pcd(new PointCloud);
    pcl::_PointXYZ temp;
    for (int i = 0; i < p3d.size(); i++) {
        temp.x = p3d[i].x; temp.y = p3d[i].y; temp.z = p3d[i].z;
        pcd->points.push_back(temp);
    }
    pcd->width=1;
    pcd->height = pcd->points.size();

    return(pcd);
}

void PointCloud_match::save_pcl_to_img(PointCloud::Ptr target00)
{
    PointCloud::Ptr hebin(new PointCloud);
    pcl::copyPointCloud(*target00, *hebin);

    double xmin = hebin->points[0].x;
    double xmax = hebin->points[0].x;
    double ymin = hebin->points[0].y;
    double ymax = hebin->points[0].y;
    double xnow = 0, ynow = 0;

    int size = hebin->points.size();
    for (int i = 0; i < size; i++)
    {
        xnow = hebin->points[i].x;
        ynow = hebin->points[i].y;
        if (xnow < xmin)xmin = xnow;
        if (xnow > xmax)xmax = xnow;
        if (ynow < ymin)ymin = ynow;
        if (ynow > ymax)ymax = ynow;
    }

    for (int i = 0; i < size; i++)
    {
        hebin->points[i].x = hebin->points[i].x - xmin;
        hebin->points[i].y = hebin->points[i].y - ymin;
    }

    int tuxmin = 0;
    int tuxmax = xmax - xmin + 1;
    int tuymin = 0;
    int tuymax = ymax - ymin + 1;

    Mat src = Mat::zeros(tuymax, tuxmax, CV_8UC1);
    for (int i = 0; i < size; i++)
    {
        src.at<uchar>(int(hebin->points[i].y), int(hebin->points[i].x)) = 255;
    }

    Mat three_channel = Mat::zeros(src.rows, src.cols, CV_8UC3);
    Mat src_zeros = Mat::zeros(src.rows, src.cols, CV_8UC1);
    vector<Mat> channels;

    channels.push_back(src_zeros);
    channels.push_back(src);
    channels.push_back(src_zeros);
    merge(channels, three_channel);
    //    imshow("???????",three_channel);
//    imwrite(PP->save_path().toStdString() + "/" + PP->str_time().toStdString() + "/hfc/ffffffff" + ".jpg", three_channel);
}
void PointCloud_match::save_pcl_to_img(PointCloud::Ptr target00, PointCloud::Ptr cloud_standerd_temp,int workpiece_index,double pipeidu000)
{
    static int ngongjiansave = 1;
        PointCloud::Ptr target(new PointCloud);
        pcl::copyPointCloud(*target00, *target);

        PointCloud::Ptr standerd(new PointCloud);
        pcl::copyPointCloud(*cloud_standerd_temp, *standerd);


        //char str_del1[100];
        //sprintf(str_del1, "/home/fenglin/????/model/match/target_%d.txt", ngongjiansave);
        //ofstream targettxt;
        //targettxt.open(str_del1, ios::out | ios::app);
        //for (int i = 0; i<target->points.size(); i++)
        //{
        //	targettxt << target->points[i].x << "\t" << target->points[i].y << "\t" << target->points[i].z << "\n";
        //}
        //targettxt.close();
        //char str_del2[100];
        //sprintf(str_del2, "/home/fenglin/????/model/match/biaozhun_%d.txt", ngongjiansave);
        //ofstream biaozhuntxt;
        //biaozhuntxt.open(str_del2, ios::out | ios::app);
        //for (int i = 0; i<standerd->points.size(); i++)
        //{
        //	biaozhuntxt << standerd->points[i].x << "\t" << standerd->points[i].y << "\t" << standerd->points[i].z << "\n";
        //}
        //biaozhuntxt.close();

        double xmin = target->points[0].x;
        double xmax = target->points[0].x;
        double ymin = target->points[0].y;
        double ymax = target->points[0].y;
        double xnow = 0, ynow = 0;
        int sizetarget = target->points.size();
        int sizestanderd = standerd->points.size();
        for (int i = 0; i < sizetarget; i++)
        {
            xnow = target->points[i].x;
            ynow = target->points[i].y;
            if (xnow < xmin)xmin = xnow;
            if (xnow > xmax)xmax = xnow;
            if (ynow < ymin)ymin = ynow;
            if (ynow > ymax)ymax = ynow;
        }

        for (int i = 0; i < sizestanderd; i++)
        {
            xnow = standerd->points[i].x;
            ynow = standerd->points[i].y;
            if (xnow < xmin)xmin = xnow;
            if (xnow > xmax)xmax = xnow;
            if (ynow < ymin)ymin = ynow;
            if (ynow > ymax)ymax = ynow;
        }

        for (int i = 0; i < sizetarget; i++)
        {
            target->points[i].x = target->points[i].x - xmin;
            target->points[i].y = target->points[i].y - ymin;
            target->points[i].z=0;
        }


        for (int i = 0; i < sizestanderd; i++)
        {
            standerd->points[i].x = standerd->points[i].x - xmin;
            standerd->points[i].y = standerd->points[i].y - ymin;
            standerd->points[i].z=0;
        }

        int tuxmin = 0;
        int tuxmax = xmax - xmin + 1;
        int tuymin = 0;
        int tuymax = ymax - ymin + 1;

        Mat src_R = Mat::zeros(tuymax, tuxmax, CV_8UC1);
        Mat src_B = Mat::zeros(tuymax, tuxmax, CV_8UC1);
        Mat src_zeros = Mat::zeros(tuymax, tuxmax, CV_8UC1);
        for (int i = 0; i < sizetarget; i++)
        {
            src_R.at<uchar>(int(target->points[i].y), int(target->points[i].x)) = 255;
        }

        for (int i = 0; i < sizestanderd; i++)
        {
            src_B.at<uchar>(int(standerd->points[i].y), int(standerd->points[i].x)) = 255;
        }

        Mat three_channel = Mat::zeros(tuymax, tuxmax, CV_8UC3);
        vector<Mat> channels;
        //viewPair(target,standerd);
        channels.push_back(src_zeros);
        channels.push_back(src_B);
        channels.push_back(src_R);
        merge(channels, three_channel);
        QString path1= PP->save_path() + "/" + PP->str_time() + "/MATCH/";
        string  path2= path1.toStdString();
        int gp=int(pipeidu000);
        imwrite(path2 + to_string(workpiece_index) +"all-"+Common::num2str56(ngongjiansave) + "号匹配结果图__" +to_string(gp)+ ".jpg", three_channel);
        //imshow("src_B",src_B);
        //imshow("src_R",src_R);
        //imshow("three_channel",three_channel);
    //    imwrite(PP->save_path().toStdString() + "/" + PP->str_time().toStdString() + "/hfc/" + num2str2(ngongjiansave) + "hhhhhhhh" + ".bmp", three_channel);
        ngongjiansave++;

}

void PointCloud_match::save_pcl_to_img1(PointCloud::Ptr target00, PointCloud::Ptr cloud_standerd_temp)
{
    static int ngongjiansave = 1;
    int imagepipei = 0;
    PointCloud::Ptr target(new PointCloud);
    pcl::copyPointCloud(*target00, *target);

    PointCloud::Ptr standerd(new PointCloud);
    pcl::copyPointCloud(*cloud_standerd_temp, *standerd);


    //char str_del1[100];
    //sprintf(str_del1, "/home/fenglin/????/model/match/target_%d.txt", ngongjiansave);
    //ofstream targettxt;
    //targettxt.open(str_del1, ios::out | ios::app);
    //for (int i = 0; i<target->points.size(); i++)
    //{
    //	targettxt << target->points[i].x << "\t" << target->points[i].y << "\t" << target->points[i].z << "\n";
    //}
    //targettxt.close();
    //char str_del2[100];
    //sprintf(str_del2, "/home/fenglin/????/model/match/biaozhun_%d.txt", ngongjiansave);
    //ofstream biaozhuntxt;
    //biaozhuntxt.open(str_del2, ios::out | ios::app);
    //for (int i = 0; i<standerd->points.size(); i++)
    //{
    //	biaozhuntxt << standerd->points[i].x << "\t" << standerd->points[i].y << "\t" << standerd->points[i].z << "\n";
    //}
    //biaozhuntxt.close();

    double xmin = target->points[0].x;
    double xmax = target->points[0].x;
    double ymin = target->points[0].y;
    double ymax = target->points[0].y;
    double xnow = 0, ynow = 0;
    int sizetarget = target->points.size();
    int sizestanderd = standerd->points.size();
    for (int i = 0; i < sizetarget; i++)
    {
        xnow = target->points[i].x;
        ynow = target->points[i].y;
        if (xnow < xmin)xmin = xnow;
        if (xnow > xmax)xmax = xnow;
        if (ynow < ymin)ymin = ynow;
        if (ynow > ymax)ymax = ynow;
    }

    for (int i = 0; i < sizestanderd; i++)
    {
        xnow = standerd->points[i].x;
        ynow = standerd->points[i].y;
        if (xnow < xmin)xmin = xnow;
        if (xnow > xmax)xmax = xnow;
        if (ynow < ymin)ymin = ynow;
        if (ynow > ymax)ymax = ynow;
    }

    for (int i = 0; i < sizetarget; i++)
    {
        target->points[i].x = target->points[i].x - xmin;
        target->points[i].y = target->points[i].y - ymin;
        target->points[i].z=0;
    }


    for (int i = 0; i < sizestanderd; i++)
    {
        standerd->points[i].x = standerd->points[i].x - xmin;
        standerd->points[i].y = standerd->points[i].y - ymin;
        standerd->points[i].z=0;
    }

    int tuxmin = 0;
    int tuxmax = xmax - xmin + 1;
    int tuymin = 0;
    int tuymax = ymax - ymin + 1;

    Mat src_R = Mat::zeros(tuymax, tuxmax, CV_8UC1);
    Mat src_B = Mat::zeros(tuymax, tuxmax, CV_8UC1);
    Mat src_zeros = Mat::zeros(tuymax, tuxmax, CV_8UC1);
    for (int i = 0; i < sizetarget; i++)
    {
        src_R.at<uchar>(int(target->points[i].y), int(target->points[i].x)) = 255;
    }

    for (int i = 0; i < sizestanderd; i++)
    {
        src_B.at<uchar>(int(standerd->points[i].y), int(standerd->points[i].x)) = 255;
    }

    Mat three_channel = Mat::zeros(tuymax, tuxmax, CV_8UC3);
    vector<Mat> channels;
    //viewPair(target,standerd);
    channels.push_back(src_zeros);
    channels.push_back(src_B);
    channels.push_back(src_R);
    merge(channels, three_channel);
    QString path1= PP->save_path() + "/" + PP->str_time() + "/MATCH/";
    string  path2= path1.toStdString();
    imwrite(path2 + "all-"+Common::num2str56(ngongjiansave) + "号最小匹配度匹配结果图" + ".jpg", three_channel);
    //imshow("src_B",src_B);
    //imshow("src_R",src_R);
    //imshow("three_channel",three_channel);
//    imwrite(PP->save_path().toStdString() + "/" + PP->str_time().toStdString() + "/hfc/" + num2str2(ngongjiansave) + "hhhhhhhh" + ".bmp", three_channel);
    ngongjiansave++;
    pcl::KdTreeFLANN<pcl::PointXYZ> kdtree;
    float radius=25;
    pcl::PointXYZ searchPoint;
    vector<int> pointIdxRadiusSearch;
    vector<float> pointRadiusSquareDistance;
    int flag=0;
    kdtree.setInputCloud(standerd);
    float mindis;
    for(int i=0;i<sizetarget;i++)
    {
        searchPoint.x=target->points[i].x;
        searchPoint.y=target->points[i].y;
        searchPoint.z=target->points[i].z;
        if(kdtree.radiusSearch(searchPoint,radius,pointIdxRadiusSearch,pointRadiusSquareDistance)>0)
        {

                mindis+=pointRadiusSquareDistance[0];

        }
        else
        {
            flag++;  //未匹配个数

        }


    }
    mindis=mindis/(sizetarget-flag);
    cout<<"mindis: "<<mindis<<endl;
    //cout<<"sizetarget: "<<sizetarget<<endl;
    cout<<"flag: "<<flag<<endl;
    cout<<"sizestanderd/7: "<<sizetarget/5<<endl;
    if(flag>sizetarget/5)
    {

        cout<<"匹配失败"<<endl;
         imagepipei = 1;


    }
    pipei_result = imagepipei;


}

direct_turn_matrix PointCloud_match::direct_turn(PointCloud::Ptr source00)
{
    cout<<"--------------1040---"<<endl;
    int nums = source00->size();
    Point3d p1, p2, p3;
    p1.x = source00->points[1].x;
    p1.y = source00->points[1].y;
    p1.z = source00->points[1].z;
    p2.x = source00->points[nums / 2].x;
    p2.y = source00->points[nums / 2].y;
    p2.z = source00->points[nums / 2].z;

    p3.x = source00->points[nums - 1].x;
    p3.y = source00->points[nums - 1].y;
    p3.z = source00->points[nums - 1].z;
    //????I1
    double I1[3], I2[3];
    I1[0] = p2.x - p1.x;
    I1[1] = p2.y - p1.y;
    I1[2] = p2.z - p1.z;
    //????I2
    I2[0] = p3.x - p1.x;
    I2[1] = p3.y - p1.y;
    I2[2] = p3.z - p1.z;
//cout<<"--------------1064---"<<endl;
    double X1 = I1[0];
    double Y1 = I1[1];
    double Z1 = I1[2];
    double X2 = I2[0];
    double Y2 = I2[1];
    double Z2 = I2[2];

    //??????n=??a,b,c)
    double a, b, c, a1, b1, c1;
    a1 = Y1 * Z2 - Y2 * Z1;
    b1 = X2 * Z1 - X1 * Z2;
    c1 = X1 * Y2 - X2 * Y1;
   // cout<<"--------------1076---"<<endl;
    //cout << "a1=" << a1 << "; b1=" << b1 << "; c1=" << c1 << endl;
    a = a1 / sqrt(a1*a1 + b1 * b1 + c1 * c1);
    b = b1 / sqrt(a1*a1 + b1 * b1 + c1 * c1);
    c = c1 / sqrt(a1*a1 + b1 * b1 + c1 * c1);
    //cout << "sqrt(a1*a1 + b1*b1 + c1*c1)=" << sqrt(a1*a1 + b1*b1 + c1*c1) << endl;
    //cout << "a=" << a << "; b=" << b << "; c=" << c << endl;

    //??????????
    double theta_x = atan(-b / c);

    Eigen::AngleAxisf init_rotation(0.0, Eigen::Vector3f::UnitZ());    //??????????????????????????(?????
    Eigen::AngleAxisf init_rotation1(0.0, Eigen::Vector3f::UnitY());
    Eigen::AngleAxisf init_rotation2(-theta_x, Eigen::Vector3f::UnitX());
    Eigen::Translation3f init_translation(0, 0, 0);
    Eigen::Matrix4f transformation_matrix = (init_translation * init_rotation*init_rotation1*init_rotation2).matrix();

    PointCloud::Ptr source0(new PointCloud);
    source0->width = 1;
    source0->height = 1;
    source0->points.resize(source0->width * source0->height);
    source0->points[0].x = a;
    source0->points[0].y = b;
    source0->points[0].z = c;
    //cout<<"---------------1100--"<<endl;
    pcl::transformPointCloud(*source0, *source0, transformation_matrix);
    a = source0->points[0].x;
    b = source0->points[0].y;
    c = source0->points[0].z;
    //cout << "&&&&&&&&&&&&&& a=" << a << "; b=" << b << "; c=" << c << endl;


    double theta_y = atan(-a / c);

    Eigen::AngleAxisf init_rotation222(0.0, Eigen::Vector3f::UnitZ());    //??????????????????????????(?????
    Eigen::AngleAxisf init_rotation21(theta_y, Eigen::Vector3f::UnitY());
    Eigen::AngleAxisf init_rotation22(0.0, Eigen::Vector3f::UnitX());
    Eigen::Translation3f init_translation2(0, 0, 0);
    Eigen::Matrix4f transformation_matrix2 = (init_translation2 * init_rotation222*init_rotation21*init_rotation22).matrix();

    pcl::transformPointCloud(*source0, *source0, transformation_matrix2);
    a = source0->points[0].x;
    b = source0->points[0].y;
    c = source0->points[0].z;
    //    cout << "&&&&&&&&&&&&&& a=" << a << "; b=" << b << "; c=" << c << endl;

    //    cout << "theta_nn1= " << theta_x << endl;
    //    cout << "theta_nn2= " << theta_y << endl;
    ///////////////////////////////////////????????+????? end

    //viewPair1(source00);
    //    pcl::transformPointCloud(*source00, *source00, transformation_matrix);
    //    pcl::transformPointCloud(*source00, *source00, transformation_matrix2);
    //cout<<"--------------1129---"<<endl;
    direct_turn_matrix turn_matrix;
    turn_matrix.transformation_matrix = transformation_matrix;
    turn_matrix.transformation_matrix2 = transformation_matrix2;
    return turn_matrix;
}

Eigen::Matrix4f PointCloud_match::getMinRect(PointCloud::Ptr target00)
{
    int downGetPoints = 10;
    PointCloud::Ptr hebin(new PointCloud);
    pcl::copyPointCloud(*target00, *hebin);

    double xmin = hebin->points[0].x;
    double xmax = hebin->points[0].x;
    double ymin = hebin->points[0].y;
    double ymax = hebin->points[0].y;
    double xnow = 0, ynow = 0;

    int size = hebin->points.size();
    for (int i = 0; i < size; i++)
    {
        xnow = hebin->points[i].x;
        ynow = hebin->points[i].y;
        if (xnow < xmin)xmin = xnow;
        if (xnow > xmax)xmax = xnow;
        if (ynow < ymin)ymin = ynow;
        if (ynow > ymax)ymax = ynow;
    }

    for (int i = 0; i < size; i++)
    {
        hebin->points[i].x = hebin->points[i].x - xmin + 1000;
        hebin->points[i].y = hebin->points[i].y - ymin + 1000;
    }

    int tuxmin = 0;
    int tuxmax = xmax - xmin + 1;
    int tuymin = 0;
    int tuymax = ymax - ymin + 1;

    Mat src = Mat::zeros(tuymax + 2000, tuxmax + 2000, CV_8UC1);
    Mat srcRGB(src.size(), CV_8UC3);
    vector<Point> src_temp;
    Point temp;
    int y_temp = 0;
    int x_temp = 0;
    //    cout<<src.size()<<endl;
    for (int i = 0; i < size; i = i + downGetPoints)
    {
        y_temp = int(hebin->points[i].y);
        x_temp = int(hebin->points[i].x);
        src.at<uchar>(y_temp, x_temp) = 255;
        temp.y = y_temp;
        temp.x = x_temp;
        src_temp.push_back(temp);
    }


    //    (Tmp_SavePath + "????????????" + ".jpg", src);

    RotatedRect box;
    box = minAreaRect(Mat(src_temp));

    Point2f rect[4];

    float width = 0;
    float height = 0;
    float ratio = 0;

    box.points(rect);


    PointCloud::Ptr hebin_trans(new PointCloud);
    pcl::copyPointCloud(*hebin, *hebin_trans);
    Eigen::AngleAxisf init_rotation_start(0.0, Eigen::Vector3f::UnitZ());    //??????????????????????????(?????
    Eigen::AngleAxisf init_rotation1_start(0.0, Eigen::Vector3f::UnitY());
    Eigen::AngleAxisf init_rotation2_start(0.0, Eigen::Vector3f::UnitX());

    Eigen::AngleAxisf init_rotation(0.0, Eigen::Vector3f::UnitZ());    //??????????????????????????(?????
    Eigen::AngleAxisf init_rotation1(3.14, Eigen::Vector3f::UnitY());
    Eigen::AngleAxisf init_rotation2(0.0, Eigen::Vector3f::UnitX());

    Eigen::Translation3f init_translation_bef(-box.center.x, -box.center.y, 0);
    Eigen::Matrix4f transformation_matrix = (init_translation_bef*init_rotation_start*init_rotation1_start*init_rotation2_start).matrix();


    Eigen::Translation3f init_translation(0, 0, 0);
    transformation_matrix = (init_translation * init_rotation*init_rotation1*init_rotation2).matrix()*transformation_matrix;


    Eigen::Translation3f init_translation_af(box.center.x, box.center.y, 0);
    transformation_matrix = (init_translation_af*init_rotation_start*init_rotation1_start*init_rotation2_start).matrix()*transformation_matrix;


    Eigen::Translation3f init_translation_back(-(-xmin + 1000), -(-ymin + 1000), 0);
    transformation_matrix = (init_translation_back*init_rotation_start*init_rotation1_start*init_rotation2_start).matrix()*transformation_matrix;
    //viewPair(target00,hebin_trans);
    pcl::transformPointCloud(*hebin_trans, *hebin_trans, transformation_matrix);

    //??????????????????
    //for (int i = 0; i < size; i = i + downGetPoints)
    //{
    //	y_temp = int(hebin_trans->points[i].y);
    //	x_temp = int(hebin_trans->points[i].x);
    //	src.at<uchar>(y_temp, x_temp) = 255;
    //}
    //cvtColor(src, srcRGB, COLOR_GRAY2BGR);
    //for (int j = 0; j < 4; j++)
    //{
    //	line(srcRGB, rect[j], rect[(j + 1) % 4], Scalar(0, 0, 255), 1, 8);//???????????????\FF????
    //}

    //namedWindow("?????", 0);
    //cvResizeWindow("?????", 500, 500);
    //imshow("?????", srcRGB);
    //waitKey(0);
    ////imwrite(PP->save_path().toStdString() + "/" + PP->str_time().toStdString() + "/hfc/????????????" + ".jpg", three_channel);

    //viewPair(target00,hebin_trans);
    return  transformation_matrix;
}


