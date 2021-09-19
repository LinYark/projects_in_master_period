#include "registerdialog.h"
#include "ui_registerdialog.h"
#include "Server/parameter.h"

RegisterDialog::RegisterDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::RegisterDialog)
{
    ui->setupUi(this);

    ui->label_time->setText(PP->key_code_time().toString());
}

RegisterDialog::~RegisterDialog()
{
    delete ui;
}
