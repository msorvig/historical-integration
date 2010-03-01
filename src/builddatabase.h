#ifndef BUILDDATABASE_H
#define BUILDDATABASE_H

#include <singletabledatabase.h>

class BuildDatabase : public SingleTableDatabase
{
public:
    BuildDatabase(const QString &location);
    void setBuildStatus(const QString &revision, const QString &status, const QString &log);
    QString getBuildStatus(const QString &revision);
    QString getBuildLog(const QString &revision);
};

#endif // BUILDDATABASE_H
