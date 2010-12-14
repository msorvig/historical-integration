#include "benchmarkdimention.h"

BenchmarkDimention::BenchmarkDimention(Database *database, const QString &dimentionName)
{
    m_benchmarkTable = new BenchmarkTable(database, "Dimention" + dimentionName);
}

void BenchmarkDimention::setDimention(const QString &dimentionName, const QString dimentionInstance)
{
    m_benchmarkTable->setDimention(dimentionName, dimentionInstance);
}

void BenchmarkDimention::commitInstance()
{
    static int instanceId = 0; // ###
    m_benchmarkTable->setValue("InstanceID", ++instanceId);
}
