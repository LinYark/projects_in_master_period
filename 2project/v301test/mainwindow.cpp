#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    hfc->myconnect();
    hfc->init();
    psb1 = ui->pushButton1;
    connect(psb1,&QPushButton::clicked, hfc,    &maincontrol::imQuit);


    psb2 = ui->pushButton2;
    m1->myconnect();
    connect(psb2,&QPushButton::clicked, m1,    &master::myturn);
    //m1->myturn();


    psb3 = ui->pushButton3;
//    connect(psb3,&QPushButton::clicked, hfc,    &maincontrol::delmaincontrol);
}


MainWindow::~MainWindow()
{
    delete ui;
}
