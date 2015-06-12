#-------------------------------------------------
#
# Project created by QtCreator 2015-05-27T17:57:04
#
#-------------------------------------------------

QT       += core gui webkit

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
greaterThan(QT_MAJOR_VERSION, 4): QT += webkitwidgets

TARGET = qrigol
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    rigolcomm.cpp \
    mlogger.cpp \
    plotdialog.cpp \
    scopedata.cpp \
    helpdialog.cpp

HEADERS  += mainwindow.h \
    rigolcomm.h \
    mlogger.h \
    plotdialog.h \
    scopedata.h \
    helpdialog.h

FORMS    += mainwindow.ui \
    plotdialog.ui \
    helpdialog.ui


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
    packages/qrigol-0.1-2.x86_64.rpm \
    help.html \
    help.css \
    index.html \
    main.html \
    about.html \
    concept.html \
    status.html \
    verthor.html \
    trigger.html \
    measure.html \
    waveform.html \
    packages/qrigol_0.2_amd64.deb \
    packages/binary-release/Debug/qrigol \
    packages/binary-release/Relase/qrigol \
    qrigol.desktop

RESOURCES += \
    resources.qrc
