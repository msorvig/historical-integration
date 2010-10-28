#include "dartabasewalker.h"


DatabaseWalker::DatabaseWalker(Database *database)
{
    m_database = database;
}

void DatabaseWalker::testWalk(BenchmarkTable *benchmarkTable)
{
    qDebug() << "Begin Walk";
    setup(benchmarkTable);
    while (next()) {
        qDebug() << "";
        qDebug() << "table name" << tableName();
        qDebug() << "where columns" << whereColumns();
        qDebug() << "where values" << whereValues();
    }
    qDebug() << "End Walk";
}

void DatabaseWalker::setup(BenchmarkTable *benchmarkTable)
{
    m_rootTable = benchmarkTable;
}

bool DatabaseWalker::next()
{
    return false;
}

QString DatabaseWalker::tableName()
{
    return QString();
}

QStringList DatabaseWalker::whereColumns() const
{
    return m_whereColumns;
}

QStringList DatabaseWalker::whereValues() const
{
    return m_whereValues;
}

QStringList DatabaseWalker::dataColumns() const
{
    return QStringList();
}
