#ifndef PICBOXFORM_H
#define PICBOXFORM_H

#include <QWidget>
#include <QLabel>
#include <QScrollArea>

namespace Ui {
class PicBoxForm;
}

struct Channel{
    Channel(QScrollArea *a, QLabel *p, QLabel *in, int x, int y, int c1, int c4, int c9, int c16);

    void reset(int camera_num){
        clear();
        if(camera_num <= 1){
            index = camera_vector.at(0);
        }
        else if(camera_num <= 4){
            index = camera_vector.at(1);
        }
        else if(camera_num <= 9){
            index = camera_vector.at(2);
        }
        else if(camera_num <= 16){
            index = camera_vector.at(3);
        }
        if(index <= camera_num)
            info->setText(QString::number(index));
        else{
            info->setText("");
        }
    }
    void init(QString sn, bool isOn){
        this->sn = sn;
        this->isOn = isOn;
        info->setText(QString::number(index) + "  sn:" + sn + "\t状态:" + (isOn == true ? "开启" : "未开启"));
        image->setStyleSheet("background-color: black");
    }

    void clear(){
        index = -1;
        sn = "N/A";
        image->setStyleSheet("");        
//        image->setStyleSheet("background-color: black");
    }


    void show();
    void show(int n);
    void hide();

    //不可修改
    QScrollArea *area;
    QLabel *image;
    QLabel *info;
    QPoint location;        //坐标位置
    QVector<int> camera_vector; //保存序号映射信息
    //可变
    int index;              //摄像头序号
    QString sn;             //摄像头对应的sn
    bool isOn;              //是否开启
};

/********************************************************
 * @ClassName:  PicBoxForm
 * @Brief:      照片显示矩阵类,主Ui的重要组成部分
 *              可以根据相机数,动态管理界面分布
 * @date        2020/08/31
 ********************************************************/
class PicBoxForm : public QWidget
{
    Q_OBJECT

public:
    explicit PicBoxForm(QWidget *parent = nullptr);
    ~PicBoxForm();

    void reset();

public slots:
    void show_image(QImage image,int camera_num);

private slots:
    void deal_button_clicked(int n);

private:
    Ui::PicBoxForm *ui;

    QList<Channel*> channel_list;

};

#endif // PICBOXFORM_H
