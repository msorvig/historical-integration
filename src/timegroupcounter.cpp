#include "timegroupcounter.h"

TimeGroupCounter::TimeGroupCounter(Database *database)
{
    m_database = database;
}

void TimeGroupCounter::skipColumn(const QString &columnName)
{
    m_skipColumns.append(columnName);
}

AttributedTable TimeGroupCounter::aggregate(AttributedTable table)
{
    // qDebug() << "TimeGroupCounter::aggregate";

    QStringList resultTableColumnNames;
    QStringList resultTableColumnTypes;
    QString selectQuery = "SELECT ";
    QString queryGroupBy;
    QString querySelectColumns;
    QString queryAggregateColumns;

    const QStringList columnNames = table.columnNames();
    // qDebug() << "columns" << columnNames;
    foreach(const QString &columnName, columnNames) {
        if (m_skipColumns.contains(columnName))
            continue;
        QString role = table.attribute(columnName + "Role");
        // qDebug() << "column" << column << "role" << role;

        if (role == "Index") {
            resultTableColumnNames += columnName;
            resultTableColumnTypes += "VARCHAR"; // ### column type
            queryGroupBy += (columnName + ", ");
            querySelectColumns += (" " + columnName + ", ");
        } else if (role == "TimeIndex") {
            resultTableColumnNames += columnName;
            resultTableColumnTypes += "INTEGER";
            queryGroupBy += QString("strftime('%W-%Y', %1, 'unixepoch'), ").arg(columnName);
            querySelectColumns += (columnName + ", ");
        } else if (role == "Data") {
            resultTableColumnNames += columnName + "Sum";
            resultTableColumnTypes += "INTEGER";
            querySelectColumns += QString(" total(%1) AS %2, ").arg(columnName).arg(columnName + "Sum");

            resultTableColumnNames += columnName + "Average";
            resultTableColumnTypes += "INTEGER";
            querySelectColumns += QString(" avg(%1) AS %2, ").arg(columnName).arg(columnName + "Avg");
        }
    }

    querySelectColumns += " count(*) AS Count";
    resultTableColumnNames += "Count";
    resultTableColumnTypes += "INTEGER";

    queryGroupBy.chop(2); // remove last ", "

    selectQuery += (" " + querySelectColumns + "\n");
    selectQuery += (" " + queryAggregateColumns + "\n");
    selectQuery += QString(" FROM %1").arg(table.tableName() + "\n");
    selectQuery += (" GROUP BY " + queryGroupBy);

    //qDebug() << "query" << selectQuery;
    //qDebug() << "result table column names" << resultTableColumnNames;
    //qDebug() << "result table column types" << resultTableColumnTypes;

    // create new table
    AttributedTable counterTable(m_database, table.tableName() + "Aggregate");
    counterTable.setTableScema(resultTableColumnNames, resultTableColumnTypes);

    QString insertQuery(QString("INSERT INTO %1 ").arg(counterTable.tableName()) + selectQuery);
    m_database->execQuery(insertQuery, true);
    return counterTable;
}

