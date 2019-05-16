#-------------------------------------------------
#
# Project created by QtCreator 2019-02-18T14:00:50
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MiniPy
TEMPLATE = app

#导入opencv依赖库和头文件
INCLUDEPATH +=  /home/jiangyu/opencv/opencv-3.2.0/output/include \
                /home/jiangyu/opencv/opencv-3.2.0/output/include/opencv \
                /home/jiangyu/opencv/opencv-3.2.0/output/include/opencv2

LIBS += /home/jiangyu/opencv/opencv-3.2.0/output/lib/libopencv_highgui.so \
        /home/jiangyu/opencv/opencv-3.2.0/output/lib/libopencv_core.so    \
        /home/jiangyu/opencv/opencv-3.2.0/output/lib/libopencv_imgproc.so \
        /home/jiangyu/opencv/opencv-3.2.0/output/lib/libopencv_imgcodecs.so \
        /home/jiangyu/opencv/opencv-3.2.0/output/lib/libopencv_videoio.so


SOURCES += main.cpp\
        widget.cpp \
    v4l2Cap.c \
    workerthread.cpp \
    opencv_measure.cpp \
    makeexif.c

HEADERS  += widget.h \
    v4l2Cap.h \
    workerthread.h \
    opencv_measure.h \
    exifparam.h

FORMS    += widget.ui

RESOURCES +=
