#include "scatterdatamodifier.h"
#include <QtDataVisualization/qscatterdataproxy.h>
#include <QtDataVisualization/qvalue3daxis.h>
#include <QtDataVisualization/q3dscene.h>
#include <QtDataVisualization/q3dcamera.h>
#include <QtDataVisualization/qscatter3dseries.h>
#include <QtDataVisualization/q3dtheme.h>
#include <QtCore/qmath.h>
#include <QtWidgets/QComboBox>
#include <QtDebug>

const int numberOfItems = 3600;
const float curveDivider = 3.0f;
const int lowerNumberOfItems = 900;
const float lowerCurveDivider = 0.75f;

ScatterDataModifier::ScatterDataModifier(Q3DScatter *scatter)
    : m_graph(scatter),
      m_fontSize(40.0f),
      m_style(QAbstract3DSeries::MeshSphere),
      m_smooth(true),
      m_itemCount(lowerNumberOfItems),
      m_curveDivider(lowerCurveDivider)
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


    //选择变色
    connect(m_graph, &Q3DScatter::selectedSeriesChanged, [this](QScatter3DSeries *series){
        series = m_graph->selectedSeries();
        if(series != NULL){
            QStringList n_strlist = series->itemLabel().split(' ');
            if(n_strlist.isEmpty())
                return;
            QStringList list = n_strlist.first().split(':');
            if(list.count() < 2)
                return;

            int n = list.at(1).toInt();
            qDebug()<<"n = "<<n;
            if(_workpiece_map.contains(n)){
                _current_character = _workpiece_map.value(n)->character;
                _current_result = _workpiece_map.value(n)->result;
                _current_number = _workpiece_map.value(n)->number;
                qDebug()<<"number"<<_current_number << "\tchar" << _current_character << "\tresult"<<_current_result;
            }
            foreach (WorkPiece *p, _workpiece_map) {
                p->reset();
                if(p->number == n){
                    p->series->setBaseColor(Qt::blue);
                }
            }
        }
    });

    //填充数据
//    addData1();

//    connect(m_graph, &Q3DScatter::)

}

ScatterDataModifier::~ScatterDataModifier()
{
    qDebug()<<"~delete";
//    data_clear();
    delete m_graph;
}

void ScatterDataModifier::setData1(QVector<QVector4D> mesh,QStringList character_list,QVector<int> result_list)
{
    qDebug()<<"计算完成!,mesh大小"<<mesh.size()<<"*4";
    qDebug()<<"字符串"<<character_list;
    qDebug()<<"比对结果"<<result_list;\

//    label->setText("请进行字符校验SSS");
//    m_graph->setTitle("请进行字符校验");

    emit set_title("请进行字符校验");
    data_clear();

    for(int i = 0; i< mesh.size(); i++) {
        if(i % 2 != 0){
            continue;
        }
        QScatterDataItem item;
        item.setPosition(QVector3D(mesh.at(i).x(),
                                   mesh.at(i).y(),
                                   mesh.at(i).z()));

        int n = mesh.at(i).w();

        if(_workpiece_map.contains(n)){      //有则加入
            _workpiece_map.value(n)->array->append(item);
        }
        else{                               //没有则新建
            QScatterDataArray *dataArray = new QScatterDataArray;
            dataArray->append(item);
            QScatterDataProxy *proxy = new QScatterDataProxy;
            proxy->addItem(item);
            QScatter3DSeries *series = new QScatter3DSeries(proxy);
            m_graph->addSeries(series);
            WorkPiece *new_workPiece = new WorkPiece;
            new_workPiece->number = n;
            new_workPiece->array = dataArray;
            new_workPiece->proxy = proxy;
            new_workPiece->series = series;
            qDebug()<<"n="<<n << "\tresult:"<<result_list.at(n-1)<<"\tcharacter:"<<character_list.at(n-1);

            if( n <=  character_list.size() && n > 0){
                new_workPiece->character = character_list.at(n-1);
            }
            else{
                new_workPiece->character = QString();
            }

            if( n <=  result_list.size() && n > 0){
                new_workPiece->result = (bool)result_list.at(n-1);
            }
            else{
                new_workPiece->result = false;
            }

            new_workPiece->char_check();
            new_workPiece->reset();

            _workpiece_map.insert(n, new_workPiece);
        }
    }
    foreach (WorkPiece *p, _workpiece_map) {
        p->series->dataProxy()->resetArray(p->array);
    }
    //    qDebug()<<"workpiece_map.count()"<<workpiece_map.count();
}

void ScatterDataModifier::del_current()
{
    if(m_graph->selectedSeries()){
        _current_result = -1;
        _current_character = QString::number(-1);
        _workpiece_map[_current_number]->character = _current_character;
        _workpiece_map[_current_number]->result = -1;
        _workpiece_map[_current_number]->char_check();
        _workpiece_map[_current_number]->reset();
    }
}

void ScatterDataModifier::setUnMatched()
{
//    label->setText("等待进行工件匹配");
//    m_graph->setTitle("正在匹配");
    emit set_title("正在匹配");
    foreach (WorkPiece *p, _workpiece_map) {
        if(p->status != del){
            p->status = match_start;
            p->reset();
        }
    }

}

bool ScatterDataModifier::check_character()
{
    bool result = true;
    foreach (WorkPiece *p, _workpiece_map) {
        if(p->status == char_failed){
            result = false;
        }
    }
    qDebug()<<"字符校验结果："<<result;
//    if(result){
//        emit set_title("字符校验已经全部通过，请点发送键发送");

//    }
    return result;
}

int ScatterDataModifier::current_number() const
{
    return _current_number;
}

void ScatterDataModifier::setCurrent_number(int current_number)
{
    _current_number = current_number;
}

int ScatterDataModifier::current_result() const
{
    return _current_result;
}

void ScatterDataModifier::setCurrent_result(int current_result)
{
    _current_result = current_result;
    _workpiece_map[_current_number]->result = current_result;
    _workpiece_map[_current_number]->reset();
}

QString ScatterDataModifier::current_character() const
{
    return _current_character;
}

void ScatterDataModifier::setCurrent_character(QString current_character)
{
    _current_character = current_character;
    _workpiece_map[_current_number]->character = current_character;
    _workpiece_map[_current_number]->char_check();
    _workpiece_map[_current_number]->reset();
}


void ScatterDataModifier::changeStyle(int style)
{
    QComboBox *comboBox = qobject_cast<QComboBox *>(sender());
    if (comboBox) {
        m_style = QAbstract3DSeries::Mesh(style+1);
        if (m_graph->seriesList().size())
            m_graph->seriesList().at(0)->setMesh(m_style);
    }
}

void ScatterDataModifier::setSmoothDots(int smooth)
{
    m_smooth = bool(smooth);
    QScatter3DSeries *series = m_graph->seriesList().at(0);
    series->setMeshSmooth(m_smooth);
}

void ScatterDataModifier::changeTheme(int theme)
{
    Q3DTheme *currentTheme = m_graph->activeTheme();
    currentTheme->setType(Q3DTheme::Theme(theme));
    emit backgroundEnabledChanged(currentTheme->isBackgroundEnabled());
    emit gridEnabledChanged(currentTheme->isGridEnabled());
    emit fontChanged(currentTheme->font());
}

void ScatterDataModifier::changePresetCamera()
{
    static int preset = Q3DCamera::CameraPresetFrontLow;

    m_graph->scene()->activeCamera()->setCameraPreset((Q3DCamera::CameraPreset)preset);

    if (++preset > Q3DCamera::CameraPresetDirectlyBelow)
        preset = Q3DCamera::CameraPresetFrontLow;
}

void ScatterDataModifier::changeLabelStyle()
{
//    m_graph->activeTheme()->setLabelBackgroundEnabled(!m_graph->activeTheme()->isLabelBackgroundEnabled());
    m_graph->activeTheme()->setLabelBackgroundEnabled(false);
    m_graph->activeTheme()->setLabelBackgroundColor(Qt::red);
}

void ScatterDataModifier::changeFont(const QFont &font)
{
    QFont newFont = font;
    newFont.setPointSizeF(m_fontSize);
    m_graph->activeTheme()->setFont(newFont);
}

void ScatterDataModifier::shadowQualityUpdatedByVisual(QAbstract3DGraph::ShadowQuality sq)
{
    int quality = int(sq);
    emit shadowQualityChanged(quality); // connected to a checkbox in main.cpp
}

void ScatterDataModifier::startMatch(int index)
{
    if(_workpiece_map.contains(index)){
        qDebug() << "开始匹配" + _workpiece_map[index]->character + "工件";
        emit set_title("开始匹配" + _workpiece_map[index]->character + "工件");
        _workpiece_map[index]->status = match_start;
        _workpiece_map[index]->reset();
    }
    else{
        qDebug()<<"工件序号错误"<<index;
    }
}

void ScatterDataModifier::matchSuccess(int index)
{
    if(_workpiece_map.contains(index)){
        qDebug()<<"工件匹配成功!焊缝已发送!";
        emit set_title("匹配成功!焊缝已发送!");
        _workpiece_map[index]->status = match_success;
        _workpiece_map[index]->reset();
    }
    else{
        qDebug()<<"工件序号错误"<<index;
    }
}

void ScatterDataModifier::matchFailed(int index)
{
    if(_workpiece_map.contains(index)){
        qDebug() << _workpiece_map[index]->character + "工件匹配失败";
        emit set_title( _workpiece_map[index]->character + "工件匹配失败");
        _workpiece_map[index]->status = match_failed;
        _workpiece_map[index]->reset();
    }
    else{
        qDebug()<<"工件序号错误"<<index;
    }
}

void ScatterDataModifier::matchPending(int index)
{
    if(_workpiece_map.contains(index)){
        qDebug() << _workpiece_map[index]->character + "工件匹配待定！";
        emit set_title( _workpiece_map[index]->character + "工件匹配待定！");
        _workpiece_map[index]->status = match_pending;
        _workpiece_map[index]->reset();
    }
    else{
        qDebug()<<"工件序号错误"<<index;
    }
}

void ScatterDataModifier::add_one_workpiece(int workpiece_index, std::vector<cv::Point3d> pcd,
                                            string name, std::vector<cv::Point3d> pcd_mesh)
{
    Q_UNUSED(pcd)
    QScatterDataArray *dataArray = new QScatterDataArray;
    QScatterDataProxy *proxy = new QScatterDataProxy;
    QScatter3DSeries *series = new QScatter3DSeries(proxy);
    m_graph->addSeries(series);
    WorkPiece *new_workPiece = new WorkPiece;
    new_workPiece->number = workpiece_index;
    new_workPiece->array = dataArray;
    new_workPiece->proxy = proxy;
    new_workPiece->series = series;
    new_workPiece->character = QString::fromStdString(name);
    new_workPiece->char_check();
    new_workPiece->reset();

    for (uint i = 0; i < pcd_mesh.size(); ++i) {
        if(i % 2 != 0){
            continue;
        }
        QScatterDataItem item;
        item.setPosition(QVector3D(pcd_mesh.at(i).x,
                                   pcd_mesh.at(i).y,
                                   pcd_mesh.at(i).z));
        new_workPiece->array->append(item);
//        new_workPiece->proxy->addItem(item);
    }
    new_workPiece->series->dataProxy()->resetArray(new_workPiece->array);
    _workpiece_map.insert(workpiece_index, new_workPiece);

    emit data_changed();
}

void ScatterDataModifier::match_finished(int workpiece_index, string name, bool flag)
{
    emit data_changed();
}


void ScatterDataModifier::changeShadowQuality(int quality)
{
    QAbstract3DGraph::ShadowQuality sq = QAbstract3DGraph::ShadowQuality(quality);
    m_graph->setShadowQuality(sq);
}

void ScatterDataModifier::setBackgroundEnabled(int enabled)
{
    m_graph->activeTheme()->setBackgroundEnabled((bool)enabled);
}

void ScatterDataModifier::setGridEnabled(int enabled)
{
    m_graph->activeTheme()->setGridEnabled((bool)enabled);
}
//! [8]

void ScatterDataModifier::toggleItemCount()
{
    if (m_itemCount == numberOfItems) {
        m_itemCount = lowerNumberOfItems;
        m_curveDivider = lowerCurveDivider;
    } else {
        m_itemCount = numberOfItems;
        m_curveDivider = curveDivider;
    }
    m_graph->seriesList().at(0)->dataProxy()->resetArray(0);
//    addData();
}

QVector3D ScatterDataModifier::randVector()
{
    return QVector3D(
                (float)(rand() % 100) / 2.0f - (float)(rand() % 100) / 2.0f,
                (float)(rand() % 100) / 100.0f - (float)(rand() % 100) / 100.0f,
                (float)(rand() % 100) / 2.0f - (float)(rand() % 100) / 2.0f);
}

QMap<int, ScatterDataModifier::WorkPiece *> ScatterDataModifier::workpiece_map() const
{
    return _workpiece_map;
}

void ScatterDataModifier::data_clear()
{
    qDebug()<<"~delete";
    foreach (WorkPiece *p, _workpiece_map) {

        m_graph->removeSeries(p->series);
    }
    _workpiece_map.clear();
}

//void ScatterDataModifier::set_title(QString text)
//{
//    if(label){
//        m_graph->removeCustomItem(label);
//    }

//    QFont titleFont = QFont("微软雅黑", 30);
//    label = new QCustom3DLabel("Oil Rigs on Imaginary Sea", titleFont,
//                                                        QVector3D(0.0f, 0.8f, 0.0f),
//                                                        QVector3D(2.0f, 2.0f, 0.0f),
//                                                        QQuaternion());
//    label->setText(text);
//    label->setBorderEnabled(false);
//    label->setBackgroundEnabled(false);
//    label->setTextColor(Qt::black);
//    label->setPositionAbsolute(true);
//    label->setFacingCamera(true);
//    m_graph->addCustomItem(label);
//}
