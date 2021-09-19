#include "picboxform.h"
#include "ui_picboxform.h"
#include <QButtonGroup>
#include "Server/parameter.h"
#include "Server/parameter.h"

PicBoxForm::PicBoxForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PicBoxForm)
{
    ui->setupUi(this);

    QButtonGroup *btn_group = new QButtonGroup;
    btn_group->addButton(ui->pbt_show1, 1);
    btn_group->addButton(ui->pbt_show4, 2);
    btn_group->addButton(ui->pbt_show9, 3);
    btn_group->addButton(ui->pbt_show16, 4);
    connect(btn_group, SIGNAL(buttonClicked(int)), this, SLOT(deal_button_clicked(int)));
    ui->pbt_show1->hide();
    ui->pbt_show4->hide();
    ui->pbt_show9->hide();
    ui->pbt_show16->hide();

    channel_list.append(new Channel(ui->scrollArea_1_1, ui->lab_image_1_1, ui->lab_info_1_1, 1, 1,  1,  1,  1,  1));
    channel_list.append(new Channel(ui->scrollArea_1_2, ui->lab_image_1_2, ui->lab_info_1_2, 1, 2, -1,  2,  2,  2));
    channel_list.append(new Channel(ui->scrollArea_1_3, ui->lab_image_1_3, ui->lab_info_1_3, 1, 3, -1, -1,  3,  3));
    channel_list.append(new Channel(ui->scrollArea_1_4, ui->lab_image_1_4, ui->lab_info_1_4, 1, 4, -1, -1, -1,  4));

    channel_list.append(new Channel(ui->scrollArea_2_1, ui->lab_image_2_1, ui->lab_info_2_1, 2, 1, -1,  3,  4,  5));
    channel_list.append(new Channel(ui->scrollArea_2_2, ui->lab_image_2_2, ui->lab_info_2_2, 2, 2, -1,  4,  5,  6));
    channel_list.append(new Channel(ui->scrollArea_2_3, ui->lab_image_2_3, ui->lab_info_2_3, 2, 3, -1, -1,  6,  7));
    channel_list.append(new Channel(ui->scrollArea_2_4, ui->lab_image_2_4, ui->lab_info_2_4, 2, 4, -1, -1, -1,  8));

    channel_list.append(new Channel(ui->scrollArea_3_1, ui->lab_image_3_1, ui->lab_info_3_1, 3, 1, -1, -1,  7,  9));
    channel_list.append(new Channel(ui->scrollArea_3_2, ui->lab_image_3_2, ui->lab_info_3_2, 3, 2, -1, -1,  8, 10));
    channel_list.append(new Channel(ui->scrollArea_3_3, ui->lab_image_3_3, ui->lab_info_3_3, 3, 3, -1, -1,  9, 11));
    channel_list.append(new Channel(ui->scrollArea_3_4, ui->lab_image_3_4, ui->lab_info_3_4, 3, 4, -1, -1, -1, 12));

    channel_list.append(new Channel(ui->scrollArea_4_1, ui->lab_image_4_1, ui->lab_info_4_1, 4, 1, -1, -1, -1, 13));
    channel_list.append(new Channel(ui->scrollArea_4_2, ui->lab_image_4_2, ui->lab_info_4_2, 4, 2, -1, -1, -1, 14));
    channel_list.append(new Channel(ui->scrollArea_4_3, ui->lab_image_4_3, ui->lab_info_4_3, 4, 3, -1, -1, -1, 15));
    channel_list.append(new Channel(ui->scrollArea_4_4, ui->lab_image_4_4, ui->lab_info_4_4, 4, 4, -1, -1, -1, 16));

    reset();

}

PicBoxForm::~PicBoxForm()
{
    delete ui;
}

void PicBoxForm::reset()
{
    int n = PP->camera_list().count();
    //改变主界面图像数量
    if(n <= 1){
        ui->pbt_show1->setChecked(true);
        deal_button_clicked(1);
    }
    else if(n <= 4){
        ui->pbt_show4->setChecked(true);
        deal_button_clicked(2);
    }
    else if(n <= 9){
        ui->pbt_show9->setChecked(true);
        deal_button_clicked(3);
    }
    else if(n <= 16){
        ui->pbt_show16->setChecked(true);
        deal_button_clicked(4);
    }

    //初始化各通道
    foreach (Channel *c, channel_list) {
        c->reset(n);
        if(c->index - 1 < n && c->index - 1 >= 0){
            CameraStatus c_s = PP->camera_list().at(c->index - 1);
            c->init(c_s.sn, c_s.isOn);
        }
    }
}

void PicBoxForm::show_image(QImage image, int camera_num)
{
//    qDebug()<<"get camera_num"<<camera_num;
    for (int i = 0; i < channel_list.count(); ++i) {
        if(channel_list.at(i)->index == camera_num + 1){
//            qDebug()<<"index"<<i<<"\t"<<channel_list.at(i)->index<<"\tcamera_num"<<camera_num;
//            channel_list.at(i)->image->setPixmap(QPixmap::fromImage(image).scaled(channel_list.at(0)->image->size()));
            channel_list.at(i)->image->setPixmap(QPixmap::fromImage(image).scaled(channel_list.at(0)->area->size(),Qt::KeepAspectRatio,
                                                                                  Qt::SmoothTransformation));
//            channel_list.at(i)->image->setPixmap(QPixmap::fromImage(image).scaled(800,600,Qt::KeepAspectRatio,
//                                                                                  Qt::SmoothTransformation));
            break;
        }
    }
//    channel_list.at(camera_num)->image->setPixmap(QPixmap::fromImage(image).scaled(channel_list.at(camera_num)->image->size()));
}

//处理按键事件
void PicBoxForm::deal_button_clicked(int n)
{
    foreach (Channel *c, channel_list) {
        c->show(n);
    }
}

Channel::Channel(QScrollArea *a, QLabel *p, QLabel *in, int x, int y, int c1, int c4, int c9, int c16){
    area = a;
    image = p;
    image->setScaledContents(true);
    info = in;
    sn = "N/A";
    location = QPoint(x,y);
    index = -1;
    isOn = false;
    camera_vector << c1 << c4 << c9 << c16;

//    image->setStyleSheet("background-color: black");
}

void Channel::show(){
    area->show();
    image->show();
    info->show();
}

void Channel::show(int n){
    if(location.x() <= n && location.y() <= n)
        show();
    else
        hide();
}

void Channel::hide(){
    area->hide();
    image->hide();
    info->hide();
}
