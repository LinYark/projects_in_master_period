#include "mainwindow.h"
#include <QApplication>
#include<QDebug>
#include<QDir>

#include <iostream>
#include<string>

//#include <opencv2/core/core.hpp> //添加的第一句
//#include <opencv2/highgui/highgui.hpp> // 添加的第二句

#include"maincontrol/maincontrol.h"

using namespace  std ;

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();


    return a.exec();
}
