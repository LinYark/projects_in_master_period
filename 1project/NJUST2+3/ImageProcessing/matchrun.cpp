#include "matchrun.h"

#ifdef FULLY_BUILD
#include "Compute/Match.h"
#endif

MatchRun::MatchRun(int workpiece_index, std::vector<cv::Point3d> pcd, std::string name, QObject *parent) : QObject(parent)
{
    _workpiece_index = workpiece_index;
    _pcd = pcd;
    _name = name;
}

void MatchRun::run()
{
    qDebug()<<"匹配线程开始:"<<_workpiece_index;
    //替换成匹配执行代码

#ifdef FULLY_BUILD
    PM = new PointCloud_match;
    std::vector<std::vector<cv::Point3d>> weldData;
//    std::string str;

    qDebug()<<"void MatchThread::add_one_workpiece"<<_workpiece_index;

//    qDebug()<<"name:       "<<QString::fromStdString(_name);


    PointCloud::Ptr target;

    char Path[200];
    memset(Path, '\0', 200);
    cout<<"000000000000000000000000000000000000000000000000000000000000000000000000"<<_name<<endl;
    string Path_tmp = PM->strToPath(_name);     //根据工件字符，给出工件的标准模型的路径
    strcpy(Path, Path_tmp.c_str());

//    qDebug()<<"Path:       "<<Path;




    if (Path_tmp=="none")     //没有找到路径时，即没有此编码工件的标准模型，跳出此次循环
    {
        qDebug()<<"std::string name: "<<QString::fromStdString(_name);
//        qDebug()<< "未检测出字符，不进行匹配，";
//        qDebug() << "工件匹配失败00000！" << endl;
        emit match_finished(_workpiece_index, _name, false);
    }
    else
    {
//        qDebug()<< "target = PM->p3dToPCD(pcd);";
        target = PM->p3dToPCD(_pcd);

//        qDebug()<< "target = PM->p3dToPCD(pcd)      ok";

        std::vector<cv::Point3d> weldDataall = PM->Match(_workpiece_index, target, Path);

        if (weldDataall.size() > 0)
        {
            if(weldDataall.size()%2 == 0)
            {
                weldData.resize(weldDataall.size()/2);
                for (int m = 0; m < weldData.size(); m++)
                {
                    weldData[m].push_back(weldDataall[m*2]);
                    weldData[m].push_back(weldDataall[m*2+1]);
                }
            }

            qDebug() << "匹配成功，发送焊缝！" << endl;
            emit send_weldData(weldData, _name);
            emit match_finished(_workpiece_index, _name, true);
        }
        else
        {
            qDebug() << "工件匹配失败！" << endl;
            emit match_finished(_workpiece_index, _name, false);
        }


    }
#else
    int time = 2 + qrand() % 5;
    QThread::sleep(time);
    bool ok = qrand() % 2;
    emit match_finished(_workpiece_index, _name, ok);
#endif

    qDebug()<<"匹配线程结束:"<<_workpiece_index;
//    emit send_weldData(weldData, str);
}
