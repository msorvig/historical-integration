# DEFINES += BENCHMARKER_DEV_MODE

include(../../src/buildbot.pri)

TARGET = qtstats
CONFIG += console
CONFIG -= app_bundle
CONFIG += release

TEMPLATE = app

SOURCES += main.cpp
