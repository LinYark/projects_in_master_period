#include "parameter.h"

Parameter* Parameter::m_pInstance = 0;

Parameter *Parameter::instance()
{
    if(!m_pInstance)
        m_pInstance = new Parameter;

    return m_pInstance;
}

void Parameter::closeInstance()
{
    if(m_pInstance)
        delete m_pInstance;
    m_pInstance = 0;
}

QVector<double> Parameter::shot_longmen_pos_record() const
{
    return _shot_longmen_pos_record;
}

QVector<QVector3D> Parameter::shot_longmen_pos_modify() const
{
    return _shot_longmen_pos_modify;
}

std::vector<cv::Mat> Parameter::CameraImg_PFA() const
{
    return _CameraImg_PFA;
}

//判断是否应当拍照
bool Parameter::is_should_shot(double pos){
    QMutexLocker m_lock(&mutex);
    if(_shot_longmen_pos_record.isEmpty()){      //首次拍摄
//        shot_longmen_pos_record.append(pos);
        return true;
    }
    else{
        if( qAbs(qAbs(_shot_longmen_pos_record.last() - pos) - _camera_shot_interval) < 15 ){
//            shot_longmen_pos_record.append(pos);
            return true;
        }
        else{
            return false;
        }
    }
}

Parameter::Parameter(QObject *parent) : QObject(parent)
{
    QString m_qstrFileName = QCoreApplication::applicationDirPath() + "/Config.ini";
    settings = new QSettings(m_qstrFileName, QSettings::IniFormat);

    //变量初始化
    load();
}

QDateTime Parameter::key_code_time() const
{
    return _key_code_time;
}

QString Parameter::key_code() const
{
    return _key_code;
}

void Parameter::setKey_code(const QString &key_code)
{
    _key_code = key_code;
    _key_code_time = QDateTime::currentDateTime();
    save();
}

QString Parameter::test_dir() const
{
    return _test_dir;
}

void Parameter::setTest_dir(const QString &test_dir)
{
    _test_dir = test_dir;
}

bool Parameter::is_first_in_edge() const
{
    return _is_first_in_edge;
}

void Parameter::setIs_first_in_edge(bool is_first_in_edge)
{
    _is_first_in_edge = is_first_in_edge;
}

bool Parameter::is_last_in_edge() const
{
    return _is_last_in_edge;
}

void Parameter::setIs_last_in_edge(bool is_last_in_edge)
{
    _is_last_in_edge = is_last_in_edge;
}

int Parameter::camera_exposure_time() const
{
    return _camera_exposure_time;
}

void Parameter::setCamera_exposure_time(int camera_exposure_time)
{
    _camera_exposure_time = camera_exposure_time;
}

QString Parameter::compute_path() const
{
    return _compute_path;
}

void Parameter::setCompute_path(const QString &compute_path)
{
    _compute_path = compute_path;
}

double Parameter::l_shift() const
{
    return _l_shift;
}

void Parameter::setL_shift(double l_shift)
{
    _l_shift = l_shift;
}

int Parameter::camera_shot_times() const
{
    return _camera_shot_times;
}

void Parameter::setCamera_shot_times(int camera_shot_times)
{
    _camera_shot_times = camera_shot_times;
}

QList<CameraStatus> Parameter::online_camera_list() const
{
    return _online_camera_list;
}

void Parameter::setOnline_camera_list(const QList<CameraStatus> &online_camera_list)
{
    _online_camera_list = online_camera_list;
}

QList<bool> Parameter::offline_camera_list() const
{
    return _offline_camera_list;
}

void Parameter::setOffline_camera_list(const QList<bool> &offline_camera_list)
{
    _offline_camera_list = offline_camera_list;
}

QString Parameter::offline_path() const
{
    return _offline_path;
}

void Parameter::setOffline_path(const QString &offline_path)
{
    _offline_path = offline_path;
}

double Parameter::camera_shot_interval() const
{
    return _camera_shot_interval;
}

void Parameter::setCamera_shot_interval(double camera_shot_interval)
{
    _camera_shot_interval = camera_shot_interval;
}

QRect Parameter::ROI_last() const
{
    return _ROI_last;
}

void Parameter::setROI_last(const QRect &ROI_last)
{
    _ROI_last = ROI_last;
}

QRect Parameter::ROI_middle() const
{
    return _ROI_middle;
}

void Parameter::setROI_middle(const QRect &ROI_middle)
{
    _ROI_middle = ROI_middle;
}

QRect Parameter::ROI_first() const
{
    return _ROI_first;
}

void Parameter::setROI_first(const QRect &ROI_first)
{
    _ROI_first = ROI_first;
}

QString Parameter::save_path() const
{
    return _save_path;
}

void Parameter::setSave_path(const QString &save_path)
{
    _save_path = save_path;
}

int Parameter::test_mode() const
{
    return _test_mode;
}

void Parameter::setTest_mode(int test_mode)
{
    _test_mode = test_mode;
}

int Parameter::self_port() const
{
    return _self_port;
}

void Parameter::setSelf_port(int self_port)
{
    _self_port = self_port;
}

QString Parameter::self_address() const
{
    return _self_address;
}

void Parameter::setSelf_address(const QString &self_adress)
{
    _self_address = self_adress;
}

int Parameter::remote_port() const
{
    return _remote_port;
}

void Parameter::setRemote_port(int remote_port)
{
    _remote_port = remote_port;
}

QString Parameter::remote_address() const
{
    return _remote_address;
}

void Parameter::setRemote_address(const QString &remote_address)
{
    _remote_address = remote_address;
}

double Parameter::longmen_end_pos() const
{
    return _longmen_end_pos;
}

void Parameter::setLongmen_end_pos(double longmen_end_pos)
{
    _longmen_end_pos = longmen_end_pos;
}

double Parameter::longmen_start_pos() const
{
    return _longmen_start_pos;
}

void Parameter::setLongmen_start_pos(double longmen_start_pos)
{
    _longmen_start_pos = longmen_start_pos;
}

QString Parameter::str_time() const
{
    return _str_time;
}

void Parameter::setstr_time(const QString &str_time)
{
    _str_time = str_time;
}

void Parameter::load()
{
    _longmen_start_pos = settings->value("_longmen_start_pos").isValid() ? settings->value("_longmen_start_pos").toInt() : 2775;
    _longmen_end_pos = settings->value("_longmen_end_pos").isValid() ? settings->value("_longmen_end_pos").toInt() : -1815;
    _l_shift = settings->value("_l_shift").isValid() ? settings->value("_l_shift").toDouble() : 0.01;
    _camera_shot_interval = settings->value("_camera_shot_interval").isValid() ? settings->value("_camera_shot_interval").toInt() : 400;
    _camera_shot_times = settings->value("_camera_shot_times").isValid() ? settings->value("_camera_shot_times").toInt() : 10;
    _remote_address = settings->value("_remote_address").isValid() ? settings->value("_remote_address").toString() : QString("192.168.4.128");
    _remote_port = settings->value("_remote_port").isValid() ? settings->value("_remote_port").toInt() : 12345;
    _self_address = settings->value("_self_address").isValid() ? settings->value("_self_address").toString() : QString("127.0.0.1 ");
    _self_port = settings->value("_self_port").isValid() ? settings->value("_self_port").toInt() : 12345;
    _test_mode = settings->value("_test_mode").isValid() ? settings->value("_test_mode").toInt() : 0;
    _camera_exposure_time = settings->value("_camera_exposure_time").isValid() ? settings->value("_camera_exposure_time").toInt() : 15000;
    _offline_path = settings->value("_offline_path").isValid() ? settings->value("_offline_path").toString()
                                                                         : QString(QCoreApplication::applicationDirPath() + "/offline_pic/");
    _save_path = settings->value("_save_path").isValid() ? settings->value("_save_path").toString()
                                                         : QString(QCoreApplication::applicationDirPath() + "/saved_pic/");
    _compute_path = settings->value("_compute_path").isValid() ? settings->value("_compute_path").toString()
                                                         : QString(QCoreApplication::applicationDirPath());
    _ROI_first = settings->value("_ROI_first").isValid() ? settings->value("_ROI_first").toRect() : QRect(10,10,1900,1180);
    _ROI_middle = settings->value("_ROI_middle").isValid() ? settings->value("_ROI_middle").toRect() : QRect(610,10,1300,1180);
    _ROI_last = settings->value("_ROI_last").isValid() ? settings->value("_ROI_last").toRect() : QRect(10,10,1900,1180);

    _is_first_in_edge = settings->value("_is_first_in_edge").isValid() ? settings->value("_is_first_in_edge").toBool(): true;
    _is_last_in_edge = settings->value("_is_last_in_edge").isValid() ? settings->value("_is_last_in_edge").toBool(): true;
    _test_dir = settings->value("_test_dir").isValid() ? settings->value("_test_dir").toString()
                                                         : QString(QCoreApplication::applicationDirPath());
    _key_code = settings->value("_key_code").isValid() ? settings->value("_key_code").toString(): QString();
    _key_code_time = settings->value("_key_code_time").isValid() ? settings->value("_key_code_time").toDateTime(): QDateTime();

    QStringList tmp = settings->value("_offline_camera_list").isValid() ? settings->value("_offline_camera_list").toStringList() : QStringList();
    foreach (QString str, tmp) {
        _offline_camera_list.append((bool)str.toInt());
    }

    tmp = settings->value("_online_camera_list").isValid() ? settings->value("_online_camera_list").toStringList() : QStringList();
    for (int i = 0; i < tmp.count()/2; ++i) {
        CameraStatus c_s;
        c_s.sn = tmp.at(i * 2);
        c_s.isOn = (bool)(tmp.at(i * 2 + 1).toInt());
        _online_camera_list.append(c_s);
    }
    qDebug()<<"load setting completed!";
}

void Parameter::save()
{
    settings->setValue("_longmen_start_pos", _longmen_start_pos);
    settings->setValue("_longmen_end_pos", _longmen_end_pos);
    settings->setValue("_l_shift", _l_shift);
    settings->setValue("_camera_shot_interval", _camera_shot_interval);
    settings->setValue("_camera_shot_times", _camera_shot_times);
    settings->setValue("_remote_address", _remote_address);
    settings->setValue("_remote_port", _remote_port);
    settings->setValue("_self_address", _self_address);
    settings->setValue("_self_port", _self_port);
    settings->setValue("_test_mode", _test_mode);
    settings->setValue("_camera_exposure_time", _camera_exposure_time);
    settings->setValue("_offline_path", _offline_path);
    settings->setValue("_save_path", _save_path);
    settings->setValue("_compute_path", _compute_path);
    settings->setValue("_ROI_first", _ROI_first);
    settings->setValue("_ROI_middle", _ROI_middle);
    settings->setValue("_ROI_last", _ROI_last);
    settings->setValue("_is_first_in_edge", _is_first_in_edge);
    settings->setValue("_is_last_in_edge", _is_last_in_edge);
    settings->setValue("_test_dir", _test_dir);
    settings->setValue("_key_code", _key_code);
    settings->setValue("_key_code_time", _key_code_time);

    QStringList tmp;
    foreach (bool f, _offline_camera_list) {
        tmp.append(QString::number((int)f));
    }
    settings->setValue("_offline_camera_list", tmp);

    QStringList tmp1;
    foreach (CameraStatus c_s, _online_camera_list) {
        tmp1.append(c_s.sn);
        tmp1.append(QString::number((int)c_s.isOn));
    }
    settings->setValue("_online_camera_list", tmp1);
}

int Parameter::active_camera_num()
{
    int num = 0;
    switch (_test_mode) {
    case OFFLINE:
        foreach (bool f, _offline_camera_list) {
            if(f){
                num++;
            }
        }
        break;
    case ONLINE:
        foreach (CameraStatus c_s, _online_camera_list) {
            if(c_s.isOn){
                num++;
            }
        }
        break;
    default:
        break;
    }
    return num;
}

QStringList Parameter::active_sn_list()
{
    QStringList list;
    if(_test_mode == ONLINE){
        foreach (CameraStatus c_s, _online_camera_list) {
            if(c_s.isOn){
                list.append(c_s.sn);
            }
        }
    }
    return list;
}

QList<CameraStatus> Parameter::camera_list() const
{
    if(_test_mode == ONLINE){
        return _online_camera_list;
    }
    else{
        QList<CameraStatus> list;
        foreach (bool f, _offline_camera_list) {
            CameraStatus c_s;
            c_s.isOn = f;
            c_s.sn = "-";
            list.append(c_s);
        }
        return list;
    }
}


QString Parameter::rect_to_string(QRect r)
{
    return QString("%1,%2,%3,%4").arg(r.x()).arg(r.y()).arg(r.width()).arg(r.height());
}

QRect Parameter::string_to_rect(QString str)
{
    QStringList list = str.split(',');
    if(list.count() != 4){
        qDebug()<<"读取ROI发生错误！";
        return QRect();
    }
    else{
        return QRect(list.at(0).toInt(), list.at(1).toInt(), list.at(2).toInt(), list.at(3).toInt());
    }
}

int Parameter::sn_to_index(QString sn){
    for (int i = 0; i < _online_camera_list.count(); ++i) {
        if(_online_camera_list.at(i).sn == sn){
            return i;
        }
    }
    return -1;
}

QImage Parameter::cvMat_to_QImage(const cv::Mat &mtx)
{
    switch (mtx.type())
    {
    case CV_8UC1:
    {
        qDebug()<<"cvMat_to_QImage::CV_8UC1";
        QImage img((const unsigned char *)(mtx.data), mtx.cols, mtx.rows, mtx.cols, QImage::Format_Grayscale8);
        return img.rgbSwapped();
    }
        break;
    case CV_8UC3:
    {
        qDebug()<<"cvMat_to_QImage::CV_8UC3";
        QImage img((const unsigned char *)(mtx.data), mtx.cols, mtx.rows, mtx.cols * 3, QImage::Format_RGB888);
        return img.rgbSwapped();
    }
        break;
    case CV_8UC4:
    {
        qDebug()<<"cvMat_to_QImage::CV_8UC4";
        QImage img((const unsigned char *)(mtx.data), mtx.cols, mtx.rows, mtx.cols * 4, QImage::Format_ARGB32);
        return img.rgbSwapped();
    }
        break;
    default:
    {
        qDebug()<<"cvMat_to_QImage::NONE";
        QImage img;
        return img;
    }
        break;
    }
}

bool Parameter::check_training_model(){

#ifndef FULLY_BUILD
    return true;
#endif
    QString model_file   = PP->compute_path()+"/wangluo/WELD_new.prototxt";        //训练模型地址
    QFile file;
    file.setFileName(model_file);
    if(file.exists()){
        return true;
    }
    else{
        return false;
    }
}









