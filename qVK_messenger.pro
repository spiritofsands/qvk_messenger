#-------------------------------------------------
#
# Project created by QtCreator 2016-06-23T13:45:02
#
#-------------------------------------------------

QT       += core gui network webenginewidgets
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

QMAKE_CXXFLAGS += -Wfatal-errors -pedantic-errors

TARGET = qVK_messenger
TEMPLATE = app

CONFIG += c++11

SOURCES += main.cpp\
        mainwindow.cpp \
    settingsmanager.cpp \
    networklogic.cpp \
    networklogic_auth.cpp \
    networklogic_requests.cpp \
    mainwindow_msg.cpp \
    dialogdelegate.cpp

HEADERS  += mainwindow.h \
    settingsmanager.h \
    storage.h \
    networklogic.h \
    request.h \
    message.h \
    profile.h \
    dialogdelegate.h

FORMS    += mainwindow.ui

RESOURCES += \
    res.qrc

DISTFILES += \
    BUGS \
    TODO
