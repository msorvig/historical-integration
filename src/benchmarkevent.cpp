#include "benchmarkevent.h"

BenchmarkEvent::BenchmarkEvent(Database *database, const QString &eventName)
{
    m_benchmarkTable = new BenchmarkTable(database, "Event" + eventName);
}

void BenchmarkEvent::setDimention(const QString &dimentionName, const QString &subDimention, const QString dimentionInstance)
{
    m_dimentionName = dimentionName;
    m_subDimentions[subDimention] = dimentionInstance;
}

void BenchmarkEvent::setResult(double result)
{
    QHash<QString, QString>::iterator it = m_subDimentions.begin();
    QHash<QString, QString>::iterator end = m_subDimentions.end();

    while (it != end) {
        int subDimentionIndex ;//= lookupSubDimentionIndex(m_dimentionName, it.key(), it.value());
        m_benchmarkTable->setDimention(m_dimentionName, QString(subDimentionIndex));
        ++it;
    }

    m_benchmarkTable->setValue("InstanceID", result);
}
