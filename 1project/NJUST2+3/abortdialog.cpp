#include "abortdialog.h"
#include "ui_abortdialog.h"

AbortDialog::AbortDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AbortDialog)
{
    ui->setupUi(this);
}

AbortDialog::~AbortDialog()
{
    delete ui;
}
