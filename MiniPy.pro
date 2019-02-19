#-------------------------------------------------
#
# Project created by QtCreator 2019-02-18T14:00:50
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = MiniPy
TEMPLATE = app


SOURCES += main.cpp\
        widget.cpp \
    v4l2Cap.c

HEADERS  += widget.h \
    v4l2Cap.h

FORMS    += widget.ui

RESOURCES +=
