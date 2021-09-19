#ifndef SECRETKEYDIALOG_H
#define SECRETKEYDIALOG_H

#include <QDialog>

namespace Ui {
class SecretKeyDialog;
}

/********************************************************
 * @ClassName:  SecretKeyDialog
 * @Brief:      密钥对话框
 * @date        2020/08/31
 ********************************************************/
class SecretKeyDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SecretKeyDialog(QWidget *parent = nullptr);
    ~SecretKeyDialog();

    static bool key_check();        //密钥检测

private slots:
    void on_pbt_register_clicked();

    void on_pbt_giveup_clicked();

private:
    Ui::SecretKeyDialog *ui;

    static QString key_gen(QString code);
    static QString hardware_code_gen();
};

#endif // SECRETKEYDIALOG_H
