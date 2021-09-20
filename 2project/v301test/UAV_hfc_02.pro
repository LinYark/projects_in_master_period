#-------------------------------------------------
#
# Project created by QtCreator 2020-08-14T10:14:25
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = UAV_hfc_01
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

HEADERS += \
        mainwindow.h \
    common\commonheaders.h \
    realsence/realsense.h \
    common/commonopencvheaders.h\
    common/commonstucts.h\
    reserveuavparam/reserveuavparam.h \
    senduavparam/senduavparam.h \
    detection/detection.h \
    track/track.h \
    location/location.h \
    maincontrol/maincontrol.h \
    master.h \
    slaver.h

SOURCES += \
        main.cpp \
        mainwindow.cpp \
    realsence/realsense.cpp \
    reserveuavparam/reserveuavparam.cpp \
    senduavparam/senduavparam.cpp \
    detection/detection.cpp \
    track/track.cpp \
    location/location.cpp \
    maincontrol/maincontrol.cpp \
    master.cpp \
    slaver.cpp



FORMS += \
        mainwindow.ui

INCLUDEPATH += \
    D:\OpenCV3.4.2\opencv342qt\include \
    D:\OpenCV3.4.2\opencv342qt\include\opencv \
    D:\OpenCV3.4.2\opencv342qt\include\opencv2

LIBS += D:\OpenCV3.4.2\opencv342qt\lib\libopencv_*.a

RESOURCES +=

