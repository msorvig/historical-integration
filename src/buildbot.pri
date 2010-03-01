DEPENDPATH += $$PWD
INCLUDEPATH += $$PWD
SOURCES += $$PWD/buildbot.cpp \
    $$PWD/taskmanager.cpp \
    visitor.cpp \
    gitclient.cpp \
    util.cpp \
    testvisitor.cpp \
    guide.cpp \
    qtbranches.cpp \
    singletabledatabase.cpp \
    mainlinechangedetector.cpp \
    qtbuildmanager.cpp \
    dedupstore.cpp \
    repositorytracker.cpp \
    process.cpp \
    globalsettings.cpp \
    builddatabase.cpp \
    watchdog.cpp
HEADERS += $$PWD/buildbot.h \
    $$PWD/taskmanager.h \
    visitor.h \
    gitclient.h \
    util.h \
    testvisitor.h \
    guide.h \
    qtbranches.h \
    singletabledatabase.h \
    mainlinechangedetector.h \
    qtbuildmanager.h \
    dedupstore.h \
    repositorytracker.h \
    process.h \
    globalsettings.h \
    builddatabase.h \
    watchdog.h
CONFIG += console
OBJECTS_DIR = .obj
MOC_DIR = .moc
QT += sql
