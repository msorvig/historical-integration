#ifndef JSONGENERATOR_H
#define JSONGENERATOR_H

#include <QtCore>
#include "Database.h"
#include "json.h"

class JsonGenerator
{
public:
    JsonGenerator(Database *database);
    // "flat" json
    QByteArray generateFlatJson(const QString &tableName);
    QByteArray generateFlatJson(BenchmarkTable *benchmarkTable);
    QByteArray generateFlatJson(const QString &tableName, const QStringList &hiearchyDimentions);
private:
    QList<QVariant> lookupDistinctColumnValues(const QString &columnName);

    Database *m_database;
    QString m_tableName;
    QHash<QString, QList<QVariant> > m_columnValues; // colunm name -> distinct column values

};

#endif // JSONGENERATOR_H
