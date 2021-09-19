#ifndef PARAMETER_H
#define PARAMETER_H

#include <QObject>

#define PP Parameter::instance()

//测试模式
enum TEST_MODE{
    OFFLINE = 0,
    ONLINE = 1
};

//相机状态
struct CameraStatus{
    QString sn;     //相机sn
    bool isOn;      //是否启用
};

/********************************************************
 * @ClassName:  Parameter
 * @Brief:      单例模式,参数保存类
 *              并且存储一些全局变量,包括相机图片,拍摄位置等信息
 * @date        2020/08/31
 ********************************************************/
class Parameter : public QObject
{
    Q_OBJECT
public:
    static Parameter *instance();
    static void closeInstance();

    //全局变量


    double dx;          //龙门位置

    //计算使用
    std::vector<int> cameraMap(){
        std::vector<int> result;

        for (int i = 0; i < camera_list().count(); ++i) {
            CameraStatus c_s = camera_list().at(i);
            if(c_s.isOn){
                result.push_back(i+1);
            }
        }
        return result;
    }

    int max_cam_num(){
        return camera_list().count();
    }

    std::vector<double>  Longmen_x_s(){
        QMutexLocker m_lock(&mutex);
        return _shot_longmen_pos_record.toStdVector();
    }

    std::vector<double>  Longmen_x_s(int camera_num){
        QMutexLocker m_lock(&mutex);
        QVector<double> result = _shot_longmen_pos_record;
        for (int i = 0; i < _shot_longmen_pos_modify.count(); ++i) {
            if((int)_shot_longmen_pos_modify.at(i).x() == camera_num){
                int n = (int)_shot_longmen_pos_modify.at(i).y();
                if(n <= result.count() && n > 0){
                    result[n-1] = _shot_longmen_pos_modify.at(i).z();
                }
            }
        }
        return result.toStdVector();
    }

    //功能函数
    void load();
    void save();
    void reset(){
        QMutexLocker m_lock(&mutex);
        _shot_longmen_pos_record.clear();
        _CameraImg_PFA.clear();
        _shot_longmen_pos_modify.clear();
    }
    int active_camera_num();                    //返回激活的摄像头数目
    QStringList active_sn_list();               //返回激活的sn列表（仅在线模式有效）
    QList<CameraStatus> camera_list() const;    //在线离线统一的接口,返回所有检测到的相机（包含未启用的）
    bool is_should_shot(double pos);            //判断是否应当拍照
    void cameraImg_PFA_append(cv::Mat mat){
        _CameraImg_PFA.push_back(mat);
    }


    //工具函数
    QString rect_to_string(QRect r);
    QRect string_to_rect(QString str);
    int sn_to_index(QString sn);        //根据sn查询相机位置（仅在线模式有效）
    QImage cvMat_to_QImage(const cv::Mat &mtx);
    static bool check_training_model();


    //Getter and Setter
    double longmen_start_pos() const;
    void setLongmen_start_pos(double longmen_start_pos);

    double longmen_end_pos() const;
    void setLongmen_end_pos(double longmen_end_pos);

    QString remote_address() const;
    void setRemote_address(const QString &remote_address);

    int remote_port() const;
    void setRemote_port(int remote_port);

    QString self_address() const;
    void setSelf_address(const QString &self_address);

    int self_port() const;
    void setSelf_port(int self_port);

    int test_mode() const;
    void setTest_mode(int test_mode);

    int camera_num() const;
    void setCamera_num(int camera_num);

    QString save_path() const;
    void setSave_path(const QString &save_path);

    QRect ROI_first() const;
    void setROI_first(const QRect &ROI_first);

    QRect ROI_middle() const;
    void setROI_middle(const QRect &ROI_middle);

    QRect ROI_last() const;
    void setROI_last(const QRect &ROI_last);

    double camera_shot_interval() const;
    void setCamera_shot_interval(double camera_shot_interval);

    QString offline_path() const;
    void setOffline_path(const QString &offline_path);

    QList<bool> offline_camera_list() const;
    void setOffline_camera_list(const QList<bool> &offline_camera_list);

    QList<CameraStatus> online_camera_list() const;
    void setOnline_camera_list(const QList<CameraStatus> &online_camera_list);

    int camera_shot_times() const;
    void setCamera_shot_times(int camera_shot_times);

    double l_shift() const;
    void setL_shift(double l_shift);

    std::vector<cv::Mat> CameraImg_PFA() const;

    QString compute_path() const;
    void setCompute_path(const QString &compute_path);

    int camera_exposure_time() const;
    void setCamera_exposure_time(int camera_exposure_time);

    bool is_last_in_edge() const;
    void setIs_last_in_edge(bool is_last_in_edge);

    bool is_first_in_edge() const;
    void setIs_first_in_edge(bool is_first_in_edge);

    QString str_time() const;
    void setstr_time(const QString &str_time);

    QString test_dir() const;
    void setTest_dir(const QString &test_dir);

    QString key_code() const;
    void setKey_code(const QString &key_code);

    QDateTime key_code_time() const;

    QVector<QVector3D> shot_longmen_pos_modify() const;
    void shot_longmen_pos_modify_append(const QVector3D &v){
        QMutexLocker m_lock(&mutex);
        _shot_longmen_pos_modify.append(v);
    }

    QVector<double> shot_longmen_pos_record() const;
    void shot_longmen_pos_record_append(double p){
        QMutexLocker m_lock(&mutex);
        _shot_longmen_pos_record.append(p);
    }

signals:

public slots:

private:
    static Parameter *m_pInstance;

private:
    explicit Parameter(QObject *parent = nullptr);

    QSettings *settings;

    //数据
    double _longmen_start_pos;      //龙门开始位置(mm)
    double _longmen_end_pos;        //龙门结束位置(mm)
    double _camera_shot_interval;   //拍摄间隔(mm)
    int _camera_shot_times;         //拍摄次数
    QString _remote_address;        //服务器地址
    int _remote_port;               //服务器端口
    QString _self_address;          //本机地址
    int _self_port;                 //本机端口
    int _test_mode;                 //测试模式（本地0，在线1）
    QString _offline_path;          //本地图片路径
    QString _save_path;             //保存路径
    QString _compute_path;          //计算数据路径
    int _camera_exposure_time;      //相机曝光时间
    QRect _ROI_first;               //ROI参数1
    QRect _ROI_middle;              //ROI参数2
    QRect _ROI_last;                //ROI参数3
    bool _is_first_in_edge;            //首相机位于边缘
    bool _is_last_in_edge;             //尾相机位于边缘
    QList<bool> _offline_camera_list;  //离线模式下相机列表
    QList<CameraStatus> _online_camera_list;         //在线模式下相机列表
    double _l_shift;                //龙门偏移量
    QString _test_dir;              //压力测试文件夹
    QString _key_code;              //注册密钥
    QDateTime _key_code_time;       //注册时间

    std::vector<cv::Mat> _CameraImg_PFA;    //所有拍摄的图像

    QString _str_time;
    QMutex mutex;

    QVector<double> _shot_longmen_pos_record;        //记录已拍摄的龙门位置
    QVector<QVector3D> _shot_longmen_pos_modify;     //记录拍摄的异常点
};

#endif // PARAMETER_H
