#-------------------------------------------------
#
# Project created by QtCreator 2015-05-27T17:57:04
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = qrigol
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    rigolcomm.cpp \
    mlogger.cpp \
    plotdialog.cpp

HEADERS  += mainwindow.h \
    rigolcomm.h \
    mlogger.h \
    plotdialog.h

FORMS    += mainwindow.ui \
    plotdialog.ui


OTHER_FILES += \
    README.md \
    COPYING \
    LICENSE \
    screenshots/screenshot_126.jpg \
    screenshots/screenshot_127.jpg \
    screenshots/screenshot_128.jpg \
    screenshots/screenshot_129.jpg
