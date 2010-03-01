include(../../src/buildbot.pri)
QT += network \
    sql \
    testlib
TARGET = tst_buildbot
CONFIG += console
CONFIG -= app_bundle
TEMPLATE = app
SOURCES += main.cpp
HEADERS += 
