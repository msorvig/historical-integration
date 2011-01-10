#ifndef REPORTGENERATOR_H
#define REPORTGENERATOR_H

#include <QtCore>
#include "database.h"

class ReportGenerator
{
public:
    ReportGenerator(Database *database, const QString &rootTableName);

    enum ReportMode { SelfContained, SelfContainedDev };
    void generateReport(const QString &reportRootDirectory, ReportMode mode = SelfContained );

private:
    QByteArray generateJson(const QString &tableName);

    Database *m_database;
    QString m_rootTableName;
    QString m_reportRootDirectory;
};

#endif // REPORTGENERATOR_H
