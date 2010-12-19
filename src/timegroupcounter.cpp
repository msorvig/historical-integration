#include "timegroupcounter.h"

TimeGroupCounter::TimeGroupCounter(Database *database)
{
    m_database = database;

}

AttributedTable TimeGroupCounter::count(AttributedTable table, const QString &groupColumn,
                                        const QString &timeColumn, Interval interval, const QString &returnTableName)
{
    uint minTime =
            m_database->selectVariant(QString("SELECT %1 FROM %2 ORDER BY Time ASC")
                                      .arg(timeColumn).arg(table.tableName())).toUInt();
    uint maxTime =
            m_database->selectVariant(QString("SELECT %1 FROM %2 ORDER BY Time DESC")
                                    .arg(timeColumn).arg(table.tableName())).toUInt();

    // Make sure we goup and count complete intervals only.
    uint intervalMinTime;
    if (interval == Week) {
        // Advance to minTime to the beginning of the first monday in the minTime, maxTime range
        QDateTime minDate(QDateTime::fromTime_t(minTime).date());
        QDateTime completeIntervalMinDate = minDate.addDays(7 - (minDate.date().dayOfWeek() - 1));
        if (completeIntervalMinDate.date().dayOfWeek() != 1)
            qDebug() << "Error: << TimeGroupCounter::count did not adwance to a monday" << completeIntervalMinDate;
        intervalMinTime = completeIntervalMinDate.toTime_t();
    } else if (interval == Day) {
        qDebug() << "Error: << TimeGroupCounter::count TODO";
    } else {
        qDebug() << "Error: << TimeGroupCounter::count TODO";

    }
//    qDebug() << "minTime" << minTime << intervalMinTime;

    // Create return table and count the number of rows for each time interval.
    // Group by groupColumn.

    AttributedTable counterTable(m_database, returnTableName);
    counterTable.setTableScema(QStringList() << groupColumn << timeColumn << "Count",
                             QStringList() << "VARCHAR" << "INTEGER" << "INTEGER");

    m_database->transaction();

    foreach (const QString group, m_database->selectDistinct(groupColumn, table.tableName())) {
        qDebug() << "group" << group;
        uint intervalTime = intervalMinTime;
        while (intervalTime + interval < maxTime) {
            QVariant count =
                    table.select("SELECT COUNT(*)FROM %table% WHERE TIME > ? AND TIME < ? AND Branch = ?"
                    , QVariantList() << intervalTime << intervalTime + interval << group);

//            qDebug () << "count for" << intervalTime <<
//                    QDateTime::fromTime_t(intervalTime).date()
//                    << "to" << QDateTime::fromTime_t(intervalTime + interval).date()
//                    << count;
            m_database->insertRow(returnTableName, QStringList() << groupColumn << timeColumn << "Count",
                                                 QList<QVariant>() << group << intervalTime << count);


            intervalTime += interval;
        }
    }

    m_database->commit();

    return counterTable;

}

