#ifndef DATABASE_H
#define DATABASE_H

#include <QObject>
#include <QtCore>
#include <QtSql>

class Database : public QObject
{
public:
    Database();
    Database(const QString &path);
    ~Database();
    void deleteDatabase();
    QSqlDatabase database();

    QStringList tables();
    QStringList columns(const QString &tableName);
    QString schema(const QString &m_tableName);

    void transaction();
    void commit();

    QString schemaFromColumns(const QStringList &columnNames, const QStringList &columnTypes);
    bool updateTableSchema(const QString &tableName, const QStringList &columnNames, const QStringList &columnTypes);

    QString createWhereClause(const QStringList &whereColumns);
    void bindWhereValues(QSqlQuery * query, const QStringList &whereValues);

    QString selectString(const QString &queryString);
    QVariant selectVariant(const  QString &queryString);
    QStringList selectStringList(const QString &queryString);
    QStringList selectDistinct(const QString &field, const QString &tableName);
    QList<QVariant> selectDistinctVariants(const QString &field, const QString &tableName);
    QStringList selectDistinctWhere(const QString &field, const QString &tableName,
                                    const QStringList &whereColumns, const QStringList &whereValues);

    void displayTable(const QString &table);

    static QString scrub(const QString &string);
    QVariant selectCell(const QString &tableName,
                        const QString &selectColumnName,
                        const QString &whereColumnName, const QVariant &whereColumnValue);
    QList<QVariant> selectMultipleWhere(const QString &tableName, const QStringList &selectColumns,
                                        const QStringList &whereColumns, const QStringList &whereValues);

    void insertRow(const QString &tableName,
                   const QStringList &insertColumnNames, const QList<QVariant> &insertColumnValues);
    void updateRow(const QString &tableName,
                   const QStringList &insertColumnNames, const QList<QVariant> &insertColumnValues,
                   const QString &whereColumnName, const QVariant &whereColumnValue);

    void insertCell(const QString &tableName,
                    const QString &insertColumnName, const QVariant &insertColumnValue);

    void updateCell(const QString &tableName,
                    const QString &insertColumnName, const QVariant &insertColumnValue,
                    const QString &whereColumnName, const QVariant &whereColumnValue);

    QString createTempTable(const QString &sourceTableName);
    void destroyTempTable(const QString &tableName);
    QString filterTable(const QString &sourceTableName, const QString &filterQuery);

    void execQuery(QSqlQuery query, bool warnOnFail);
    QSqlQuery execQuery(const QString &spec, bool warnOnFail);
protected:
    void openDatabase();
    void closeDatabase();
    void createNewDatabase();
    void connectToDatabase();
    bool databaseExists();
    void executeSqlitePragmas();
private:
    QString m_path;
    QSqlDatabase m_database;
    int m_transactionCount;
    int m_tempTableIndex;
    QHash<QString, QSqlQuery> m_queryCache;
};

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

#endif // DATABASE_H
