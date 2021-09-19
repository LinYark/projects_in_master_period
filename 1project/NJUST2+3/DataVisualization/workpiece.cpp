#include "workpiece.h"

#define DENSITY 2       //数据密度，防止画面过于卡顿

WorkPiece::WorkPiece(int workpiece_index, std::string name, std::vector<cv::Point3d> pcd_mesh, QObject *parent)
    : QObject(parent)
{
    array = new QScatterDataArray;
    proxy = new QScatterDataProxy;
    _series = new QScatter3DSeries(proxy);
    _series->setName(QString::number(workpiece_index));      //用name保存编号（选择状态改变时方便查找）
    _index = workpiece_index;
    _character = QString::fromStdString(name);
    if(_character.isEmpty() || _character == "none"){
        _isCharSuccess = false;
        status = char_failed;
    }
    else{
        _isCharSuccess = true;
        status = char_success;
    }

    for (uint i = 0; i < pcd_mesh.size(); ++i) {
        if(i % DENSITY != 0){
            continue;
        }
        QScatterDataItem item;
        item.setPosition(QVector3D(pcd_mesh.at(i).x,
                                   pcd_mesh.at(i).y,
                                   pcd_mesh.at(i).z));
        array->append(item);
    }
    _series->dataProxy()->resetArray(array);

    _isDel = false;
    _isSelect = false;

    timer_matching = new QTimer;
    connect(timer_matching, &QTimer::timeout, [this]{
        if(_series->baseColor() == QColor(Qt::green)){
            _series->setBaseColor(Qt::darkCyan);
        }
        else{
            _series->setBaseColor(Qt::green);
        }
    });

    update();
}

void WorkPiece::update()
{
//    if(_isMatchSuccess){
//        _series->setBaseColor(Qt::green);
//        status_text = tr("匹配成功，焊缝已发送");
//    }
//    else if(_isCharSuccess){
//        _series->setBaseColor(Qt::cyan);
//        status_text = tr("字符检测成功，待匹配");
//    }
//    else{
//        _series->setBaseColor(Qt::red);
//        status_text = tr("字符检测失败");
//    }
    switch (status) {
    case char_failed:
        _series->setBaseColor(Qt::gray);
        status_text = tr("字符检测失败");
        break;
    case char_success:
        _series->setBaseColor(Qt::cyan);
        status_text = tr("字符检测成功，等待匹配");
        break;
    case matching:
        status_text = tr("正在匹配中");
        break;
    case match_success:
        _series->setBaseColor(Qt::green);
        status_text = tr("匹配成功，焊缝已发送");
        break;
    case match_failed:
        _series->setBaseColor(Qt::red);
        status_text = tr("匹配失败");
        break;
    default:
        break;
    }

    //选择状态
    if(_isSelect){
        _series->setBaseColor(Qt::blue);
    }

    //文字格式
    _series->setItemLabelFormat(tr("编号:%1 字符:%2 状态:%3").arg(_index).arg(_character).arg(status_text));

    //是否隐藏
    _series->setVisible(!_isDel);
}

void WorkPiece::match_started()
{
    status = matching;
    timer_matching->start(800);
}

void WorkPiece::clear()
{

}

int WorkPiece::index() const
{
    return _index;
}

QScatter3DSeries *WorkPiece::series() const
{
    return _series;
}

QString WorkPiece::character() const
{
    return _character;
}

void WorkPiece::setCharacter(const QString &character)
{
    _character = character;
    if(_character.isEmpty() || _character == "none"){
        _isCharSuccess = false;
    }
    else{
        _isCharSuccess = true;
        if(status == char_failed){
            status = char_success;
        }
    }

    emit dateChanged(_index);
}

bool WorkPiece::isDel() const
{
    return _isDel;
}

void WorkPiece::setIsDel(bool isDel)
{
    _isDel = isDel;
    emit dateChanged(_index);
}

bool WorkPiece::isCharSuccess() const
{
    return _isCharSuccess;
}

void WorkPiece::setIsCharSuccess(bool isCharSuccess)
{
    _isCharSuccess = isCharSuccess;
    emit dateChanged(_index);
}

bool WorkPiece::isMatchSuccess() const
{
    return _isMatchSuccess;
}

void WorkPiece::setIsMatchSuccess(bool isMatchSuccess)
{
    _isMatchSuccess = isMatchSuccess;
    timer_matching->stop();
//    qDebug()<<"timer_matching->stop()"<< _index;
    status = isMatchSuccess ? match_success : match_failed;
    emit dateChanged(_index);
}

bool WorkPiece::isSelect() const
{
    return _isSelect;
}

void WorkPiece::setIsSelect(bool isSelect)
{
    _isSelect = isSelect;
}
