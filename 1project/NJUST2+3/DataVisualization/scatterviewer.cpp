#include "scatterviewer.h"
#include "workpiece.h"

ScatterViewer::ScatterViewer(Q3DScatter *scatter)
    : QObject(scatter), m_graph(scatter)
{
    m_graph->scene()->activeCamera()->setCameraPreset(Q3DCamera::CameraPresetFront);

    m_graph->axisX()->setTitle("X Lable");
    m_graph->axisY()->setTitle("Y Lable");
    m_graph->axisZ()->setTitle("Z Lable");

    m_graph->axisX()->setTitleVisible(false);
    m_graph->axisY()->setTitleVisible(false);
    m_graph->axisZ()->setTitleVisible(false);

    m_graph->axisX()->setSegmentCount(1);
    m_graph->axisY()->setSegmentCount(1);
    m_graph->axisZ()->setSegmentCount(1);

    m_graph->axisX()->setLabelFormat("");
    m_graph->axisY()->setLabelFormat("");
    m_graph->axisZ()->setLabelFormat("");

    m_graph->setShadowQuality(QAbstract3DGraph::ShadowQualityNone);

    m_graph->activeTheme()->setGridEnabled(false);

    current_index = -1;

    //选择变色
    connect(m_graph, &Q3DScatter::selectedSeriesChanged, [this](QScatter3DSeries *series){
        series = m_graph->selectedSeries();
        if(series != NULL){
            int n = series->name().toInt();
            emit selectedSeriesChanged(n);
            if(_workpiece_map.contains(current_index)){
                auto last_p = _workpiece_map.value(current_index);
                last_p->setIsSelect(false);
                last_p->update();
            }
            if(_workpiece_map.contains(n)){
                auto curr_p = _workpiece_map.value(n);
                curr_p->setIsSelect(true);
                curr_p->update();
                current_index = n;
            }
        }
        else{
            emit selectedSeriesChanged(-1);
            if(_workpiece_map.contains(current_index)){
                auto last_p = _workpiece_map.value(current_index);
                last_p->setIsSelect(false);
                last_p->update();
            }
            current_index = -1;
        }
    });
}

ScatterViewer::~ScatterViewer()
{
    this->reset();
}

void ScatterViewer::del_current()
{
    if(m_graph->selectedSeries()){
        _workpiece_map[current_index]->setIsDel(true);
        _workpiece_map[current_index]->update();
        current_index = -1;
    }
}

void ScatterViewer::add_one_workpiece(int workpiece_index, std::vector<cv::Point3d> pcd, string name, std::vector<cv::Point3d> pcd_mesh)
{
    Q_UNUSED(pcd)

    WorkPiece *new_workPiece = new WorkPiece(workpiece_index, name, pcd_mesh);
    connect(new_workPiece, &WorkPiece::dateChanged, this, &ScatterViewer::data_changed);
    m_graph->addSeries(new_workPiece->series());
    _workpiece_map.insert(workpiece_index, new_workPiece);

    emit data_added(workpiece_index);
}

void ScatterViewer::match_started(int workpiece_index)
{
    if(_workpiece_map.contains(workpiece_index)){
        _workpiece_map.value(workpiece_index)->match_started();
    }
}

void ScatterViewer::match_finished(int workpiece_index, string name, bool flag)
{
    if(_workpiece_map.contains(workpiece_index)){
        WorkPiece *p = _workpiece_map.value(workpiece_index);
        p->setCharacter(QString::fromStdString(name));
        p->setIsMatchSuccess(flag);
        p->update();
    }

}

QMap<int, WorkPiece *> ScatterViewer::workpiece_map() const
{
    return _workpiece_map;
}

void ScatterViewer::select_workpiece(int workpiece_index)
{
    if(current_index == workpiece_index)
        return;

    if(_workpiece_map.contains(workpiece_index)){
        WorkPiece *p = _workpiece_map.value(workpiece_index);
        p->series()->setSelectedItem(0);
    }
}

void ScatterViewer::reset()
{
    foreach (auto s, m_graph->seriesList()) {
        m_graph->removeSeries(s);
    }

    qDeleteAll(_workpiece_map);
    _workpiece_map.clear();
}

void ScatterViewer::clear_select()
{
    m_graph->clearSelection();
}
