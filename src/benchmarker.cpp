#include "benchmarker.h"
#include <QtCore>
#include <QtTest>

Benchmarker::Benchmarker()
{
    const ticks t0 = getticks();
    const ticks t1 = getticks();
    m_overhead = elapsed(t1, t0);
    qDebug() << "overhead" << m_overhead;
    m_overhead = elapsed(t1, t0);
    qDebug() << "overhead" << m_overhead;
    m_overhead = elapsed(t1, t0);
    qDebug() << "overhead" << m_overhead;

    testlibIntegration.database()->database().transaction();
}

Benchmarker::~Benchmarker()
{
    qDebug() << "commit";
    testlibIntegration.database()->database().commit();
}

double Benchmarker::checkPoint()
{
    ticks t1 = getticks();
    double result = qMax(0.0, elapsed(t1, m_t0) - m_overhead);
    m_t0 = t1;
    return result;
}

void Benchmarker::setDimention(const QString &dimentionName, const QString &dimentionValue)
{
    resultsTable->setDimention(dimentionName, dimentionValue);
}

void Benchmarker::setResult(double result)
{
    resultsTable->setValue(result);
}

double Benchmarker::checkPointSetResult()
{
    const double result = checkPoint();
    resultsTable->setValue(result);
    return result;
}

