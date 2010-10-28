#include "testmanager.h"
#include "log.h"
#include <QtCore>

TestManager::TestManager(const QString &qtSrcDir)
{
    qDebug() << "TestManager using Qt source at" << qtSrcDir;
    const QString testLocation = "/tests/auto/";

    m_testsPath = QDir::cleanPath(qtSrcDir + testLocation);

    //if (QDir::exists(m_testsPath) == false) {
    //    Log::addError("TestManager: tests directory does not exist: " + m_testsLocation);
    //}
}

QStringList TestManager::tests()
{
    QDir testsDir(m_testsPath);
    QStringList tests = testsDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    QStringList fullPathTests;

    foreach (const QString &test, tests) {
        fullPathTests.append(QDir::cleanPath(m_testsPath + "/" + test));
    }

    return fullPathTests;
}

QString TestManager::testVersion(const QString &test)
{
    return QString();
}

void TestManager::runAllTests()
{
    QStringList allTests = tests();

    foreach (const QString &test, allTests) {
        QString testPath = test;
        qDebug() << "testing" << test;

    }
}
