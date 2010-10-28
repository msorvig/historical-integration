#include <QtCore>
#include <QtGui>
#include <QtTest>

#include <testrunner.h>
#include <testmanager.h>

class tst_testmanager : public QObject
{
Q_OBJECT
private slots:
    void testRunner();
    void testManager();
};

void tst_testmanager::testRunner()
{
    TestRunner runner("../testlibintegration", "c:/depot/qt");
    if (runner.isBuilt() == false) {
        //QSKIP("../testlibintegration not built, skipping test", QTest::SkipAll);
        qWarning("../testlibintegration not built, skipping test");
        return;
    }

    QStringList testFunctions = runner.testFunctions();
    qDebug() << "";
    qDebug() << "testFunctions" << testFunctions;
    QVERIFY(testFunctions.isEmpty() == false);

    foreach (const QString &testFunction, testFunctions) {
        QStringList data = runner.testData(testFunction);
        qDebug() << "data for testfunction" << testFunction << data;
    }

    int startupTime = runner.startupTime();
    QVERIFY(startupTime > 10);

    qDebug() << "";
    qDebug() << "startupTime" << startupTime;

    foreach (const QString &testFunction, testFunctions) {
        TestRunResult result = runner.runTestFunction(testFunction);
        qDebug() << "running" << testFunction << "runtime" << result.runTime;
        //qDebug() << "out" << result.stdOut;
    }

}

void tst_testmanager::testManager()
{
    TestManager manager("c:/depot/qt");
    QStringList tests = manager.tests();

    qDebug() << "tests" << tests.count();
}

QTEST_MAIN(tst_testmanager);

#include "tst_testmanager.moc"
