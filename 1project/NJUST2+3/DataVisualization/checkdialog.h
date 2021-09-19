#ifndef CHECKDIALOG_H
#define CHECKDIALOG_H

#include <QDialog>

namespace Ui {
class CheckDialog;
}

class ScatterViewer;
class WorkPiece;
class HebingThread;

/********************************************************
 * @ClassName:  CheckDialog
 * @Brief:      检测清单对话框,完成了检测数据的人机交互功能
 *              表单数据和3D显示的图形数据能够做到一一对应,
 *              且能自由双向修改
 * @date        2020/08/31
 ********************************************************/
class CheckDialog : public QDialog
{
    Q_OBJECT

public:
    explicit CheckDialog(ScatterViewer *mod, HebingThread *hebing, QWidget *parent = nullptr);

    ~CheckDialog();

    void reload();          //重新加载数据

public slots:

signals:
    void prepare_to_rematch(std::vector<int> index_list, std::vector<std::string> char_list);


private slots:
    void on_checkBox_showDel_stateChanged(int arg1);

    void on_tableWidget_itemSelectionChanged();

    void on_lineEdit_char_textChanged(const QString &arg1);

    void on_checkBox_del_stateChanged(int arg1);

    void on_pbt_revloe_clicked();

    void on_pbt_pre_clicked();

    void on_pbt_next_clicked();

    void on_pbt_rematch_clicked();

private:
    Ui::CheckDialog *ui;

    ScatterViewer *scatterViewer;
    HebingThread *hebingThread;

    WorkPiece *currentWorkpiece();

    QMap<int, QList<QImage> > image_map;
    int current_num;        //当前序号
    void set_pic_num(int num);

};

#endif // CHECKDIALOG_H
