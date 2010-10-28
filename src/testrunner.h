#ifndef TESTRUNNER_H
#define TESTRUNNER_H

#include <QObject>
#include <QtCore>
#include "process.h"

class TestRunResult
{
public:
    bool ranOk;
    QString stdOut;
    QString stdErr;
    int runTime;
};

class TestRunner : public QObject
{
public:
    TestRunner(const QString &testPath, const QString &qtBuildPath);

    bool isBuilt();

    QStringList testFunctions();
    QStringList testData(const QString &testFunction);

    //QStringList benchmarkFunctions();

    int startupTime();
    TestRunResult runTestExecutable(const QStringList &arguments);
    TestRunResult runTestFunction(const QString &test, const QString &data = QString());
    TestRunResult runTestFunctions(const QStringList &tests);
    TestRunResult runAllTestFunctions();

protected:
    void findTestExecutable(const QString &test);
    QString findQtLibs(const QString &test);
    int callTestExecutableWait(const QStringList &arguments, int timeout);

    QStringList parseTestFunctionsOutput(const QString &output);
    QStringList parseDataTagOutput(const QString &output);
    QStringList simplifyStrings(const QStringList &strings);

private:
    QProcess m_testProcess;
    QString m_testExecutablePath;
    QString m_testWorkDir;
    QString m_qtLibsPath;
    Process m_process;
};

#endif // TESTRUNNER_H
