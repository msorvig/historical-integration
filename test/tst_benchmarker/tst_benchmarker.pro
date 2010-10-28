include(../../src/buildbot.pri)
QT += network \
    sql \
    testlib
TARGET = tst_benchmarker
CONFIG += console
CONFIG -= app_bundle
TEMPLATE = app
SOURCES += tst_benchmarker.cpp
HEADERS +=
