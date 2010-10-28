#include "builddatabase.h"
#include "globalsettings.h"
#include "reports.h"

QString buildsTable = QString("Revision varchar, Status varchar, Log varchar");

BuildDatabase::BuildDatabase()
:SingleTableDatabase(GlobalSettings::instance()->workspaceLocation + QLatin1String("/database/buildstatus")
                     , buildsTable, QLatin1String("Builds"))
{
    openDatabase();
}

BuildDatabase::BuildDatabase(const QString &location)
:SingleTableDatabase(location, buildsTable, QLatin1String("Builds"))
{
    openDatabase();
}

void BuildDatabase::setBuildStatus(const QString &revision, const QString &status, const QString &log)
{
    qDebug() << "setBuildStatus" << revision << status;

    QSqlQuery query(m_database);

    query.prepare("INSERT INTO Builds (Revision, Status, Log) VALUES (:Revision, :Status, :Log)");

    query.bindValue(":Revision", revision);
    query.bindValue(":Status", status);
    query.bindValue(":Log", log);
    execQuery(query, true);
}

QString BuildDatabase::getBuildStatus(const QString &revision)
{
    QSqlQuery query(m_database);
    query.prepare("SELECT Status FROM Builds WHERE Revision='" + revision + "'");
    bool ok = query.exec();
    if (!ok) {
        qDebug() << "sql query exec failure:" << query.lastQuery() << query.lastError().text();
    }

    if (query.isActive()) {
        query.next();
        return query.value(0).toString();
    }
    return QByteArray();
}

QString BuildDatabase::getBuildLog(const QString &revision)
{
    QSqlQuery query;
    query.prepare("SELECT Log FROM Builds WHERE Revision='" + revision + "'");
    bool ok = query.exec();
    if (!ok) {
        qDebug() << "sql query exec failure:" << query.lastQuery() << query.lastError().text();
    }

    if (query.isActive()) {
        query.next();
        return query.value(0).toString();
    }
    return QString();
}

void BuildDatabase::writeReport()
{
    QString reportHtmlLocation = GlobalSettings::instance()->workspaceLocation
                                 + QLatin1String("/html/static/buildstatus.html");
    QString reportJsonLocation = GlobalSettings::instance()->workspaceLocation
                                 + QLatin1String("/html/dynamic/buildstatus.json");

    writeFile(reportHtmlLocation, "Hello");
    writeFile(reportJsonLocation, "World");

}
