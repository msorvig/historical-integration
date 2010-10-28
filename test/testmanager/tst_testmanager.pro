include(../../src/buildbot.pri)
QT += network sql testlib
TARGET = tst_testmanager
CONFIG += console
CONFIG -= app_bundle
TEMPLATE = app
SOURCES += tst_testmanager.cpp
HEADERS +=
