#ifndef BENCHMARKERTABLE_H
#define BENCHMARKERTABLE_H

#include "database.h"
#include <QtCore>
#include <QtSql>

class BenchmarkTable
{
public:
    BenchmarkTable(Database *database, const QString &tableName);
    ~BenchmarkTable();

    void updateBenchmarkTables(const QStringList &indexDimentions, const QStringList &valueDimentions);

    // Data inserting API:
    void setDimention(const QString &name, const QString &value);
    void setDimention(const QString &name, const QVariant &value);
    void clearDimention(const QString &name);

    void setValue(double value);
    void setValue(const QString &name, const QString &value);
    void setValue(const QString &name, double value);

    // TODO: void setValue(const QString &dimention, double value);

    void setAttribute(const QString &key, const QString &value);

    // Querying API
    QString tableName();
    QString attributeTableName();
    QStringList indexDimentions();
    QStringList valueDimentions();
    // Todo: units, description text, ++

    BenchmarkTable filtered(const QString &query);
private:
    void prepareValueInsertion(const QString &valueDimentionName, const QString sqlDimentionType);
    void insertValue(const QString &valueColumnName, const QVariant &value);

    // Dimention lists - maintained on inserts only.
    QHash<QString, QVariant> m_indexDimentions; // dimention name -> (current) value
    QHash<QString, QString> m_indexDimentionTypes; // dimention name -> type
    QStringList m_indexDimentionOrder;
    QHash<QString, QString> m_valueDimentionTypes; // dimention name -> type
    QStringList m_valueDimentionOrder;
    QString schemaFromDimentions();

    QSqlDatabase m_sqlDatabase;
    Database *m_database;
    QString m_tableName;
    QString m_attributeTableName;
    bool m_setValueCalled;
};

#endif // BENCHMARKERTABLE_H
