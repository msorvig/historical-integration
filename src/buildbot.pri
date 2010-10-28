include ($$PWD/../../benchmarker/src/json.pri)
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
    watchdog.cpp \
    reports.cpp \
    driver.cpp \
    database.cpp \
    jsongenerator.cpp \
    dimentioaldatastore.cpp \
    webgenerator.cpp \
    livesearch.cpp \
    benchmarker.cpp \
    benchmarkertestlibintegration.cpp \
    hierarchydatagenerator.cpp \
    dartabasewalker.cpp \
    testrunner.cpp \
    log.cpp \
    testmanager.cpp \
    testbuilder.cpp \
    persistentqueue.cpp
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
    watchdog.h \
    reports.h \
    driver.h \
    database.h \
    jsongenerator.h \
    dimentioaldatastore.h \
    webgenerator.h \
    livesearch.h \
    benchmarker.h \
    benchmarkertestlibintegration.h \
    hierarchydatagenerator.h \
    dartabasewalker.h \
    testrunner.h \
    log.h \
    testmanager.h \
    testbuilder.h \
    persistentqueue.h
CONFIG += console
OBJECTS_DIR = .obj
MOC_DIR = .moc
QT += sql \
    testlib
RESOURCES += html.qrc
OTHER_FILES += buildreport.haml \
    jquery-1.4.2.js \
    table.haml \
    tablecontroller.js \
    singletable.js \
    livesearch.js \
    livesearch.js \
    livesearchdemo.js
