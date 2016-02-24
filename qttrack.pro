#-------------------------------------------------
#
# Project created by QtCreator 2016-02-23T19:18:00
#
#-------------------------------------------------

QT       += core gui widgets

TARGET = qttrack
TEMPLATE = app
CONFIG += link_pkgconfig c++11
PKGCONFIG += libical


SOURCES += main.cpp\
        timetablewindow.cpp \
    calendartask.cpp \
    calendarmodel.cpp \
    calendartimespan.cpp \
    treecalendarmodel.cpp \
    timespan.cpp

HEADERS  += timetablewindow.h \
    calendartask.h \
    calendarmodel.h \
    calendartimespan.h \
    treecalendarmodel.h \
    timespan.h

FORMS    += timetablewindow.ui

