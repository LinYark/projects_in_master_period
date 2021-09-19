#include "testdialog.h"
#include "ui_testdialog.h"
#include <QFileDialog>
#include <QFileSystemModel>
#include "Server/parameter.h"

TestDialog::TestDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TestDialog)
{
    ui->setupUi(this);

    test_row = -1;
    offline_path = PP->offline_path();      //暂存
    fresh_list();
}

TestDialog::~TestDialog()
{
    PP->setOffline_path(offline_path);      //恢复
    delete ui;
}

void TestDialog::one_test_finished()
{
    test_row++;
    if(test_row < ui->listWidget->count()){
        ui->listWidget->setCurrentRow(test_row);
        ui->listWidget->currentItem()->setBackground(Qt::green);
        QString str = PP->test_dir() + ui->listWidget->item(test_row)->text() + "/yuantu/";
//        qDebug()<<str;
        PP->setOffline_path(str);
        emit start_test(str);
    }
}

void TestDialog::on_toolButton_clicked()
{
    QString str = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                    PP->test_dir(),
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);
    if(str.isEmpty())
        return;

    str.append("/");

    PP->setTest_dir(str);
    PP->save();

    fresh_list();

}

void TestDialog::fresh_list()
{
    ui->lineEdit->setText(PP->test_dir());

    QDir dir(PP->test_dir());
    ui->listWidget->clear();
    ui->listWidget->addItems(dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot));

}

void TestDialog::on_pbt_start_clicked()
{
    qDebug()<<"start!";

    test_row = -1;

    one_test_finished();

}
