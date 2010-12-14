#include "benchmarker.h"
#include <QtCore>
#include <QtTest>
#include "database.h"
#include "reportgenerator.h"

extern Database *g_database();

Benchmarker::Benchmarker(const QString &benchmarkName)
{
    const ticks t0 = getticks();
    const ticks t1 = getticks();
    m_overhead = elapsed(t1, t0);
    qDebug() << "overhead" << m_overhead;
    m_overhead = elapsed(t1, t0);
    qDebug() << "overhead" << m_overhead;
    m_overhead = elapsed(t1, t0);
    qDebug() << "overhead" << m_overhead;

    // are we running in a QTestLib test function?
    if (qstrlen(QTest::currentTestFunction()) != 0) {
        m_testlibIntegration = new BenchmarkerTestlibIntegration();
        m_database = m_testlibIntegration->database();
    } else {
        m_testlibIntegration = 0;
        QString databasePath = QDir::cleanPath(QDir::currentPath() + "/" +
                               benchmarkName.toLower() + ".sqlite");
        // Mutliple test runs are currently not supported. Delete the database.
        Database(databasePath).deleteDatabase();
        m_database = new Database(databasePath);
    }
    m_resultsTable = new BenchmarkTable(m_database, benchmarkName.toLower());
    m_resultsTable->setAttribute("Name", benchmarkName);
    m_database->transaction();
}

Benchmarker::~Benchmarker()
{
    qDebug() << "commit";
    m_database->commit();

    if(m_testlibIntegration == 0)
        createReport();

    delete m_testlibIntegration;
    delete m_resultsTable;
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
    m_resultsTable->setDimention(dimentionName, QVariant(dimentionValue));
}

void Benchmarker::setDimention(const QString &dimentionName, int dimentionValue)
{
    m_resultsTable->setDimention(dimentionName, QVariant(dimentionValue));
}

void Benchmarker::setDimention(const QString &dimentionName, double dimentionValue)
{
    m_resultsTable->setDimention(dimentionName, QVariant(dimentionValue));
}

void Benchmarker::setResult(double result)
{
    m_resultsTable->setValue(result);
}

double Benchmarker::checkPointSetResult()
{
    const double result = checkPoint();
    m_resultsTable->setValue(result);
    return result;
}

void Benchmarker::setBenchmarkTitle(const QString &title)
{
    m_resultsTable->setAttribute("BenchmarkTitle", title);
}

void Benchmarker::setDimentionTitle(const QString &dimentionName, const QString &title)
{
    m_resultsTable->setAttribute("DimentionTitle" + dimentionName, title);
}

void Benchmarker::setResultTitle(const QString &title)
{
    m_resultsTable->setAttribute("ResultTitle", title);
}

void Benchmarker::createReport()
{
    ReportGenerator reportGenerator(m_database, m_resultsTable->tableName());
    reportGenerator.generateReport(QDir::currentPath());
}

