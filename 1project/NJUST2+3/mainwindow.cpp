#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "optiondialog.h"
#include "mainwindow.h"
#include "Camera/camerathread.h"
#include "Server/serverctrl.h"
#include <Q3DScatter>
#include <QMessageBox>
#include "DataVisualization/scatterdatamodifier.h"
#include "ImageProcessing/imageprocessing.h"
#include "Server/parameter.h"
#include "picboxform.h"
#include "longmenform.h"
#include "chardialog.h"
#include "Server/virtualserver.h"
#include "abortdialog.h"
#include "testdialog.h"
#include "ImageProcessing/clipthread.h"
#include "ImageProcessing/hebingthread.h"
#include "ImageProcessing/matchthread.h"
#include "DataVisualization/checkdialog.h"
#include "DataVisualization/scatterviewer.h"
#include "SecretKey/registerdialog.h"
#include <QTimer>

Q_DECLARE_METATYPE(cv::Mat)
Q_DECLARE_METATYPE(std::vector<cv::Mat>)
Q_DECLARE_METATYPE(QVector<QVector4D>)

#define TEST_WAIT_TIME 2 * 60 * 1000    //压力测试预留计算时间2分钟

//#define CHECK_CHARACTER         //字符校验全部完成时，才能进行下一步工作

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
//    ui->menuBar->setEnabled(false);
    ui->mainToolBar->setEnabled(false);
    ui->statusBar->showMessage(tr("正在加载模型数据"), 5000);
    timerId = startTimer(200);

//    qDebug()<<"MainWindow:"<<QThread::currentThreadId();

    qRegisterMetaType<cv::Mat>("cv::Mat");
    qRegisterMetaType<std::vector<cv::Mat>>("std::vector<cv::Mat>");
    qRegisterMetaType<QVector<QVector4D>>("QVector<QVector4D>");
    qRegisterMetaType<std::vector<std::vector<cv::Point3d> > >("std::vector<std::vector<cv::Point3d> >");
    qRegisterMetaType<std::string>("std::string");
    qRegisterMetaType<std::vector<cv::Point3d>>("std::vector<cv::Point3d>");
    qRegisterMetaType<std::vector<std::string>>("std::vector<std::string>");
    qRegisterMetaType<std::vector<std::vector<std::string> >>("std::vector<std::vector<std::string> >");
    qRegisterMetaType<std::vector<cv::Point2f>>("std::vector<cv::Point2f>");
    qRegisterMetaType<std::vector<std::vector<int> >>("std::vector<std::vector<int> >");
    qRegisterMetaType<std::vector<std::vector<QPointF> >>("std::vector<std::vector<QPointF> >");


    //参数设置模块
    optionDlg = new OptionDialog(this);

    //图像组件
    picBoxForm = new PicBoxForm;
    QGridLayout *layout1 = new QGridLayout;
    layout1->addWidget(picBoxForm);
    ui->widget_picBox->setLayout(layout1);

    //龙门组件
    longmenForm = new LongmenForm;
    QGridLayout *layout2 = new QGridLayout;
    layout2->addWidget(longmenForm);
    ui->widget_longmen->setLayout(layout2);

    //相机控制线程
    camera_thread = new CameraThread();
    connect(optionDlg, SIGNAL(camera_init()), camera_thread, SLOT(camera_init()));
    connect(optionDlg, SIGNAL(exposure_time_changed()), camera_thread, SLOT(set_camera_exposure_time()));       //曝光时间
    connect(camera_thread, SIGNAL(camera_init_finished(QStringList)), optionDlg, SLOT(camera_init_finished(QStringList)));
    connect(camera_thread, SIGNAL(shot_finished(QImage,int)), picBoxForm, SLOT(show_image(QImage,int)));        //显示图片
    connect(camera_thread, &CameraThread::update_info, ui->textEdit, &QPlainTextEdit::appendPlainText);
    connect(this, &MainWindow::camera_reset, camera_thread, &CameraThread::camera_reset);

    //服务器通信线程
    serverctrl = new ServerCtrl;
    connect(serverctrl, &ServerCtrl::update_info, ui->textEdit, &QPlainTextEdit::appendPlainText);
    connect(serverctrl, &ServerCtrl::longmen_ready, [this]{     //设置界面自动启动(仅在线模式)
        if(PP->test_mode() == ONLINE){
            optionDlg->start_auto_test();
            if(optionDlg->isHidden()){
                on_act_Option_triggered();
            }
        }
    });
    connect(this, SIGNAL(take_a_shot(double)), camera_thread, SLOT(take_a_shot(double)));
    connect(serverctrl, &ServerCtrl::longmen_pos_changed, [this](double pos){
        if(PP->test_mode() == ONLINE){
            longmenForm->set_longmen_value(pos);
            PP->dx = pos;       //记录龙门位置
            if(PP->is_should_shot(pos)){            //判断拍照条件
                PP->shot_longmen_pos_record_append(pos);
                emit take_a_shot(pos);
                longmenForm->set_shot_pos();
                ui->textEdit->appendPlainText(tr("执行第%1次拍摄").arg(PP->shot_longmen_pos_record().count()));
            }

            if(PP->shot_longmen_pos_record().count() >= PP->camera_shot_times()
                    || qAbs(pos - PP->longmen_end_pos()) < 10){     //判断结束条件，拍摄次数达标或者龙门走完
                serverctrl->SendShutdown_OK();                      //发送拍摄完成信号
                emit shot_finished();
                ui->textEdit->appendPlainText(tr("结束拍摄!共拍摄%1次").arg(PP->shot_longmen_pos_record().count()));
//                imageProcessing->end_test();
                save_longmen_pos_modify();
                ui->widget_3d->show();
                picBoxForm->hide();
            }
        }
    });

    //虚拟服务器线程
    virtualServer = new VirtualServer(this);
    connect(virtualServer, &VirtualServer::take_a_shot, [this](double pos){
//        qDebug()<<"VirtualServer::take_a_shot"<<pos;
        longmenForm->set_longmen_value(pos);
        PP->shot_longmen_pos_record_append(pos);
        emit take_a_shot(pos);
        longmenForm->set_shot_pos();
        ui->textEdit->appendPlainText(tr("执行第%1次拍摄").arg(PP->shot_longmen_pos_record().count()));

        if(PP->shot_longmen_pos_record().count() >= PP->camera_shot_times()
                || qAbs(pos - PP->longmen_end_pos()) < 10){     //判断结束条件，拍摄次数达标或者龙门走完
            virtualServer->stop();
            ui->textEdit->appendPlainText(tr("结束拍摄!共拍摄%1次").arg(PP->shot_longmen_pos_record().count()));
            emit shot_finished();
            ui->widget_3d->show();
            picBoxForm->hide();
//            imageProcessing->end_test();
        }
    });

    //数据处理线程
    if(!PP->check_training_model()){
        QMessageBox::warning(this, tr("警告"),
                             tr("计算数据路径未正确设置.\n请在[开始检测][计算数据路径]中正确设置，之后重启程序"),
                             QMessageBox::Ok);
    }
    clipThread = new ClipThread;
    connect(camera_thread, &CameraThread::one_group_ready, clipThread, &ClipThread::add_one_group);
    connect(clipThread, &ClipThread::net_load_ready, this, &MainWindow::net_load_ready);
    hebingThread = new HebingThread;
    connect(clipThread, &ClipThread::one_group_ready, hebingThread, &HebingThread::add_one_group);
    matchThread = new MatchThread;
    connect(hebingThread, &HebingThread::one_workpiece_ready, matchThread, &MatchThread::add_one_workpiece);
    connect(matchThread, &MatchThread::send_weldData, serverctrl, &ServerCtrl::send_weldData);

    connect(this, &MainWindow::shot_finished, [this]{
        QTimer::singleShot(TEST_WAIT_TIME, this, &MainWindow::one_test_finished);
//        QTimer *timer_watch = new QTimer;
//        timer_watch->setInterval(1000);
//        connect(timer_watch, &QTimer::timeout, [this]{
//           qDebug()<<"tiemr"<<QThreadPool::globalInstance()->activeThreadCount();
//        });
//        timer_watch->start();
    });

//    imageProcessing = new ImageProcessing;
//    connect(camera_thread, &CameraThread::one_group_ready, imageProcessing, &ImageProcessing::add_one_group);
//    connect(imageProcessing, &ImageProcessing::compute_progress, longmenForm, &LongmenForm::update_compute_progress);
//    connect(imageProcessing, &ImageProcessing::update_info, ui->textEdit, &QPlainTextEdit::appendPlainText);
//    connect(imageProcessing, &ImageProcessing::send_weldData, serverctrl, &ServerCtrl::send_weldData);

    //3D显示相关
    graph = new Q3DScatter;
    container = QWidget::createWindowContainer(graph);

    if (!graph->hasContext()) {
        QMessageBox msgBox;
        msgBox.setText("Couldn't initialize the OpenGL context.");
        msgBox.exec();
    }

    container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QFont ft;
    ft.setPointSize(24);
    ui->label->setFont(ft);
    ui->widget_3d->layout()->addWidget(container);
    ui->widget_3d->hide();

//    modifier = new ScatterDataModifier(graph);
//    connect(hebingThread, &HebingThread::one_workpiece_ready, modifier, &ScatterDataModifier::add_one_workpiece);
//    connect(matchThread, &MatchThread::match_finished, modifier, &ScatterDataModifier::match_finished);
//    connect(imageProcessing, SIGNAL(show_mesh_data(QVector<QVector4D>,QStringList,QVector<int>)),
//            this, SLOT(show_mesh_data(QVector<QVector4D>,QStringList,QVector<int>)));
//    connect(imageProcessing, &ImageProcessing::start_match, modifier, &ScatterDataModifier::startMatch);
//    connect(imageProcessing, &ImageProcessing::match_success, modifier, &ScatterDataModifier::matchSuccess);
//    connect(imageProcessing, &ImageProcessing::match_failed, modifier, &ScatterDataModifier::matchFailed);
//    connect(imageProcessing, &ImageProcessing::match_pending, modifier, &ScatterDataModifier::matchPending);
//    connect(modifier, &ScatterDataModifier::set_title, ui->label, &QLabel::setText);
//    connect(modifier, &ScatterDataModifier::set_title, ui->textEdit, &QPlainTextEdit::appendPlainText);

//    charDialog = new CharDialog(this);

    scatterViewer = new ScatterViewer(graph);
    connect(hebingThread, &HebingThread::one_workpiece_ready, scatterViewer, &ScatterViewer::add_one_workpiece);
    connect(matchThread, &MatchThread::match_started, scatterViewer, &ScatterViewer::match_started);
    connect(matchThread, &MatchThread::match_finished, scatterViewer, &ScatterViewer::match_finished);

    checkDialog = new CheckDialog(scatterViewer, hebingThread);
    connect(checkDialog, &CheckDialog::prepare_to_rematch, hebingThread, &HebingThread::prepare_to_rematch);
    connect(hebingThread, &HebingThread::send_to_rematch, matchThread, &MatchThread::send_to_rematch);

#ifdef CHECK_CHARACTER
    ui->act_examine_char_complete->setEnabled(false);
#endif
    //测试----------------------------------------------;
//    ui->widget_3d->show();
//    picBoxForm->hide();
}

MainWindow::~MainWindow()
{
    save_log();
    delete ui;
}

void MainWindow::show_mesh_data(QVector<QVector4D> mesh,QStringList str_list,QVector<int> result_list)
{
//    modifier->setData1(mesh,str_list,result_list);
//    if(modifier->check_character()){
//        ui->act_examine_char_complete->setEnabled(true);
//    }
//    ui->widget_3d->show();
//    picBoxForm->hide();

//    emit one_test_finished();
}

void MainWindow::on_act_Option_triggered()
{
    if(optionDlg->exec() == QDialog::Accepted){        //测试启动
        PP->reset();
        reset_ui();
        emit camera_reset();
        clipThread->reload();
        hebingThread->reload();
        ui->widget_3d->hide();
        picBoxForm->show();

        if(PP->test_mode() == ONLINE){
            serverctrl->reset_server();
            serverctrl->SendControData_STARTOK();   //向服务器请求龙门位置
        }
        else{
            virtualServer->start();
        }
        ui->textEdit->appendPlainText(tr("开始测试!"));
        qDebug()<<"系统重置完成！";
    }
}

void MainWindow::reset_ui()
{
    picBoxForm->reset();                    //根据设置重置摄像头ui
    longmenForm->reset();
    save_log();
    ui->textEdit->clear();
    scatterViewer->reset();
    checkDialog->reload();
#ifdef CHECK_CHARACTER
    ui->act_examine_char_complete->setEnabled(false);
#endif

    QString str_time = QDateTime::currentDateTime().toString("yyyy-MM-dd-hh-mm");
    PP->setstr_time(str_time);
    QDir dir;
    dir.mkdir(PP->save_path() + "/" + str_time);
    dir.mkdir(PP->save_path() + "/" + str_time + "/yuantu");
    dir.mkdir(PP->save_path() + "/" + str_time + "/MASK");
    dir.mkdir(PP->save_path() + "/" + str_time + "/WELD");
    dir.mkdir(PP->save_path() + "/" + str_time + "/WELD_MINUS");
    dir.mkdir(PP->save_path() + "/" + str_time + "/COMBINE");
    dir.mkdir(PP->save_path() + "/" + str_time + "/MATCH");
}

void MainWindow::save_log()
{
    if(ui->textEdit->toPlainText().isEmpty())
        return;

    QFile file(PP->save_path() + "/" + PP->str_time() + QString("/Log_%1.txt").arg(QDateTime::currentDateTime().toString("yyyy-MM-dd-hh-mm")));
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        return;

    QTextStream out(&file);
    out << ui->textEdit->toPlainText();
    qDebug()<<"日志已保存"<<file.fileName();
}

void MainWindow::save_longmen_pos_modify()
{
    QString path = PP->save_path() + "/" + PP->str_time() + "/yuantu/LongmenPos.txt";
    QFile file(path);
    if(!file.open(QIODevice::Append|QIODevice::Text)){
        qDebug()<<"write LongmenPos.txt failed";
        return;
    }
    QTextStream out(&file);
    foreach (QVector3D v, PP->shot_longmen_pos_modify()) {
        out << v.x() << ","
            << v.y() << ","
            << v.z() << endl;
    }
}

//删除选中工件
void MainWindow::on_act_Del_triggered()
{
    if(ui->widget_3d->isHidden())
        return;

    scatterViewer->del_current();
//    modifier->del_current();
//    if(modifier->check_character()){
//        ui->act_examine_char_complete->setEnabled(true);
//    }
}

//校验字符
void MainWindow::on_act_Character_triggered()
{
//    checkDialog->exec();
//    checkDialog->setModal(false);
    checkDialog->show();
//    if(ui->widget_3d->isHidden())
//        return;

//    charDialog->init(modifier->current_number(),
//                     modifier->current_character(),
//                     modifier->current_result(),
//                     imageProcessing->show_pics(modifier->current_number()));
//    if(charDialog->exec() == QDialog::Accepted){
//        modifier->setCurrent_character(charDialog->character());
//        if(modifier->check_character()){
//            ui->act_examine_char_complete->setEnabled(true);
//        }
//    }
}

//完成字符校验
void MainWindow::on_act_examine_char_complete_triggered()
{
//    qDebug()<<modifier->character_list() << modifier->result_list();
//    modifier->setUnMatched();
//    imageProcessing->examine_char_complete(modifier->character_list(), modifier->result_list());
}

//发送焊缝
void MainWindow::on_act_send_triggered()
{
//    imageProcessing->P_weld_send();
}

void MainWindow::on_act_abort_triggered()
{
    AbortDialog dlg(this);
    dlg.exec();
}

void MainWindow::on_act_Test_triggered()
{
    TestDialog dlg;
    connect(this, &MainWindow::one_test_finished, &dlg, &TestDialog::one_test_finished);
    connect(&dlg, &TestDialog::start_test, [this](QString path){
       qDebug()<<"get!"<<path;
       QThread::sleep(1);
//       PP->reset();
//       reset_ui();
//       emit camera_reset();
//       if(PP->test_mode() == ONLINE){
//           serverctrl->reset_server();
//           serverctrl->SendControData_STARTOK();   //向服务器请求龙门位置
//       }
//       else{
//           virtualServer->start();
//       }
//       ui->textEdit->appendPlainText(tr("开始测试!"));
//       imageProcessing->start_test();
//       ui->widget_3d->hide();
//       picBoxForm->show();

//       emit one_test_finished();
       PP->reset();
       reset_ui();
       emit camera_reset();
       clipThread->reload();
       hebingThread->reload();
       ui->widget_3d->hide();
       picBoxForm->show();

       if(PP->test_mode() == ONLINE){
           serverctrl->reset_server();
           serverctrl->SendControData_STARTOK();   //向服务器请求龙门位置
       }
       else{
           virtualServer->start();
       }
       ui->textEdit->appendPlainText(tr("开始测试!"));
//       emit one_test_finished();
       qDebug()<<"系统重置完成！";
    });
        dlg.exec();
}

void MainWindow::net_load_ready()
{
//    ui->menuBar->setEnabled(true);
    ui->mainToolBar->setEnabled(true);
    killTimer(timerId);
    ui->statusBar->showMessage(tr("模型数据加载已完成"), 5000);
}

void MainWindow::timerEvent(QTimerEvent *e)
{
    QString msg = ui->statusBar->currentMessage();
    ui->statusBar->showMessage(msg.append("."));
}


void MainWindow::on_action_triggered()
{
    //测试
    QFile file(":/image/point4ds_txt2.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)){
        qDebug()<<"测试文件读取失败！";
        return;
    }

    std::vector<cv::Point3d> pcd;
    std::vector<cv::Point3d> pcd_mesh;
    QStringList char_list;
    char_list << "AAA" << "BBB" << "CCC" << "DDD" << "EEE" << "FFF" << "GGG";

    int workpiece_index = -1;
    while (!file.atEnd()) {
        QList<QByteArray> list = file.readLine().simplified().split(' ');
        Q_ASSERT(list.count() >= 4);

        cv::Point3d v;
        int index = qRound(list.at(3).toFloat());
        if(workpiece_index != index){
            if(pcd_mesh.size() > 0){
//                modifier->add_one_workpiece(workpiece_index, pcd, char_list.at(workpiece_index-1).toStdString(), pcd_mesh);
                scatterViewer->add_one_workpiece(workpiece_index, pcd, char_list.at(workpiece_index-1).toStdString(), pcd_mesh);
                qDebug()<<"发送！"<<workpiece_index<<pcd_mesh.size() << char_list.at(workpiece_index-1);
                pcd_mesh.clear();
            }
            workpiece_index = index;
        }

        v.x = list.at(0).toFloat();
        v.y = list.at(1).toFloat();
        v.z = list.at(2).toFloat();

        pcd_mesh.push_back(v);
    }
    if(pcd_mesh.size() > 0){
//        modifier->add_one_workpiece(workpiece_index, pcd, char_list.at(workpiece_index-1).toStdString(), pcd_mesh);
        scatterViewer->add_one_workpiece(workpiece_index, pcd, char_list.at(workpiece_index-1).toStdString(), pcd_mesh);
        qDebug()<<"发送！"<<workpiece_index<<pcd_mesh.size() << char_list.at(workpiece_index-1);
        pcd_mesh.clear();
    }

    ui->widget_3d->show();
    picBoxForm->hide();
}

void MainWindow::on_act_Register_triggered()
{
    RegisterDialog dlg(this);
    dlg.exec();
}
