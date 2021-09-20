QT -= gui

CONFIG += c++11 console
CONFIG -= app_bundle

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += main.cpp \
    pipeline/pipelinedata.cpp \
    pipeline/pipelinectl.cpp \
    PartRealSense/realsenseh.cpp \
    PartRealSense/realsensectl.cpp \
    encode/lib/video_encode_main.cpp \
    encode/lib/video_encode_csvparser.cpp \
    encode/lib/NvVideoEncoder.cpp \
    encode/lib/NvV4l2ElementPlane.cpp \
    encode/lib/NvV4l2Element.cpp \
    encode/lib/NvUtils.cpp \
    encode/lib/NvLogging.cpp \
    encode/lib/NvElementProfiler.cpp \
    encode/lib/NvElement.cpp \
    encode/lib/NvBuffer.cpp \
    encode/encodectl.cpp \
    common.cpp \
    Detect/detect.cpp \
    LocTrack/track/fhog.cpp \
    LocTrack/track/kcftracker.cpp \
    LocTrack/LocTrack.cpp \
    LocTrack/tracklogic.cpp \
    Detect/Yolo3Detection.cpp

HEADERS += \
    pipeline/pipelinedata.h \
    pipeline/pipelinectl.h \
    PartRealSense/realsenseh.h \
    PartRealSense/realsensectl.h \
    encode/lib/video_encode.h \
    encode/lib/v4l2_nv_extensions.h \
    encode/lib/NvVideoEncoder.h \
    encode/lib/NvV4l2ElementPlane.h \
    encode/lib/NvV4l2Element.h \
    encode/lib/NvUtils.h \
    encode/lib/NvLogging.h \
    encode/lib/NvElementProfiler.h \
    encode/lib/NvElement.h \
    encode/lib/NvBuffer.h \
    encode/lib/nvbuf_utils.h \
    encode/lib/libv4l2.h \
    encode/encodectl.h \
    common.h \
    Detect/pluginsRT/YoloRT.h \
    Detect/pluginsRT/UpsampleRT.h \
    Detect/pluginsRT/ShortcutRT.h \
    Detect/pluginsRT/RouteRT.h \
    Detect/pluginsRT/ResizeLayerRT.h \
    Detect/pluginsRT/ReshapeRT.h \
    Detect/pluginsRT/ReorgRT.h \
    Detect/pluginsRT/RegionRT.h \
    Detect/pluginsRT/MaxPoolingFixedSizeRT.h \
    Detect/pluginsRT/FlattenConcatRT.h \
    Detect/pluginsRT/DeformableConvRT.h \
    Detect/pluginsRT/ActivationReLUCeilingRT.h \
    Detect/pluginsRT/ActivationMishRT.h \
    Detect/pluginsRT/ActivationLeakyRT.h \
    Detect/Yolo3Detection.h \
    Detect/utils.h \
    Detect/tkdnn.h \
    Detect/NetworkRT.h \
    Detect/Network.h \
    Detect/Layer.h \
    Detect/kernels.h \
    Detect/Int8Calibrator.h \
    Detect/Int8BatchStream.h \
    Detect/DetectionNN.h \
    Detect/detect.h \
    LocTrack/track/ffttools.h \
    LocTrack/track/fhog.h \
    LocTrack/track/kcftracker.h \
    LocTrack/track/labdata.hpp \
    LocTrack/track/recttools.h \
    LocTrack/track/targetver.h \
    LocTrack/track/tracker.h \
    LocTrack/LocTrack.h \
    LocTrack/tracklogic.h

INCLUDEPATH += /usr/include/opencv4 \
               /usr/include/opencv4/opencv2 \
               /usr/local/include/librealsense2

LIBS += /usr/lib/aarch64-linux-gnu/libopencv_core.so   \
        /usr/lib/aarch64-linux-gnu/libopencv_highgui.so \
        /usr/lib/aarch64-linux-gnu/libopencv_imgproc.so\
        /usr/lib/aarch64-linux-gnu/libopencv_imgcodecs.so\
        /usr/lib/aarch64-linux-gnu/libopencv_dnn.so \
        /usr/lib/aarch64-linux-gnu/libopencv_videoio.so \
        /home/nvidia/librealsense/build/librealsense2.so

LIBS += -L/usr/lib/aarch64-linux-gnu -lv4l2

LIBS += /usr/lib/libtkDNN.so \
        /usr/lib/libkernels.so \
        /usr/lib/aarch64-linux-gnu/libcublas.so \
        /usr/lib/aarch64-linux-gnu/libcudnn.so  \
        /usr/lib/aarch64-linux-gnu/libnvinfer.so

LIBS += -L/usr/lib/aarch64-linux-gnu/tegra -lnvbuf_utils

CUDA_SDK = "/usr/local/cuda-10.0"   # Path to cuda SDK install
CUDA_DIR = "/usr/local/cuda-10.0"            # Path to cuda toolkit install

# DO NOT EDIT BEYOND THIS UNLESS YOU KNOW WHAT YOU ARE DOING....

SYSTEM_NAME = unix         # Depending on your system either 'Win32', 'x64', or 'Win64'
SYSTEM_TYPE = 64           # '32' or '64', depending on your system
CUDA_ARCH = compute_62          # Type of CUDA architecture, for example 'compute_10', 'compute_11', 'sm_10'
NVCC_OPTIONS = --use_fast_math


# include paths
INCLUDEPATH += $$CUDA_DIR/include

# library directories
QMAKE_LIBDIR += $$CUDA_DIR/lib64/

CUDA_OBJECTS_DIR = ./

# CUDA settings <-- may change depending on your system



# Add the necessary libraries
CUDA_LIBS = -lcuda -lcudart

# The following makes sure all path names (which often include spaces) are put between quotation marks
CUDA_INC = $$join(INCLUDEPATH,'" -I"','-I"','"')
#LIBS += $$join(CUDA_LIBS,'.so ', '', '.so')
LIBS += $$CUDA_LIBS

# Configuration of the Cuda compiler
CONFIG(debug, debug|release) {
    # Debug mode
    cuda_d.input = CUDA_SOURCES
    cuda_d.output = $$CUDA_OBJECTS_DIR/${QMAKE_FILE_BASE}_cuda.o
    cuda_d.commands = $$CUDA_DIR/bin/nvcc -D_DEBUG $$NVCC_OPTIONS $$CUDA_INC $$NVCC_LIBS --machine $$SYSTEM_TYPE -arch=$$CUDA_ARCH -c -o ${QMAKE_FILE_OUT} ${QMAKE_FILE_NAME}
    cuda_d.dependency_type = TYPE_C
    QMAKE_EXTRA_COMPILERS += cuda_d
}
else {
    # Release mode
    cuda.input = CUDA_SOURCES
    cuda.output = $$CUDA_OBJECTS_DIR/${QMAKE_FILE_BASE}_cuda.o
    cuda.commands = $$CUDA_DIR/bin/nvcc $$NVCC_OPTIONS $$CUDA_INC $$NVCC_LIBS --machine $$SYSTEM_TYPE -arch=$$CUDA_ARCH -c -o ${QMAKE_FILE_OUT} ${QMAKE_FILE_NAME}
    cuda.dependency_type = TYPE_C
    QMAKE_EXTRA_COMPILERS += cuda
}
