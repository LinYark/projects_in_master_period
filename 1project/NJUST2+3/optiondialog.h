#ifndef OPTIONDIALOG_H
#define OPTIONDIALOG_H

#include <QDialog>

namespace Ui {
class OptionDialog;
}

class QButtonGroup;
struct CameraStatus;

/********************************************************
 * @ClassName:  OptionDialog
 * @Brief:      [设置]对话框,完成各种设置参数的设置,并对相机
 *              进行管理
 * @date        2020/08/31
 ********************************************************/
class OptionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit OptionDialog(QWidget *parent = nullptr);
    ~OptionDialog();

    int exec();


public slots:
    void camera_init_finished(QStringList d_list);
    void start_auto_test();      //开启自动测试

private slots:
    void on_pbt_previous_clicked();

    void on_pbt_next_clicked();

    void on_pbt_up_clicked();

    void on_pbt_down_clicked();

    void icon_button_clicked(int i);        //点击开关相机之后

    void check_btn_state();         //刷新组件状态

    void on_pbt_cancel_auto_clicked();

    void on_tbt_datasaved_clicked();

    void on_tbt_data_clicked();

    void on_pbt_save_clicked();

    void on_tbt_computePath_clicked();

    void on_lineEdit_camera_shot_times_editingFinished();

signals:
    void camera_init();         //开启相机

    void exposure_time_changed();   //曝光时间改变

private:
    Ui::OptionDialog *ui;

    //控件组
    QButtonGroup *sn_group;         //编号按钮列表
    QButtonGroup *icon_group;       //相机按钮列表
    QList<QWidget*> widget_list;    //相机窗体列表

    //数据组
    QStringList real_sn_list;               //真实相机sn列表
    QList<bool> offline_camera_list;        //离线相机状态列表
    QList<CameraStatus> online_camera_list; //在线模式下相机列表


    int camera_num;                 //最大相机数

    void ui_init();                 //ui初始化
    void adjust_camera_list();      //根据输入/实际连接的最大相机数，修正相机列表
    void save();                    //保存参数

    QTimer *timer;
    int countdown;          //倒计时

};

#endif // OPTIONDIALOG_H
