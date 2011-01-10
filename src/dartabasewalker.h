#ifndef DARTABASEWALKER_H
#define DARTABASEWALKER_H

#include <QObject>
#include "database.h"
#include "attributedtable.h"
#include "benchmarkertable.h"

/*
    DatabaseWalker walks the "data tree" defined by the
    benchmark tables in the database.
*/
class DatabaseWalker : public QObject
{
public:
    DatabaseWalker(Database *database);

    void setup(AttributedTable *attributedTable);
    void setup(BenchmarkTable *rootBenchmarkTable);
    bool next();

    QString tableName();
    QStringList whereColumns() const;
    QStringList whereValues() const;

    QStringList dataColumns() const;
    QStringList dimentionColumns() const;

    bool isUniform() const;
    bool isData() const;

    // testing API:
    void printWalk(BenchmarkTable *rootBenchmarkTable);

private:
    Database *m_database;
    BenchmarkTable *m_rootTable;
    QStringList m_indexColumns;
    QStringList m_valueColumns;

    QStringList m_whereColumns;
    QStringList m_whereValues;

    QList<QStringList> m_workStack;
    QStringList m_current;
    QList<int> m_currentItems;

    int m_currentLevel;
    int m_maxLevel;
    int m_currentItem;
    int m_maxItem;
};

#endif // DARTABASEWALKER_H
