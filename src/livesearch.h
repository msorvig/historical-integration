#ifndef LIVESEARCH_H
#define LIVESEARCH_H

#include <QtCore>
#include "database.h"

class LiveSearch : public QObject
{
public:
    LiveSearch(Database *database, const QString &tableName);
    void generateIndexFiles(const QString &path);

    Database *m_database;
    QString m_tableName;
};

#endif // LIVESEARCH_H
