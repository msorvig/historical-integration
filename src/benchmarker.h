#ifndef BENCHMARKER_H
#define BENCHMARKER_H

#include <QObject>
#include <QtTest>
#include "cycle.h"
#include "database.h"
#include "benchmarkertestlibintegration.h"

class Benchmarker
{
public:
    Benchmarker(const QString &benchmarkName);
    ~Benchmarker();
    double checkPoint();

    void setDimention(const QString &dimentionName, const QString &dimentionValue);
    void setDimention(const QString &dimentionName, int dimentionValue);
    void setDimention(const QString &dimentionName, double dimentionValue);

    void setResult(double result);
    double checkPointSetResult();

    void setBenchmarkTitle(const QString &title);
    void setDimentionTitle(const QString &dimentionName, const QString &title);
    void setResultTitle(const QString &title);
private:
    void createReport();

    ticks  m_t0;
    double m_overhead;

    BenchmarkerTestlibIntegration *m_testlibIntegration;
    BenchmarkTable *m_resultsTable;
    Database *m_database;
};

#endif // BENCHMARKER_H
