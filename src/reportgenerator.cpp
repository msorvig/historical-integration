#include "reportgenerator.h"
#include "jsongenerator.h"
#include "webgenerator.h"

ReportGenerator::ReportGenerator(Database *database, const QString &rootTableName)
{
    m_database = database;
    m_rootTableName = rootTableName;
}

void ReportGenerator::generateReport(const QString &reportRootDirectory)
{
    m_reportRootDirectory = reportRootDirectory;

    QByteArray json = generateJson(m_rootTableName);

    WebGenerator webGenerator;
#ifdef BENCHMARKER_DEV_MODE
    QByteArray report = webGenerator.instantiateSelfContainedDev("report.html", json);
#else
    QByteArray report = webGenerator.instantiateSelfContained("report.html", json);
#endif
    writeFile(m_rootTableName.toLower() + ".html", report);
}

QByteArray ReportGenerator::generateJson(const QString &tableName)
{
    JsonGenerator jsonGenerator(m_database);
    return jsonGenerator.generateFlatJson(tableName);
}


