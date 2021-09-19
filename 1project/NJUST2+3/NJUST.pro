#-------------------------------------------------
#
# Project created by QtCreator 2019-11-08T10:03:59
#
#-------------------------------------------------

QT      += core gui network
QT      += datavisualization

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = NJUST
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

DEFINES += FULLY_BUILD
contains(DEFINES,FULLY_BUILD){
message('-->>(enter fully build mode! )')
}else{
message('-->>(enter simple build mode! )')
}


SOURCES += \
        Camera/basler_camera.cpp \
        Camera/camerathread.cpp \
    DataVisualization/checkdialog.cpp \
    DataVisualization/checkmodel.cpp \
        DataVisualization/scatterdatamodifier.cpp \
    DataVisualization/scatterviewer.cpp \
    DataVisualization/workpiece.cpp \
    ImageProcessing/clipneta.cpp \
    ImageProcessing/clipnetb.cpp \
    ImageProcessing/cliprun.cpp \
        ImageProcessing/clipthread.cpp \
        ImageProcessing/hebingthread.cpp \
    ImageProcessing/matchrun.cpp \
    ImageProcessing/matchthread.cpp \
    SecretKey/boardid.cpp \
    SecretKey/cpuid.cpp \
    SecretKey/diskid.cpp \
    SecretKey/macaddress.cpp \
    SecretKey/registerdialog.cpp \
    SecretKey/secretkeydialog.cpp \
    Server/parameter.cpp \
    Server/serverctrl.cpp \
    Server/virtualserver.cpp \
    abortdialog.cpp \
    chardialog.cpp \
    longmenform.cpp \
    main.cpp \
    mainwindow.cpp \
        optiondialog.cpp \
        picboxform.cpp \
        Compute/common.cpp \
        testdialog.cpp

contains(DEFINES,FULLY_BUILD){
SOURCES += \
    Compute/LunKuoPiPeiHY.cpp \
    Compute/Match.cpp \
    Compute/character.cpp \
    Compute/cjson.cpp \
    Compute/json.cpp \
    Compute/mask_detector.cpp \
    Compute/weld_detector.cpp \
}

HEADERS += \
        Camera/basler_camera.h \
        Camera/camerathread.h \
    DataVisualization/checkdialog.h \
    DataVisualization/checkmodel.h \
        DataVisualization/scatterdatamodifier.h \
    DataVisualization/scatterviewer.h \
    DataVisualization/workpiece.h \
    ImageProcessing/clipneta.h \
    ImageProcessing/clipnetb.h \
    ImageProcessing/cliprun.h \
    ImageProcessing/clipthread.h \
    ImageProcessing/hebingthread.h \
    ImageProcessing/matchrun.h \
    ImageProcessing/matchthread.h \
    SecretKey/boardid.h \
    SecretKey/cpuid.h \
    SecretKey/diskid.h \
    SecretKey/macaddress.h \
    SecretKey/registerdialog.h \
    SecretKey/secretkeydialog.h \
        Server/parameter.h \
        Server/serverctrl.h \
        Server/serverdefs.h \
        Server/virtualserver.h \
        abortdialog.h \
        chardialog.h \
        longmenform.h \
        mainwindow.h \
        optiondialog.h \
        picboxform.h \
        Compute/common.h \
        testdialog.h

contains(DEFINES,FULLY_BUILD){
HEADERS += \
    Compute/LunKuoPiPeiHY.h \
    Compute/Match.h \
    Compute/character.h \
    Compute/cjson.h \
    Compute/json.h \
    Compute/mask_detector.h \
    Compute/weld_detector.h \
}

FORMS += \
        DataVisualization/checkdialog.ui \
        SecretKey/registerdialog.ui \
        SecretKey/secretkeydialog.ui \
        abortdialog.ui \
        chardialog.ui \
        longmenform.ui \
        mainwindow.ui \
        optiondialog.ui \
        picboxform.ui \
        testdialog.ui

CONFIG += precompile_header
PRECOMPILED_HEADER = stable.h

RESOURCES += \
    resource.qrc

unix:!macx: LIBS += -L$$PWD/../../Qt5.14.0/5.14.0/gcc_64/lib/ -lQt5DataVisualization

INCLUDEPATH += $$PWD/../../Qt5.14.0/5.14.0/gcc_64/include
DEPENDPATH += $$PWD/../../Qt5.14.0/5.14.0/gcc_64/include

#pylon5
INCLUDEPATH +=  /opt/pylon5/include
QMAKE_CXXFLAGS+=$(shell /opt/pylon5/bin/pylon-config --cflags)
QMAKE_LFLAGS+=$(shell /opt/pylon5/bin/pylon-config --libs-rpath)
LIBS+=$(shell /opt/pylon5/bin/pylon-config --libs)

#opencv
INCLUDEPATH += /usr/local/include \
                /usr/local/include/opencv4 \
                /usr/local/include/opencv4/opencv2 \
                /usr/include/c++/7

LIBS += /usr/local/lib/libopencv_highgui.so    \
        /usr/local/lib/libopencv_core.so       \
        /usr/local/lib/libopencv_imgproc.so    \
        /usr/local/lib/libopencv_imgcodecs.so  \
        /usr/local/lib/libopencv_dnn.so

contains(DEFINES,FULLY_BUILD){
#pcl
INCLUDEPATH += /usr/include/eigen3

INCLUDEPATH += /usr/include/vtk-6.3
LIBS += /usr/lib/x86_64-linux-gnu/libvtk*.so

INCLUDEPATH += /usr/include/boost
LIBS += /usr/lib/x86_64-linux-gnu/libboost_*.so

INCLUDEPATH += /usr/include/pcl-1.8
LIBS += /usr/lib/x86_64-linux-gnu/libpcl_*.so

##crnn-caffe
#INCLUDEPATH += /home/fenglin/Programs/crnn-caffe/include  \
#              /home/fenglin/Programs/crnn-caffe/build/include
#LIBS += -L/home/fenglin/Programs/crnn-caffe/build/lib -lcaffe


#SSD-caffe
INCLUDEPATH += /home/jiangnan/program/caffe/include  \
              /home/jiangnan/program/caffe/build/include
LIBS += -L/home/jiangnan/program/caffe/build/lib -lcaffe

LIBS+= -L/usr/lib/x86_64-linux-gnu  -lglog -lgflags -lprotobuf -lleveldb -lsnappy -llmdb -lboost_system  -lm -lboost_thread -lstdc++  -lprotobuf  -lcblas -latlas -lcublas -lcurand
LIBS+= -L/usr/lib/x86_64-linux-gnu/libhdf5_hl_cpp.so \
         /usr/lib/x86_64-linux-gnu/libhdf5_cpp.so



#CUDA
CUDA_SDK = "/usr/local/cuda"   # Path to cuda SDK install
CUDA_DIR = "/usr/local/cuda"            # Path to cuda toolkit install

# DO NOT EDIT BEYOND THIS UNLESS YOU KNOW WHAT YOU ARE DOING....

SYSTEM_NAME = ubuntu         # Depending on your system either 'Win32', 'x64', or 'Win64'
SYSTEM_TYPE = 64           # '32' or '64', depending on your system
CUDA_ARCH = sm_50           # Type of CUDA architecture, for example 'compute_10', 'compute_11', 'sm_10'
NVCC_OPTIONS = --use_fast_math

# include paths
INCLUDEPATH += $$CUDA_DIR/include \
            /usr/local/cuda/samples/common/inc

# library directories
QMAKE_LIBDIR += $$CUDA_DIR/lib64/

CUDA_OBJECTS_DIR = ./

# Add the necessary libraries
CUDA_LIBS = -lcuda \
 -lcudart \
/usr/local/cuda-10.0/lib64/libcufft.so \

##Zbar
#INCLUDEPATH += /home/fenglin/Programs/zbar-0.10/include \
#               /home/fenglin/Programs/zbar-0.10/include/zbar  \

#LIBS += /usr/lib/x86_64-linux-gnu/libzbar.so.0 \
#        /usr/lib/x86_64-linux-gnu/libzbar.so.0.2.0

}

DISTFILES += \
    temp_info
