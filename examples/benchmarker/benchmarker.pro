include(../../src/buildbot.pri)

DEFINES += BENCHMARKER_DEV_MODE

TARGET = benchmarker
CONFIG += console
CONFIG -= app_bundle
CONFIG += release

TEMPLATE = app

SOURCES += main.cpp
