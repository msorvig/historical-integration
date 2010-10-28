#ifndef BENCHMARKERTESTLIBINTEGRATION_H
#define BENCHMARKERTESTLIBINTEGRATION_H

#include <QObject>
#include "database.h"
//#include "benchmarktable.h"

class BenchmarkerTestlibIntegration : public QObject
{
public:
    BenchmarkerTestlibIntegration();
    Database *database();
    BenchmarkTable *testCaseTable();
};

class TestCaseReportGenerator : public QObject
{
Q_OBJECT
public:

public slots:
    void createReport();

};

#endif // BENCHMARKERTESTLIBINTEGRATION_H
