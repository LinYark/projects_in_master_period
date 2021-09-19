//===================================================================================================================================
#include <iostream>
#include "LunKuoPiPeiHY.h"

//点云可视化
void viewPair(PointCloud::Ptr pcd_src, PointCloud::Ptr pcd_tgt)
{
    //int vp_1, vp_2;
    // Create a PCLVisualizer object
    pcl::visualization::PCLVisualizer viewer("registration Viewer");
    //viewer.createViewPort (0.0, 0, 0.5, 1.0, vp_1);
    // viewer.createViewPort (0.5, 0, 1.0, 1.0, vp_2);
    pcl::visualization::PointCloudColorHandlerCustom<pcl::PointXYZ> src_h(pcd_src, 0, 255, 0);
    pcl::visualization::PointCloudColorHandlerCustom<pcl::PointXYZ> tgt_h(pcd_tgt, 255, 0, 0);
    viewer.addPointCloud(pcd_src, src_h, "source cloud");
    viewer.addPointCloud(pcd_tgt, tgt_h, "tgt cloud");
    //viewer.addPointCloud(pcd_final, final_h, "final cloud");
    //viewer.addCoordinateSystem(1.0);
    while (!viewer.wasStopped())
    {
        viewer.spinOnce(100);
        boost::this_thread::sleep(boost::posix_time::microseconds(100000));
    }
}

//===================================================================================================================================
//º¯ÊýÃû³Æ£ºLunKuoPiPei::LunKuoPiPeiHY_11()
//º¯Êý¹ŠÄÜ£º°Ñ¹€ŒþÖÐµÄÈýÎ¬ÆœÃæÂÖÀª×ª»¯ÎªÈýÎ¬Á¢ÌåµãÔÆÊýŸÝ
//===================================================================================================================================
PointCloud::Ptr LunKuoPiPei::LunKuoPiPeiHY_11(PointCloud::Ptr cloud1)
{
    PointCloud::Ptr source0(new PointCloud);
    Mat image1;
    vector<Point2d>XiangSu1;
    Point2d tu1;
    double Zpoint;

    int NumPoint = cloud1->width*cloud1->height;    //µãÔÆµÄÊýÁ¿
    for (int i = 0; i < NumPoint; i++)
    {
        tu1.x = cloud1->points[i].x;
        tu1.y = cloud1->points[i].y;    //ÂÖÀªµãÔÆµÄXY×ø±ê
        XiangSu1.push_back(tu1);
    }
    Zpoint = cloud1->points[0].z;    //ÂÖÀªµãÔÆµÄZ×ø±ê

    //ÒÔÏÂÊÇZÖáµþŒÓÐÎ³ÉµãÔÆ
    int num = 2;
    int num1 = 30 * num;
    int n1 = NumPoint*num;
    vector<Point3d> points1(NumPoint*num);

    for (int i = 0; i < num; i++)
    {
        for (int j = 0; j < NumPoint; j++)
        {
            points1[NumPoint*i + j].x = (double)XiangSu1[j].x;    //X
            points1[NumPoint*i + j].y = (double)XiangSu1[j].y;    //Y
            points1[NumPoint*i + j].z = (rand() / double(RAND_MAX))*double(num1) + double(num1*i)+ Zpoint;    //Z
        }    //ÒªÈ¡µÃ0¡«1Ö®ŒäµÄž¡µãÊý£¬¿ÉÒÔÊ¹ÓÃrand() / double(RAND_MAX)¡£
    }
    //°ÑÆœÃæÂÖÀªµãÔÆ×ª»»³ÉÁ¢ÌåµãÔÆ

    // Fill in the cloud data
    source0->width = 100;
    source0->height = NumPoint*num / 100;
    source0->points.resize(source0->width * source0->height);

    for (size_t i = 0; i < source0->points.size(); i++)
    {
        source0->points[i].x = points1[i].x;
        source0->points[i].y = points1[i].y;
        source0->points[i].z = points1[i].z;
    }
    //****************************************ÒÔÉÏÊÇŽÓ¶þÎ¬ÍŒZÖáµþŒÓÉú³ÉµãÔÆÊýŸÝ***************************************
    return source0;
}

//===================================================================================================================================
//º¯ÊýÃû³Æ£ºLunKuoPiPei::LunKuoPiPeiHY_2()
//º¯Êý¹ŠÄÜ£º°ÑÁœ×éµãÔÆÊýŸÝŸ­¹ýSACµãÔÆŽÖÅä×ŒºÍICPµãÔÆŸ«Åä×ŒÖ®ºó£¬µÃµœŸ«È·Åä×ŒºóµÄµãÔÆÊýŸÝ
//===================================================================================================================================
PiPei LunKuoPiPei::LunKuoPiPeiHY_2(PointCloud::Ptr cloud1, PointCloud::Ptr cloud2)
{
    PiPei ShuChu;
    double PiPeiDu;    //Æ¥Åä¶È
//    viewPair(cloud1, cloud2);
    PointCloud::Ptr cloud_src_o(new PointCloud);//原点云，待配准
   // pcl::io::loadPCDFile("/home/ssl/QT/0929/source.pcd", *cloud_src_o);
    PointCloud::Ptr cloud_tgt_o(new PointCloud);//目标点云
    //pcl::io::loadPCDFile("/home/ssl/QT/0929/target.pcd", *cloud_tgt_o);
    pcl::copyPointCloud(*cloud1, *cloud_src_o); //复制
    pcl::copyPointCloud(*cloud2, *cloud_tgt_o); //复制

    clock_t start = clock();
    std::vector<int> indices_src; //保存去除的点的索引
    pcl::removeNaNFromPointCloud(*cloud_src_o, *cloud_src_o, indices_src);

    //std::cout << "remove *cloud_src_o nan" << endl;

    //下采样滤波
    pcl::VoxelGrid<pcl::PointXYZ> voxel_grid;
    voxel_grid.setLeafSize(26, 26, 26);
    voxel_grid.setInputCloud(cloud_src_o);
    PointCloud::Ptr cloud_src(new PointCloud);
    voxel_grid.filter(*cloud_src);

    //std::cout << "down size *cloud_src_o from " << cloud_src_o->size() << "to" << cloud_src->size() << endl;

    pcl::io::savePCDFileASCII("bunny_src_down.pcd", *cloud_src);
    //计算表面法线
    pcl::NormalEstimation<pcl::PointXYZ, pcl::Normal> ne_src;
    ne_src.setInputCloud(cloud_src);
    pcl::search::KdTree< pcl::PointXYZ>::Ptr tree_src(new pcl::search::KdTree< pcl::PointXYZ>());
    ne_src.setSearchMethod(tree_src);
    pcl::PointCloud<pcl::Normal>::Ptr cloud_src_normals(new pcl::PointCloud< pcl::Normal>);
    ne_src.setRadiusSearch(55);
    ne_src.compute(*cloud_src_normals);

    std::vector<int> indices_tgt;
    pcl::removeNaNFromPointCloud(*cloud_tgt_o, *cloud_tgt_o, indices_tgt);

    //std::cout << "remove *cloud_tgt_o nan" << endl;


    pcl::VoxelGrid<pcl::PointXYZ> voxel_grid_2;
    voxel_grid_2.setLeafSize(32, 32, 32);
    voxel_grid_2.setInputCloud(cloud_tgt_o);
    PointCloud::Ptr cloud_tgt(new PointCloud);
    voxel_grid_2.filter(*cloud_tgt);

    //std::cout << "down size *cloud_tgt_o.pcd from " << cloud_tgt_o->size() << "to" << cloud_tgt->size() << endl;

    pcl::io::savePCDFileASCII("bunny_tgt_down.pcd", *cloud_tgt);

    pcl::NormalEstimation<pcl::PointXYZ, pcl::Normal> ne_tgt;
    ne_tgt.setInputCloud(cloud_tgt);
    pcl::search::KdTree< pcl::PointXYZ>::Ptr tree_tgt(new pcl::search::KdTree< pcl::PointXYZ>());
    ne_tgt.setSearchMethod(tree_tgt);
    pcl::PointCloud<pcl::Normal>::Ptr cloud_tgt_normals(new pcl::PointCloud< pcl::Normal>);
    //ne_tgt.setKSearch(20);
    ne_tgt.setRadiusSearch(55);
    ne_tgt.compute(*cloud_tgt_normals);

    //计算FPFH
    pcl::FPFHEstimation<pcl::PointXYZ, pcl::Normal, pcl::FPFHSignature33> fpfh_src;
    fpfh_src.setInputCloud(cloud_src);
    fpfh_src.setInputNormals(cloud_src_normals);
    pcl::search::KdTree<PointT>::Ptr tree_src_fpfh(new pcl::search::KdTree<PointT>);
    fpfh_src.setSearchMethod(tree_src_fpfh);
    pcl::PointCloud<pcl::FPFHSignature33>::Ptr fpfhs_src(new pcl::PointCloud<pcl::FPFHSignature33>());
    fpfh_src.setRadiusSearch(90);
    fpfh_src.compute(*fpfhs_src);

    //std::cout << "compute *cloud_src fpfh" << endl;

    pcl::FPFHEstimation<pcl::PointXYZ, pcl::Normal, pcl::FPFHSignature33> fpfh_tgt;
    fpfh_tgt.setInputCloud(cloud_tgt);
    fpfh_tgt.setInputNormals(cloud_tgt_normals);
    pcl::search::KdTree<PointT>::Ptr tree_tgt_fpfh(new pcl::search::KdTree<PointT>);
    fpfh_tgt.setSearchMethod(tree_tgt_fpfh);
    pcl::PointCloud<pcl::FPFHSignature33>::Ptr fpfhs_tgt(new pcl::PointCloud<pcl::FPFHSignature33>());
    fpfh_tgt.setRadiusSearch(90);
    fpfh_tgt.compute(*fpfhs_tgt);
    //std::cout << "compute *cloud_tgt fpfh" << endl;

    //SAC配准
    pcl::SampleConsensusInitialAlignment<pcl::PointXYZ, pcl::PointXYZ, pcl::FPFHSignature33> scia;
    scia.setInputSource(cloud_src);
    scia.setInputTarget(cloud_tgt);
    scia.setSourceFeatures(fpfhs_src);
    scia.setTargetFeatures(fpfhs_tgt);
    //scia.setMinSampleDistance(1);
    //scia.setNumberOfSamples(2);
    //scia.setCorrespondenceRandomness(20);
    PointCloud::Ptr sac_result(new PointCloud);
    scia.align(*sac_result);

    //std::cout << "sac has converged:" << scia.hasConverged() << "  score: " << scia.getFitnessScore() << endl;

    Eigen::Matrix4f sac_trans;
    sac_trans = scia.getFinalTransformation();
    //pcl::io::savePCDFileASCII("junjie_transformed_sac.pcd", *sac_result);
    clock_t sac_time = clock();

    Eigen::Matrix4f init_transform, init_transform_ni;
    init_transform_ni = sac_trans.inverse().cast<float>();

    //std::cout << init_transform_ni << endl;
    //std::cout<<init_transform_ni(2,2)<<endl;
    double x,y;
    double r31,r33,r32,r21,r11;
    r31=init_transform_ni(2,0);
    r33=init_transform_ni(2,2);
    r32=init_transform_ni(2,1);
    r21=init_transform_ni(1,0);
    r11=init_transform_ni(0,0);
    x=atan2(r32,r33);
    y=atan2(-r31,sqrt(r32*r32+r33*r33));
    std::cout<<"the angle of x: "<<x<<endl;
    std::cout<<"the angle of y: "<<y<<endl;

    if((-3.14/2<x&&x<3.14/2)&&(-3.14/2<y&&y<3.14/2))
        ShuChu.flag=1;
    else if(x>3.14/2&&y>3.14/2)
        ShuChu.flag=1;
    else if(x<-3.14/2&&y<-3.14/2)
        ShuChu.flag=1;
    else
        ShuChu.flag=-1;


    //viewPair(cloud_tgt, sac_result);
    //visualize_pcd(sac_result, cloud_tgt_o, sac_result);

    //icp配准
    PointCloud::Ptr icp_result(new PointCloud);
    pcl::IterativeClosestPoint<pcl::PointXYZ, pcl::PointXYZ> icp;
    icp.setInputSource(cloud_src);
    icp.setInputTarget(cloud_tgt_o);
    //Set the max correspondence distance to 4cm (e.g., correspondences with higher distances will be ignored)
    icp.setMaxCorrespondenceDistance(80);
    // 最大迭代次数
    icp.setMaximumIterations(1000);
    // 两次变化矩阵之间的差值
    icp.setTransformationEpsilon(1e-10);
    // 均方误差
    icp.setEuclideanFitnessEpsilon(0.00001);
    icp.align(*icp_result, sac_trans);

    PiPeiDu = icp.getFitnessScore();

    clock_t end = clock();

//    cout << "total time: " << (double)(end - start) / (double)CLOCKS_PER_SEC << " s" << endl;
//    //我把计算法线和点特征直方图的时间也算在SAC里面了
//    cout << "sac time: " << (double)(sac_time - start) / (double)CLOCKS_PER_SEC << " s" << endl;
//    cout << "icp time: " << (double)(end - sac_time) / (double)CLOCKS_PER_SEC << " s" << endl;.

//    std::cout << "ICP has converged:" << icp.hasConverged()
//        << " score: " << icp.getFitnessScore() << std::endl;
    Eigen::Matrix4f icp_trans;
    icp_trans = icp.getFinalTransformation();

    Eigen::Matrix4f transformation_matrix0, transformation_matrix0_ni;
    transformation_matrix0_ni = icp_trans.inverse().cast<float>();

    //cout<<"ransformationProbability"<<icp.getTransformationProbability()<<endl;

    //std::cout << transformation_matrix0_ni << endl;

    //使用创建的变换对未过滤的输入点云进行变换
    pcl::transformPointCloud(*cloud_src_o, *icp_result, icp_trans);
    //保存转换的输入点云
    //pcl::io::savePCDFileASCII("junjie_transformed_sac_icp.pcd", *icp_result);

    //cout<<"pi pei du ==================================="<<PiPeiDu<<endl;
//    viewPair(cloud_tgt_o, icp_result);
    //viewPair(cloud_tgt_o, cloud1);
    //viewPair(cloud_tgt_o, cloud2);
    ShuChu.result00=icp_result;
    ShuChu.init_transform_ni00 = init_transform_ni;
    ShuChu.transformation_matrix0_ni00 = transformation_matrix0_ni;
    ShuChu.PiPeiDu00 = PiPeiDu;
    cout<<"outpipeidu: "<<ShuChu.PiPeiDu00<<endl;
    return ShuChu;
}

