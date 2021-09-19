#ifndef LONGMENFORM_H
#define LONGMENFORM_H

#include <QWidget>

namespace Ui {
class LongmenForm;
}

/********************************************************
 * @ClassName:  LongmenForm
 * @Brief:      龙门位置显示框架,主界面ui的一个组成部分
 *              能够实时显示龙门的位置,并显示记录拍照位置坐标,
 *              以及计算进度
 * @date        2020/08/31
 ********************************************************/
class LongmenForm : public QWidget
{
    Q_OBJECT

public:
    explicit LongmenForm(QWidget *parent = nullptr);
    ~LongmenForm();

    void reset();                           //重置龙门位置
    void set_longmen_value(double v);       //设置龙门位置
    void set_shot_pos();                    //设置拍照坐标

public slots:
    void update_compute_progress(double p); //更新计算进度

private:
    Ui::LongmenForm *ui;

    bool isInvert;      //标识龙门坐标是否递减
};

#endif // LONGMENFORM_H
