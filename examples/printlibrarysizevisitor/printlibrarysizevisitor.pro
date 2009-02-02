include (../../src/buildbot.pri)
TEMPLATE = app
TARGET = printlibrarysizevisitor
DEPENDPATH += .
INCLUDEPATH += .

# Input
SOURCES += main.cpp
CONFIG += qt warn_on debug  create_prl link_prl
OBJECTS_DIR = .obj/debug-shared
MOC_DIR = .moc/debug-shared
