#ifndef TESTMANAGER_H
#define TESTMANAGER_H

#include <QObject>
#include <testrunner.h>

class TestManager : public QObject
{
public:
    TestManager(const QString &qtSrcDir);

    QStringList tests();
    QString testVersion(const QString &test);

    void runAllTests();
signals:
    void testCompleted(const QString &test, TestRunResult result);

private:
    QString m_testsPath;

};

#endif // TESTMANAGER_H
