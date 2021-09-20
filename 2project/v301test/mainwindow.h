#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include<QPushButton>
#include"maincontrol/maincontrol.h"

#include"master.h"
#include"slaver.h"
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();


    QPushButton *psb1 ;
    maincontrol *hfc = new maincontrol;

    QPushButton *psb2 ;
    master *m1= new master;

    QPushButton *psb3 ;

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
