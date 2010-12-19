#ifndef TIMEGROUPCOUNTER_H
#define TIMEGROUPCOUNTER_H

#include <QtCore>
#include "database.h"
#include "attributedtable.h"

class TimeGroupCounter
{
public:
    enum Interval { Day = 86400,
                    Week = Day * 7,
                    ProgrammersMonth = Day * 30 };
    TimeGroupCounter(Database *database);
    AttributedTable count(AttributedTable table, const QString &groupColumn,
                          const QString &timeColumn, Interval interval, const QString &returnTableName);
private:
    Database *m_database;
};

#endif // TIMEGROUPCOUNTER_H
