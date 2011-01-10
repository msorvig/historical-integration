#include "dartabasewalker.h"


DatabaseWalker::DatabaseWalker(Database *database)
{
    m_database = database;
}

void DatabaseWalker::printWalk(BenchmarkTable *benchmarkTable)
{
    if (benchmarkTable == 0)
        benchmarkTable = new BenchmarkTable(m_database, "testFunctionRuntime"); // ###

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

void DatabaseWalker::setup(AttributedTable *attributedTable)
{

}

void DatabaseWalker::setup(BenchmarkTable *benchmarkTable)
{
    m_rootTable = benchmarkTable;
    m_indexColumns = benchmarkTable->indexDimentions();
    m_valueColumns = benchmarkTable->valueDimentions();


    m_workStack.append(m_database->selectDistinct(m_indexColumns.at(0), benchmarkTable->tableName()));
    m_currentLevel = 0;
    m_maxLevel = m_indexColumns.count() - 1;
    /*
    qDebug() << m_indexColumns
             << m_valueColumns
             << m_database->selectDistinct(m_indexColumns.at(0), benchmarkTable->tableName())
             << m_database->selectDistinct(m_indexColumns.at(1), benchmarkTable->tableName());
*/
}

bool DatabaseWalker::next()
{

    while (m_workStack.at(m_currentLevel).isEmpty()) {
        // done with the current level, ascend
        if (m_currentLevel == 0)
            return false; // done
        // pop
        m_currentLevel--;
    }

    while (m_currentLevel <= m_maxLevel) {
        // pick next at this level
        QString item = m_workStack[m_currentLevel].takeFirst();
        m_current[m_currentLevel] = item;
    }
    // descend if possible;




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
