#include "chardialog.h"
#include "ui_chardialog.h"
#include "Server/parameter.h"

CharDialog::CharDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CharDialog)
{
    ui->setupUi(this);

    ui->lab_check->hide();
    ui->rbt_no->hide();
    ui->rbt_yes->hide();

    ui->lab_pic->resize(QSize(601, 413));
//    connect(this, SIGNAL())
}

CharDialog::~CharDialog()
{
    delete ui;
}

void CharDialog::init(int num, QString ch, bool f, std::vector<cv::Mat> v)
{
    ui->lab_number->setText(QString::number(num));
    ui->lineEdit_char->setText(ch);
    ui->rbt_yes->setChecked(f);
    ui->rbt_no->setChecked(!f);

    image_list.clear();
    for (uint i = 0; i < v.size(); ++i) {
        image_list.append(PP->cvMat_to_QImage(v.at(i)));
    }
    current_num = 0;

    if(!image_list.isEmpty()){
        set_pic_num(current_num);
    }
}

QString CharDialog::character() const
{
    return ui->lineEdit_char->text();
}

bool CharDialog::result() const
{
    if(ui->rbt_yes->isChecked()){
        return true;
    }
    else{
        return false;
    }
}

int CharDialog::number() const
{
    return ui->lab_number->text().toInt();
}

void CharDialog::on_pbt_pre_clicked()
{
    if(current_num > 0){
        current_num--;
        set_pic_num(current_num);
    }
}

void CharDialog::on_pbt_next_clicked()
{
    if(current_num < image_list.count()-1){
        current_num++;
        set_pic_num(current_num);
    }
}

void CharDialog::set_pic_num(int num)
{
//    this->updateGeometry();
    qDebug()<<ui->lab_pic->size();
    ui->lab_pic->setPixmap(QPixmap::fromImage(image_list.at(num).scaled(ui->lab_pic->size())));
    ui->lab_page->setText(QString::number(num+1) + "/" + QString::number(image_list.count()));
}

void CharDialog::on_pbt_revloe_clicked()
{
    QMatrix m;
    m.rotate(90);

    image_list[current_num] = image_list[current_num].transformed(m, Qt::FastTransformation);
    set_pic_num(current_num);
}
