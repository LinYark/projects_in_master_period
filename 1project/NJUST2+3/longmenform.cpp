#include "longmenform.h"
#include "ui_longmenform.h"
#include "Server/parameter.h"

LongmenForm::LongmenForm(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::LongmenForm)
{
    ui->setupUi(this);

    reset();
}

LongmenForm::~LongmenForm()
{
    delete ui;
}

void LongmenForm::reset()
{
    ui->horizontalSlider->setMinimum(0);
    ui->horizontalSlider->setMaximum(qAbs(PP->longmen_end_pos()-PP->longmen_start_pos()));
    ui->lab_longmen_start->setText(QString("%1mm").arg(PP->longmen_start_pos()));
    ui->lab_longmen_end->setText(QString("%1mm").arg(PP->longmen_end_pos()));
    ui->horizontalSlider->setValue(ui->horizontalSlider->minimum());
    ui->progressBar->setValue(0);
}

void LongmenForm::set_longmen_value(double v)
{
    ui->horizontalSlider->setValue(qAbs(v-PP->longmen_start_pos()));
}

void LongmenForm::set_shot_pos()
{
    QStringList str;
    foreach (double pos, PP->shot_longmen_pos_record()) {
        str.append(QString::number(pos));
    }

    ui->lineEdit_shot_pos->setText(str.join(","));
}

void LongmenForm::update_compute_progress(double p)
{
    ui->progressBar->setValue(p);
}












