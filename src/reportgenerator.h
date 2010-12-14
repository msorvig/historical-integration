#ifndef REPORTGENERATOR_H
#define REPORTGENERATOR_H

#include <QtCore>
#include "database.h"

class ReportGenerator
{
public:
    ReportGenerator(Database *database, const QString &rootTableName);


    void generateReport(const QString &reportRootDirectory);

private:
    QByteArray generateJson(const QString &tableName);

    Database *m_database;
    QString m_rootTableName;
    QString m_reportRootDirectory;
};

#endif // REPORTGENERATOR_H
