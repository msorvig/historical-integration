#ifndef DARTABASEWALKER_H
#define DARTABASEWALKER_H

#include <QObject>
#include "database.h"

class DatabaseWalker : public QObject
{
public:
    DatabaseWalker(Database *database);

    void testWalk(BenchmarkTable *benchmarkTable);

    void setup(BenchmarkTable *benchmarkTable);
    bool next();

    QString tableName();
    QStringList whereColumns() const;
    QStringList whereValues() const;
    QStringList dataColumns() const;
private:
    Database *m_database;
    BenchmarkTable *m_rootTable;
    QStringList m_whereColumns;
    QStringList m_whereValues;
};

#endif // DARTABASEWALKER_H
