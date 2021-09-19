#include "optiondialog.h"
#include "ui_optiondialog.h"
#include <QButtonGroup>
#include <QFileDialog>
#include "Server/parameter.h"
#include <QMessageBox>

#define COUNTDOWN_TIME 5

OptionDialog::OptionDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OptionDialog)
{
    ui->setupUi(this);

    ui_init();
    connect(ui->pbt_connect, &QPushButton::clicked, this, &OptionDialog::camera_init);
    connect(ui->pbt_cancel, &QPushButton::clicked, this, &OptionDialog::reject);

    countdown = COUNTDOWN_TIME;     //10秒倒计时
    timer = new QTimer;
    timer->setInterval(1000);
    connect(timer, &QTimer::timeout, [this]{
        countdown--;
        ui->lab_auto->setText(tr("收到龙门启动信号，剩余%1秒自动启动测试").arg(countdown));
        if(countdown == 0){
            timer->stop();
            ui->widget_auto->hide();
            save();
            this->accept();
        }
    });
    ui->widget_auto->hide();
}

OptionDialog::~OptionDialog()
{
    delete ui;
}

int OptionDialog::exec()
{
    ui->stackedWidget->setCurrentIndex(0);
    ui->pbt_next->setFocus();
    emit camera_init();     //开始自动检测相机
    check_btn_state();
    if(!timer->isActive()){
        ui->widget_auto->hide();
    }
    return QDialog::exec();
}

void OptionDialog::camera_init_finished(QStringList d_list)
{
    real_sn_list = d_list;

    qDebug()<<"OptionDialog::camera_init_finished"<< d_list;

    if(d_list.isEmpty()){
        ui->lab_camera_status->setText(tr("连接失败"));
        ui->lab_camera_status->setStyleSheet("color:red;");
    }
    else{
        ui->lab_camera_status->setText(tr("已连接"));
        ui->lab_camera_status->setStyleSheet("color:green;");
    }
    check_btn_state();
}

void OptionDialog::start_auto_test()
{
    ui->widget_auto->show();
    countdown = COUNTDOWN_TIME;
    timer->start();
}

void OptionDialog::on_pbt_previous_clicked()
{
    int n = ui->stackedWidget->currentIndex();
    if(n > 0){
        ui->stackedWidget->setCurrentIndex(n - 1);
    }
    check_btn_state();
}

void OptionDialog::on_pbt_next_clicked()
{
    int n = ui->stackedWidget->currentIndex();
    switch (n) {
    case 0:
        adjust_camera_list();
        ui->stackedWidget->setCurrentIndex(1);
        break;
    case 1:
        ui->stackedWidget->setCurrentIndex(2);
        break;
    case 2:         //保存参数,开始测量
        save();
        this->accept();
        break;
    default:
        break;
    }
    check_btn_state();
}

void OptionDialog::on_pbt_up_clicked()
{
    int id = sn_group->checkedId();
    if(id > 0){
        //文字移动
        QString str = sn_group->button(id)->text();
        sn_group->button(id)->setText(sn_group->button(id-1)->text());
        sn_group->button(id-1)->setText(str);
        sn_group->button(id-1)->setChecked(true);
        //保存的数据移动
        if(ui->rbt_offline->isChecked()){       //离线模式
            offline_camera_list.swapItemsAt(id, id-1);       //since Qt 5.13
        }
        else{                                   //在线模式
            online_camera_list.swapItemsAt(id, id-1);       //since Qt 5.13
        }
    }

    check_btn_state();
}

void OptionDialog::on_pbt_down_clicked()
{
    int id = sn_group->checkedId();
    if(id != camera_num-1 && id >= 0){
        //文字移动
        QString str = sn_group->button(id)->text();
        sn_group->button(id)->setText(sn_group->button(id+1)->text());
        sn_group->button(id+1)->setText(str);
        sn_group->button(id+1)->setChecked(true);
        //保存的数据移动
        if(ui->rbt_offline->isChecked()){       //离线模式
            offline_camera_list.swapItemsAt(id, id+1);       //since Qt 5.13
        }
        else{                                   //在线模式
            online_camera_list.swapItemsAt(id, id+1);       //since Qt 5.13
        }
    }

    check_btn_state();
}

void OptionDialog::icon_button_clicked(int i)
{
    if(ui->rbt_offline->isChecked()){       //离线模式
        offline_camera_list[i] = icon_group->button(i)->isChecked();
    }
    else{                                   //在线模式
        online_camera_list[i].isOn = icon_group->button(i)->isChecked();
    }
}

//控件初始化工作
void OptionDialog::ui_init()
{
    //控件分组
    sn_group = new QButtonGroup(this);
    sn_group->addButton(ui->pbt_SN1,0);
    sn_group->addButton(ui->pbt_SN2,1);
    sn_group->addButton(ui->pbt_SN3,2);
    sn_group->addButton(ui->pbt_SN4,3);
    sn_group->addButton(ui->pbt_SN5,4);
    sn_group->addButton(ui->pbt_SN6,5);
    sn_group->addButton(ui->pbt_SN7,6);
    sn_group->addButton(ui->pbt_SN8,7);
    sn_group->addButton(ui->pbt_SN9,8);
    sn_group->addButton(ui->pbt_SN10,9);
    sn_group->addButton(ui->pbt_SN11,10);
    sn_group->addButton(ui->pbt_SN12,11);
    sn_group->addButton(ui->pbt_SN13,12);
    sn_group->addButton(ui->pbt_SN14,13);
    sn_group->addButton(ui->pbt_SN15,14);
    sn_group->addButton(ui->pbt_SN16,15);
    sn_group->setExclusive(true);

    icon_group = new QButtonGroup(this);
    icon_group->addButton(ui->pbt_camera1,0);
    icon_group->addButton(ui->pbt_camera2,1);
    icon_group->addButton(ui->pbt_camera3,2);
    icon_group->addButton(ui->pbt_camera4,3);
    icon_group->addButton(ui->pbt_camera5,4);
    icon_group->addButton(ui->pbt_camera6,5);
    icon_group->addButton(ui->pbt_camera7,6);
    icon_group->addButton(ui->pbt_camera8,7);
    icon_group->addButton(ui->pbt_camera9,8);
    icon_group->addButton(ui->pbt_camera10,9);
    icon_group->addButton(ui->pbt_camera11,10);
    icon_group->addButton(ui->pbt_camera12,11);
    icon_group->addButton(ui->pbt_camera13,12);
    icon_group->addButton(ui->pbt_camera14,13);
    icon_group->addButton(ui->pbt_camera15,14);
    icon_group->addButton(ui->pbt_camera16,15);
    icon_group->setExclusive(false);
    connect(icon_group, SIGNAL(buttonClicked(int)), this, SLOT(icon_button_clicked(int)));

    widget_list << ui->widget_camera1 << ui->widget_camera2 << ui->widget_camera3 << ui->widget_camera4
                << ui->widget_camera5 << ui->widget_camera6 << ui->widget_camera7 << ui->widget_camera8
                << ui->widget_camera9 << ui->widget_camera10<< ui->widget_camera11<< ui->widget_camera12
                << ui->widget_camera13<< ui->widget_camera14<< ui->widget_camera15<< ui->widget_camera16;

    //设置输入限制
    ui->lineEdit_camera_shot_interval->setValidator(new QIntValidator(0.001, 1000, this));  //拍摄间隔
    ui->lineEdit_ip_Remote->setInputMask(QString("000.000.000.000"));
    ui->lineEdit_port_Remote->setValidator(new QIntValidator(1, 100000, this));
    ui->lineEdit_ip_self->setInputMask(QString("000.000.000.000"));
    ui->lineEdit_port_self->setValidator(new QIntValidator(1, 100000, this));

    //初始化
    ui->lineEdit_ip_Remote->setText(PP->remote_address());
    ui->lineEdit_port_Remote->setText(QString::number(PP->remote_port()));
    ui->lineEdit_ip_self->setText(PP->self_address());
    ui->lineEdit_port_self->setText(QString::number(PP->self_port()));
    ui->rbt_offline->setChecked(!(bool)PP->test_mode());
    ui->rbt_online->setChecked((bool)PP->test_mode());

    ui->lineEdit_offlinePath->setText(PP->offline_path());
    ui->lineEdit_savePath->setText(PP->save_path());
    ui->lineEdit_computePath->setText(PP->compute_path());
    connect(ui->rbt_offline, &QRadioButton::toggled, this, &OptionDialog::check_btn_state);
    connect(ui->rbt_online, &QRadioButton::toggled, this, &OptionDialog::check_btn_state);

    ui->lineEdit_startPos->setText(QString::number(PP->longmen_start_pos()));
    ui->lineEdit_stopPos->setText(QString::number(PP->longmen_end_pos()));
    ui->lineEdit_l_shift->setText(QString::number(PP->l_shift()));
    ui->lineEdit_camera_shot_interval->setText(QString::number(PP->camera_shot_interval()));
    ui->lineEdit_camera_shot_times->setText(QString::number(PP->camera_shot_times()));
    ui->lineEdit_camera_exposure_time->setText(QString::number(PP->camera_exposure_time()));
    ui->lineEdit_ROI_start->setText(PP->rect_to_string(PP->ROI_first()));
    ui->lineEdit_ROI_middle->setText(PP->rect_to_string(PP->ROI_middle()));
    ui->lineEdit_ROI_last->setText(PP->rect_to_string(PP->ROI_last()));
    ui->comboBox_first->setCurrentIndex((int)PP->is_first_in_edge());
    ui->comboBox_last->setCurrentIndex((int)PP->is_last_in_edge());

    offline_camera_list = PP->offline_camera_list();
    online_camera_list = PP->online_camera_list();
    ui->spinBox_camera_num->setValue(offline_camera_list.count());

}

/******************************************************************************
 * 第三步开始时预先要做处理:
 * 1.读取到的列表camera_list和现有维持的列表camera_list_adjust做匹配,
 *   camera_list中有,camera_list_adjust中没有,将此元素加入camera_list_adjust后方,
 *   camera_list中没有,camera_list_adjust中有,删除camera_list_adjust中对应元素,
 *   都有,不操作
 * ****************************************************************************/
void OptionDialog::adjust_camera_list()
{
    if(ui->rbt_offline->isChecked()){       //离线模式
        camera_num = ui->spinBox_camera_num->value();
        while (offline_camera_list.count() < camera_num) {      //保证camera_num和offline_camera_list个数相匹配
            offline_camera_list.append(true);
        }
        while (offline_camera_list.count() > camera_num){
            offline_camera_list.removeLast();
        }
    }
    else{                                   //在线模式
        camera_num = real_sn_list.count();

        for (int i = online_camera_list.count()-1; i >= 0 ; i--) {
            if(!real_sn_list.contains(online_camera_list.at(i).sn)){    //real_sn_list中没有,online_camera_list中有
                online_camera_list.removeAt(i);
            }
        }

        for (int i = 0; i < real_sn_list.count(); ++i) {
            bool f = false;
            for (int j = 0; j < online_camera_list.count(); ++j) {
                if(online_camera_list.at(j).sn == real_sn_list.at(i)){
                    f = true;
                    break;
                }
            }

            if(!f){                  //real_sn_list中有,online_camera_list中没有
                CameraStatus c_s;
                c_s.sn = real_sn_list.at(i);
                c_s.isOn = true;
                online_camera_list.append(c_s);
            }
        }
        while (online_camera_list.count() > widget_list.count()) {      //实际相机数大于16,则只显示16个
            online_camera_list.removeLast();
        }
    }
    ui->lab_camera_num->setText(QString::number(camera_num));

}

void OptionDialog::save()
{
    PP->setRemote_address(ui->lineEdit_ip_Remote->text());
    PP->setRemote_port(ui->lineEdit_port_Remote->text().toInt());
    PP->setSelf_address(ui->lineEdit_ip_self->text());
    PP->setSelf_port(ui->lineEdit_port_self->text().toInt());
    PP->setTest_mode((int)ui->rbt_online->isChecked());
    PP->setOffline_path(ui->lineEdit_offlinePath->text());
    PP->setSave_path(ui->lineEdit_savePath->text());
    PP->setCompute_path(ui->lineEdit_computePath->text());
    PP->setLongmen_start_pos(ui->lineEdit_startPos->text().toInt());
    PP->setLongmen_end_pos(ui->lineEdit_stopPos->text().toInt());
    PP->setL_shift(ui->lineEdit_l_shift->text().toDouble());
    PP->setCamera_shot_interval(ui->lineEdit_camera_shot_interval->text().toInt());
    PP->setCamera_shot_times(ui->lineEdit_camera_shot_times->text().toInt());
    if(ui->lineEdit_camera_exposure_time->text().toInt() != PP->camera_exposure_time()){
        PP->setCamera_exposure_time(ui->lineEdit_camera_exposure_time->text().toInt());
        emit exposure_time_changed();       //发送相机曝光时间变化信号
    }
    PP->setROI_first(PP->string_to_rect(ui->lineEdit_ROI_start->text()));
    PP->setROI_middle(PP->string_to_rect(ui->lineEdit_ROI_middle->text()));
    PP->setROI_last(PP->string_to_rect(ui->lineEdit_ROI_last->text()));
    PP->setIs_first_in_edge((bool)ui->comboBox_first->currentIndex());
    PP->setIs_last_in_edge((bool)ui->comboBox_last->currentIndex());
    PP->setOffline_camera_list(offline_camera_list);
    PP->setOnline_camera_list(online_camera_list);

    PP->save();
}

//监控控件状态变化
void OptionDialog::check_btn_state()
{
    int n = ui->stackedWidget->currentIndex();
    ui->lab_arrow0->hide();
    ui->lab_arrow1->hide();
    ui->lab_arrow2->hide();
    ui->lab_list1->setEnabled(false);
    ui->lab_list2->setEnabled(false);
    switch (n) {
    case 0:
        ui->pbt_next->setText(tr("下一页"));
        ui->pbt_previous->hide();
        ui->lab_arrow0->show();
        if(ui->rbt_offline->isChecked() || ui->lab_camera_status->text().contains(tr("已连接"))){
            ui->pbt_next->setEnabled(true);
        }
        else{
            ui->pbt_next->setEnabled(false);
        }
        if(ui->rbt_offline->isChecked()){
            ui->widget_offline->show();
            ui->widget_online->hide();
        }
        else{
            ui->widget_offline->hide();
            ui->widget_online->show();
        }
        break;
    case 1:
        ui->pbt_next->setText(tr("下一页"));
        ui->pbt_previous->show();
        ui->lab_arrow1->show();
        ui->lab_list1->setEnabled(true);
        break;
    case 2:
        ui->pbt_next->setText(tr("开始测试"));
        ui->pbt_previous->show();
        ui->lab_arrow2->show();
        ui->lab_list1->setEnabled(true);
        ui->lab_list2->setEnabled(true);
        break;
    default:
        break;
    }

    //设置第三页
    if(ui->rbt_offline->isChecked()){       //离线模式
        for (int i = 0; i < widget_list.count(); ++i) {
            if(i < offline_camera_list.count()){
                widget_list.at(i)->setEnabled(true);
                sn_group->button(i)->setText("-");
                icon_group->button(i)->setChecked(offline_camera_list.at(i));
            }
            else{
                widget_list.at(i)->setEnabled(false);
                sn_group->button(i)->setText("N/A      ");
                icon_group->button(i)->setChecked(false);
            }
        }
    }
    else{                                   //在线模式
        for (int i = 0; i < widget_list.count(); ++i) {
            if(i < online_camera_list.count()){
                widget_list.at(i)->setEnabled(true);
                sn_group->button(i)->setText(online_camera_list.at(i).sn);
                icon_group->button(i)->setChecked(online_camera_list.at(i).isOn);
            }
            else{
                widget_list.at(i)->setEnabled(false);
                sn_group->button(i)->setText("N/A      ");
                icon_group->button(i)->setChecked(false);
            }
        }
    }
}

void OptionDialog::on_pbt_cancel_auto_clicked()
{
    ui->lab_auto->setText(tr("已取消自动测试！"));
    timer->stop();
}

void OptionDialog::on_tbt_datasaved_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                    ui->lineEdit_savePath->text(),
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);
    if(!dir.isEmpty())
        ui->lineEdit_savePath->setText(dir);
}

void OptionDialog::on_tbt_data_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                    ui->lineEdit_offlinePath->text(),
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);
    if(!dir.isEmpty())
        ui->lineEdit_offlinePath->setText(dir);
}

void OptionDialog::on_pbt_save_clicked()
{
    this->save();
//    ui->lab_auto->setText(tr("保存成功!"));
}

void OptionDialog::on_tbt_computePath_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                    ui->lineEdit_computePath->text(),
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);
    if(!dir.isEmpty())
        ui->lineEdit_computePath->setText(dir);
}

void OptionDialog::on_lineEdit_camera_shot_times_editingFinished()
{
    double start = ui->lineEdit_startPos->text().toDouble();
    double stop = ui->lineEdit_stopPos->text().toDouble();
    double interval = ui->lineEdit_camera_shot_interval->text().toDouble();

    double len = qAbs(start - stop);
    int max_times = len / interval;
    if(ui->lineEdit_camera_shot_times->text().toInt() > max_times){
        ui->lineEdit_camera_shot_times->setText(QString::number(max_times));
        QMessageBox::warning(this, tr(""),
                                         tr("拍摄次数超过龙门限定值!\n"
                                            "已重设为最大值"),
                                         QMessageBox::Ok,
                                         QMessageBox::Ok);
    }
}
