#-------------------------------------------------
#
# Project created by QtCreator 2018-07-04T17:46:52
#
#-------------------------------------------------
QT       += core gui
QT       += network

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = FM_master
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

#NOTE here you must add your own dir of thirdparty inc and lib
FFMPEG_INC_DIR = F:/thirdparty/ffmpeg-4.0-win64-dev/include
FFMPEG_LIB_DIR = F:/thirdparty/ffmpeg-4.0-win64-dev/lib
FMMANAGER_INC_DIR = F:/thirdparty/v0.1.0/include
FMMANAGER_LIB_DIR = F:/thirdparty/v0.1.0/libs
MAVLINK_INC_DIR = F:/thirdparty/v0.1.0/thirdparty/mavlink/include/mavlink/v1.0


INCLUDEPATH +=  $$FFMPEG_INC_DIR

INCLUDEPATH +=  $$FMMANAGER_INC_DIR \
                $$FMMANAGER_INC_DIR/FmAlgorthm \
                $$FMMANAGER_INC_DIR/FmLinkCore

INCLUDEPATH +=  $$MAVLINK_INC_DIR \
                $$MAVLINK_INC_DIR/multirotorpilot

SOURCES +=  main.cpp\
            mainwindow.cpp\
            DecodeThread.cpp\
            feimaplayer.cpp\
            nalu.cpp\
            SerialProg.cpp\
            UasProvideProcess.cpp


HEADERS  += mainwindow.h\
            DecodeThread.h\
            feimaplayer.h\
            nalu.h\
            SerialProg.h\
            UasProvideProcess.h

FORMS    += mainwindow.ui

LIBS += -L$$FFMPEG_LIB_DIR/ \
        -lavcodec   \
        -lavformat  \
        -lavutil    \
        -lswscale

LIBS += -L$$FMMANAGER_LIB_DIR/ \
        -lFmLinkInstance


LIBS += -L$$FMMANAGER_LIB_DIR/ \
        -lFmLinkCore
#unix:!macx|win32: LIBS += -L$$PWD/../../thirdparty/v0.1.0/libs/ -lFmLinkCore

#INCLUDEPATH += $$PWD/../../thirdparty/v0.1.0/libs
#DEPENDPATH += $$PWD/../../thirdparty/v0.1.0/libs

#unix:!macx|win32: LIBS += -L$$PWD/../../thirdparty/ffmpeg-4.0-win64-dev/lib/ -lavcodec \
#                        -lavformat  \
#                        -lavutil    \
#                        -lswscale   \
#                        -lFmLinkInstance

#INCLUDEPATH += $$PWD/../../thirdparty/ffmpeg-4.0-win64-dev/include
#DEPENDPATH += $$PWD/../../thirdparty/ffmpeg-4.0-win64-dev/include

