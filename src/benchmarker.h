#ifndef BENCHMARKER_H
#define BENCHMARKER_H

#include <QObject>
#include <QtTest>
#include "cycle.h"
#include "database.h"
#include "benchmarkertestlibintegration.h"


class Benchmarker : public QObject
{
public:
    Benchmarker();
    ~Benchmarker();
    double checkPoint();

    void setDimention(const QString &dimentionName, const QString &dimentionValue);
    void setResult(double result);
    double checkPointSetResult();
private:
    ticks  m_t0;
    double m_overhead;

    BenchmarkerTestlibIntegration testlibIntegration;
    BenchmarkTable *resultsTable;
};

#endif // BENCHMARKER_H
