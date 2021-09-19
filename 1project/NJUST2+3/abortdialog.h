#ifndef ABORTDIALOG_H
#define ABORTDIALOG_H

#include <QDialog>

namespace Ui {
class AbortDialog;
}

/********************************************************
 * @ClassName:  AbortDialog
 * @Brief:      [关于]对话框,显示系统制造商等信息
 * @date        2020/08/31
 ********************************************************/
class AbortDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AbortDialog(QWidget *parent = nullptr);
    ~AbortDialog();

private:
    Ui::AbortDialog *ui;
};

#endif // ABORTDIALOG_H
