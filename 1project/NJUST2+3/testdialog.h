#ifndef TESTDIALOG_H
#define TESTDIALOG_H

#include <QDialog>

namespace Ui {
class TestDialog;
}

/********************************************************
 * @ClassName:  TestDialog
 * @Brief:      测试类,完成各种测试功能,正式版本中屏蔽
 * @date        2020/08/31
 ********************************************************/
class TestDialog : public QDialog
{
    Q_OBJECT

public:
    explicit TestDialog(QWidget *parent = nullptr);
    ~TestDialog();

public slots:
    void one_test_finished();

signals:
    void start_test(QString);

private slots:
    void on_toolButton_clicked();

    void on_pbt_start_clicked();

private:
    Ui::TestDialog *ui;

    void fresh_list();

    int test_row;


    QString offline_path;
};

#endif // TESTDIALOG_H
