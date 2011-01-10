#ifndef TIMEGROUPCOUNTER_H
#define TIMEGROUPCOUNTER_H

#include <QtCore>
#include "database.h"
#include "attributedtable.h"

class TimeGroupCounter
{
public:
    enum Interval { Day, Week, Month };

    TimeGroupCounter(Database *database);
    void skipColumn(const QString &columnName);

    AttributedTable aggregate(AttributedTable table);
private:
    Database *m_database;
    QString m_skipColumns;
};

#endif // TIMEGROUPCOUNTER_H
