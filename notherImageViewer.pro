#-------------------------------------------------
#
# Project created by QtCreator 2017-06-10T23:45:19
#
#-------------------------------------------------

QT       += core gui qml quick

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = notherImageViewer
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp \
    slideshowview.cpp

HEADERS  += mainwindow.h \
    slideshowview.h

FORMS    += mainwindow.ui

DISTFILES += \
    slideshow.qml

RESOURCES += \
    qml.qrc
