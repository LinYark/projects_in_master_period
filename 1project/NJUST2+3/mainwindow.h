#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class ConfigDialog;
class OptionDialog;
class CameraThread;
class ServerCtrl;
class ScatterDataModifier;
class PicBoxForm;
class LongmenForm;
class ImageProcessing;
class CharDialog;
class VirtualServer;
class ClipThread;
class HebingThread;
class MatchThread;
class CheckDialog;
class ScatterViewer;

using namespace QtDataVisualization;
using namespace std;

/********************************************************
 * @ClassName:  MainWindow
 * @Brief:      主界面UI类,程序的主入口,管理组织所有ui和线程对象,
 *              管理各个线程之间的通信
 * @date        2020/08/31
 ********************************************************/
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

signals:
    void take_a_shot(double);

    void shot_finished();       //拍摄完成

    void camera_reset();

    void one_test_finished();

private slots:
    void show_mesh_data(QVector<QVector4D>,QStringList,QVector<int>);

    void on_act_Option_triggered();

    void on_act_Del_triggered();

    void on_act_Character_triggered();

    void on_act_examine_char_complete_triggered();

    void on_act_send_triggered();

    void on_act_abort_triggered();

    void on_act_Test_triggered();

    void net_load_ready();

    void on_action_triggered();

    void on_act_Register_triggered();

protected:
    void timerEvent(QTimerEvent *e);

private:
    Ui::MainWindow *ui;

    OptionDialog *optionDlg;            //参数设置（主线程）
    PicBoxForm *picBoxForm;             //图片显示（主线程）
    LongmenForm *longmenForm;           //龙门显示
    CharDialog *charDialog;             //字符校验
    CameraThread *camera_thread;        //相机控制线程
    ServerCtrl *serverctrl;             //服务器通信线程
    VirtualServer *virtualServer;       //虚拟服务器
    ImageProcessing *imageProcessing;   //计算线程

    ClipThread *clipThread;             //修剪线程
    HebingThread *hebingThread;         //合并线程
    MatchThread *matchThread;           //匹配线程

    int timerId;

    //3D显示
    QWidget *container;
    Q3DScatter *graph;
    ScatterDataModifier *modifier;
    ScatterViewer *scatterViewer;
    CheckDialog *checkDialog;

    void reset_ui();

    void save_log();
    void save_longmen_pos_modify();     //保存拍照异常点
};

#endif // MAINWINDOW_H
