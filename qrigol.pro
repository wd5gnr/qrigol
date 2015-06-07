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
    plotdialog.cpp \
    scopedata.cpp

HEADERS  += mainwindow.h \
    rigolcomm.h \
    mlogger.h \
    plotdialog.h \
    scopedata.h

FORMS    += mainwindow.ui \
    plotdialog.ui


OTHER_FILES += \
    README.md \
    COPYING \
    LICENSE \
    screenshots/screenshot_126.jpg \
    screenshots/screenshot_127.jpg \
    screenshots/screenshot_128.jpg \
    screenshots/screenshot_129.jpg \
    screenshots/screenshot_130.jpg \
    screenshots/screenshot_131.jpg \
    icons/greenlight.png \
    icons/redlight.png \
    icons/disconnect,png \
    icons/connect.png \
    screenshots/screenshot_200.png \
    screenshots/screenshot_201.png \
    screenshots/screenshot_202.png \
    screenshots/screenshot_203.png \
    screenshots/screenshot_204.png \
    screenshots/screenshot_205.png \
    packages/qrigol_0.1_amd64.deb \
    packages/qrigol-0.1-2.x86_64.rpm

RESOURCES += \
    resources.qrc
