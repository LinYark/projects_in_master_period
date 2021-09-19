#include "checkdialog.h"
#include "ui_checkdialog.h"
#include "scatterviewer.h"
#include "workpiece.h"
#include "ImageProcessing/hebingthread.h"

CheckDialog::CheckDialog(ScatterViewer *mod, HebingThread *hebing, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::CheckDialog)
{
    ui->setupUi(this);
    scatterViewer = mod;
    hebingThread = hebing;

    connect(scatterViewer, &ScatterViewer::data_added, [this](int index){
        if(scatterViewer->workpiece_map().contains(index)){
            WorkPiece *p = scatterViewer->workpiece_map().value(index);
            int row = ui->tableWidget->rowCount();
            ui->tableWidget->insertRow(row);
            ui->tableWidget->setItem(row, 0, new QTableWidgetItem(QString::number(p->index())));
            ui->tableWidget->setItem(row, 1, new QTableWidgetItem(p->isCharSuccess() ? QString("√") : QString("×")));
            ui->tableWidget->setItem(row, 2, new QTableWidgetItem(p->isMatchSuccess() ? QString("√") : QString("×")));
            ui->tableWidget->setItem(row, 3, new QTableWidgetItem(p->isDel() ? QString("√") : QString("×")));
        }

    });

    connect(scatterViewer, &ScatterViewer::data_changed, [this](int index){
        if(scatterViewer->workpiece_map().contains(index)){
            WorkPiece *p = scatterViewer->workpiece_map().value(index);
            for (int i = 0; i < ui->tableWidget->rowCount(); ++i) {
                if(ui->tableWidget->item(i, 0)->text().toInt() == index){
                    ui->tableWidget->item(i, 1)->setText(p->isCharSuccess() ? QString("√") : QString("×"));
                    ui->tableWidget->item(i, 2)->setText(p->isMatchSuccess() ? QString("√") : QString("×"));
                    ui->tableWidget->item(i, 3)->setText(p->isDel() ? QString("√") : QString("×"));
                    break;
                }
            }
        }
    });

    connect(scatterViewer, &ScatterViewer::selectedSeriesChanged, [this](int index){
        if(scatterViewer->workpiece_map().contains(index)){
            for (int i = 0; i < ui->tableWidget->rowCount(); ++i) {
                if(ui->tableWidget->item(i, 0)->text().toInt() == index){
                    ui->tableWidget->selectRow(i);
                    break;
                }
            }
        }
        else{
            ui->tableWidget->clearSelection();
        }
    });
    ui->tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);//平均分配列宽


}

CheckDialog::~CheckDialog()
{
    delete ui;
}

/*
 * @Brief:  重新加载数据
 * @Return: NULL
 */
void CheckDialog::reload()
{
    image_map.clear();
    current_num = 0;
    ui->tableWidget->clearContents();
    ui->tableWidget->setRowCount(0);

    ui->checkBox_del->setChecked(false);
    ui->checkBox_showDel->setChecked(true);
}

/*
 * @Brief:  相应[显示删除]按钮
 * @Return: NULL
 */
void CheckDialog::on_checkBox_showDel_stateChanged(int arg1)
{
    if(arg1){
        ui->tableWidget->hideColumn(3);
        for (int i = 0; i < ui->tableWidget->rowCount(); ++i) {
            if(ui->tableWidget->item(i, 3)->text() == "√"){
                ui->tableWidget->hideRow(i);
            }
        }
    }
    else{
        ui->tableWidget->showColumn(3);
        for (int i = 0; i < ui->tableWidget->rowCount(); ++i) {
            if(ui->tableWidget->item(i, 3)->text() == "√"){
                ui->tableWidget->showRow(i);
            }
        }
    }
}

/*
 * @Brief:  响应表格选择状态变化,对话框左侧状态文字和候选图形需要相应改变,
 *          并通知3D显示改变选择状态
 * @Return: NULL
 */
void CheckDialog::on_tableWidget_itemSelectionChanged()
{
    //改变ui文字信息
    auto w_p = currentWorkpiece();
    ui->lab_number->setText(QString::number(w_p->index()));
    ui->lineEdit_char->setText(w_p->character());
    ui->checkBox_del->setChecked(w_p->isDel());

    //改变3d选择
    scatterViewer->select_workpiece(w_p->index());

    //重新加载候选图片列表
    QList<QImage> image_list;
//    qDebug()<<"SSSS" << hebingThread <<w_p->index();
    if(!image_map.contains(w_p->index())){
        auto v = hebingThread->show_pics(w_p->index());
//        qDebug()<<"show_pics size:"<<v.size();
        for (uint i = 0; i < v.size(); ++i) {
            image_list.append(PP->cvMat_to_QImage(v.at(i)));
        }
        image_map.insert(w_p->index(), image_list);
    }

    current_num = 0;
    set_pic_num(current_num);
}

WorkPiece *CheckDialog::currentWorkpiece()
{
    int row = ui->tableWidget->currentRow();
    int index = ui->tableWidget->item(row, 0)->text().toInt();
    return scatterViewer->workpiece_map().value(index);
}

/*
 * @Brief:  字符修改完成时,通知数据结构产生对应变化
 * @Param:  arg1,修改后的文字
 * @Return: NULL
 */
void CheckDialog::on_lineEdit_char_textChanged(const QString &arg1)
{
    auto w_p = currentWorkpiece();
    if(arg1 != w_p->character()){
        w_p->setCharacter(arg1);
        w_p->update();
    }
}

/*
 * @Brief:  删除复选框状态发生变化时,通知数据结构产生对应变化
 * @Param:  arg1,复选框是否被选中
 * @Return: NULL
 */
void CheckDialog::on_checkBox_del_stateChanged(int arg1)
{
    auto w_p = currentWorkpiece();
    if(arg1 != w_p->isDel()){
        w_p->setIsDel(arg1);
        w_p->update();
    }
}

/*
 * @Brief:  响应候选图片[旋转]
 * @Return: NULL
 */
void CheckDialog::on_pbt_revloe_clicked()
{
    QMatrix m;
    m.rotate(90);

    if(image_map.contains(currentWorkpiece()->index())){
        auto image_list = image_map.value(currentWorkpiece()->index());
        if(image_list.count() > current_num){
            image_list[current_num] = image_list[current_num].transformed(m, Qt::FastTransformation);
            image_map.insert(currentWorkpiece()->index(), image_list);
            set_pic_num(current_num);
        }
    }
}

/*
 * @Brief:  响应候选图片[前一张]
 * @Return: NULL
 */
void CheckDialog::on_pbt_pre_clicked()
{
    if(current_num > 0){
        current_num--;
    }
    else{
        current_num = image_map.value(currentWorkpiece()->index()).count()-1;
    }
    set_pic_num(current_num);
}

/*
 * @Brief:  响应候选图片[后一张]
 * @Return: NULL
 */
void CheckDialog::on_pbt_next_clicked()
{
    if(current_num < image_map.value(currentWorkpiece()->index()).count()-1){
        current_num++;
    }
    else{
        current_num = 0;
    }
    set_pic_num(current_num);
}

/*
 * @Brief:  响应按钮[重新匹配]
 * @Return: NULL
 */
void CheckDialog::on_pbt_rematch_clicked()
{
    scatterViewer->clear_select();

    std::vector<int> index_list;
    std::vector<std::string> char_list;
    foreach (WorkPiece * w_p, scatterViewer->workpiece_map()) {
        if(w_p->isCharSuccess() && !w_p->isMatchSuccess()){
            index_list.push_back(w_p->index());
            char_list.push_back(w_p->character().toStdString());
        }
    }
   qDebug()<<"重新匹配："<<index_list;

    emit prepare_to_rematch(index_list, char_list);
}

void CheckDialog::set_pic_num(int num)
{
//    qDebug()<<ui->lab_pic->size();
    auto list = image_map.value(currentWorkpiece()->index());
    if(!list.isEmpty()){
        ui->lab_pic->setPixmap(QPixmap::fromImage(image_map.value(currentWorkpiece()->index()).at(num).scaled(ui->lab_pic->size())));
        ui->lab_page->setText(QString::number(num+1) + "/" + QString::number(image_map.value(currentWorkpiece()->index()).count()));
    }
    else{
        ui->lab_pic->setPixmap(QPixmap());
        ui->lab_page->setText("0");
    }
}
