#ifndef SCATTERVIEWER_H
#define SCATTERVIEWER_H

#include <QtDataVisualization>
#include <QtDataVisualization/q3dscatter.h>
#include <QtDataVisualization/qabstract3dseries.h>
#include <QtGui/QFont>

using namespace QtDataVisualization;
using namespace std;

class WorkPiece;

/********************************************************
 * @ClassName:  ScatterViewer
 * @Brief:      3D引擎代理类,完成QtDataVisualization模块驱动,
 *              工件数据模型的建立和维护,以及和外部(主界面,数据表单,
 *              数据计算线程)的通信
 * @date        2020/08/31
 ********************************************************/
class ScatterViewer : public QObject
{
    Q_OBJECT
public:
    explicit ScatterViewer(Q3DScatter *scatter);
    ~ScatterViewer();

    void del_current();

    QMap<int, WorkPiece *> workpiece_map() const;

    void select_workpiece(int workpiece_index);       //选择编号index的工件

    void reset();

    void clear_select();

public slots:
    void add_one_workpiece(int workpiece_index,                 /*工件编号*/
                           std::vector<cv::Point3d> pcd,        /*该工件点云数据信息pointCloud*/
                           std::string name,                    /*工件对应的字符*/
                           std::vector<cv::Point3d> pcd_mesh);
    void match_started(int workpiece_index);
    void match_finished(int workpiece_index, std::string name, bool flag);


signals:
    void data_added(int index);
    void data_changed(int index);
    void selectedSeriesChanged(int index);

private:
    Q3DScatter *m_graph;

    QMap<int, WorkPiece*> _workpiece_map;
    int current_index;

//    int m_fontSize;
//    QAbstract3DSeries::Mesh m_style;
//    bool m_smooth;
//    int m_itemCount;
//    float m_curveDivider;
//    QCustom3DLabel *label;

//    QVector<QVector4D> mesh;

};

#endif // SCATTERVIEWER_H
