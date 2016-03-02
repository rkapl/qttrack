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
    calendartask.cpp \
    calendarmodel.cpp \
    calendartimespan.cpp \
    treecalendarmodel.cpp \
    timespan.cpp \
    timelistdialog.cpp \
    tasklistwindow.cpp \
    libicalflusher.cpp \
    fixtimewidget.cpp

HEADERS  += \
    calendartask.h \
    calendarmodel.h \
    calendartimespan.h \
    treecalendarmodel.h \
    timespan.h \
    timelistdialog.h \
    tasklistwindow.h \
    libicalflusher.h \
    fixtimewidget.h

FORMS    += timetablewindow.ui \
    timelistdialog.ui

RESOURCES += \
    resources.qrc

include(tests/tests.pri)

DISTFILES += \
    ACKNOWLEDGEMENTS.TXT

