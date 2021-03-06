#include <QtCore>
#include <QtGui>
#include <QtTest>
#include <benchmarker.h>

class tst_testlibintegration : public QObject
{
Q_OBJECT
private slots:
    void f1();
    void f2();
    void f3();
    void f4_data();
    void f4();
    void timer();
};

void tst_testlibintegration::f1()
{
    qDebug() << "F1";
    Benchmarker benchmarker;
}

void tst_testlibintegration::f2()
{
    qDebug() << "F2";
    Benchmarker benchmarker;
}

void tst_testlibintegration::f3()
{
    qDebug() << "F3";
    Benchmarker benchmarker;
}

void tst_testlibintegration::f4_data()
{
    QTest::addColumn<int>("testdata");
    QTest::newRow("d1") << 1;
    QTest::newRow("d2") << 2;
    QTest::newRow("d3") << 3;
}

void tst_testlibintegration::f4()
{
    QFETCH(int, testdata);
    qDebug() << "test data" << testdata;
    Benchmarker benchmarker;
}

void tst_testlibintegration::timer()
{
    Benchmarker benchmarker;
    benchmarker.checkPoint();
//    qDebug() << "Hello";
    double res1 = benchmarker.checkPoint();
    double res2 = benchmarker.checkPoint();
    double res3 = benchmarker.checkPoint();
    double res4 = benchmarker.checkPoint();

    qDebug() << res1;
    qDebug() << res2;
    qDebug() << res3;
    qDebug() << res4;
}


QTEST_MAIN(tst_testlibintegration);

#include "tst_testlibintegration.moc"
