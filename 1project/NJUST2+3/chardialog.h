#ifndef CHARDIALOG_H
#define CHARDIALOG_H

#include <QDialog>

namespace Ui {
class CharDialog;
}

/********************************************************
 * @ClassName:  CharDialog
 * @Brief:      字符检测类,完成字符检验相关功能,
 *              后已被CheckDialog所完全替代,故废弃
 * @date        2020/08/31
 ********************************************************/
class CharDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CharDialog(QWidget *parent = nullptr);
    ~CharDialog();

    void init(int num, QString ch, bool f, std::vector<cv::Mat> v);

    QString character() const;

    bool result() const;

    int number() const;

private slots:
    void on_pbt_pre_clicked();

    void on_pbt_next_clicked();

    void on_pbt_revloe_clicked();

private:
    Ui::CharDialog *ui;
    QList<QImage> image_list;
    int current_num;        //当前序号
    void set_pic_num(int num);
};

#endif // CHARDIALOG_H
