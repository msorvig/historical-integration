#ifndef JSONGENERATOR_H
#define JSONGENERATOR_H

#include <QtCore>
#include "Database.h"
#include "json.h"

class JsonGenerator
{
public:
    JsonGenerator(Database *database);
    // multi-dimentional table json
    QByteArray generateJson(BenchmarkTable *benchmarkTable);
    QByteArray generateJson(const QString &tableName, const QStringList &indexDimentions,
                                                      const QStringList &outputDimentions);

    // one-dimentional hiearchical json
    QByteArray generateHierarchyJson(BenchmarkTable *benchmarkTable);
    QByteArray generateHierarchyJson(const QString &tableName, const QStringList &hiearchyDimentions,
                                                               const QStringList &outputDimentions);


    //    QByteArray generateAssociativeJson(const QString &tableName, const QString &KeyColumn);

private:
    json_object * generateDimentionLabels(const QStringList &columns);
    json_object * generateDimentionValues(const QStringList &columns);
    json_object * generateDataRows(const QStringList &indexColumns, int currentIndex,
                                   const QStringList &outputColumns,
                                   const QStringList &whereLabels, const QStringList &whereValues);
    json_object * generateDataTable(const QStringList &indexColumns, const QStringList &outputColumns);

    json_object * generateHierarchyDataRows(const QStringList &indexColumns, int currentIndex,
                                   const QStringList &outputColumns,
                                   const QStringList &whereLabels, const QStringList &whereValues);
    json_object * generateHierarchyDataTable(const QStringList &indexColumns, const QStringList &outputColumns);

    QStringList lookupDistinctColumnValues(const QString &columnName);
    QStringList lookupDistinctColumnValuesWhere(const QString &columnName,
                                                const QStringList &whereLabels,
                                                const QStringList &whereValues);

    Database *m_database;
    QString m_tableName;
    QHash<QString, QStringList> m_columnValues; // colunm name -> distinct column values

};

#endif // JSONGENERATOR_H
