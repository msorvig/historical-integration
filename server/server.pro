include(../src/buildbot.pri)
QT += network \
    sql
TARGET = server
CONFIG += console
CONFIG -= app_bundle
TEMPLATE = app
SOURCES += main.cpp
HEADERS +=
