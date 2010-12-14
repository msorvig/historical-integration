#ifndef BENCHMARKEVENT_H
#define BENCHMARKEVENT_H

#include <database.h>
#include <QtCore>

class BenchmarkEvent
{
public:
    BenchmarkEvent(Database *database,  const QString &dimentionName);
    void setDimention(const QString &dimentionName, const QString &subDimention, const QString dimentionInstance);
    void setResult(double result);
private:
    BenchmarkTable *m_benchmarkTable;
    QString m_dimentionName;
    QHash<QString, QString> m_subDimentions; // subDimention -> dimentionInstance
};

#endif // BENCHMARKEVENT_H
