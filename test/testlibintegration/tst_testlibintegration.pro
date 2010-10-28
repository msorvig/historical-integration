include(../../src/buildbot.pri)
QT += network sql testlib
TARGET = tst_testlibintegration
CONFIG += console
CONFIG -= app_bundle
TEMPLATE = app
SOURCES += tst_testlibintegration.cpp
HEADERS +=
