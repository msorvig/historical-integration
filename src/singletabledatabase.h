#ifndef SINGLETABLEDATABASE_H
#define SINGLETABLEDATABASE_H

#include <QtCore>
#include <QtSql>

class SingleTableDatabase
{
public:
    SingleTableDatabase(const QString &location, const QString &tableSpec, const QString &tableName);
    ~SingleTableDatabase();

    void openDatabase();
    void closeDatabase();
    void deleteDatabase();

    void createTable();
    void dropTable();

    QSqlTableModel *sqlTableModel();
    QSqlTableModel *sqlTableSpecModel();

    QString selectStringQuery(const QString &queryString);
    void execQuery(QSqlQuery query, bool warnOnFail);
    QSqlQuery execQuery(const QString &spec, bool warnOnFail = true);
private:
    QString getDatabaseTableSpec();
    void setDatabaseTableSpec(const QString &spec);
    void addColumns(const QMap<QString, QString> &columns);
    void createNewDatabase();
    void connectToDatabase();
    bool databaseExists();
    bool tryUpdateDatabaseTableSchema();

protected:
    QString m_databaseLocation;
    QString m_tableName;
    QString m_tableSpec;
public:
    QSqlDatabase m_database;
};

#define SINGLE_TABLE_DATABASE_INSERT_IMPLEMENTATION_2(Column1, Column2) \
QSqlQuery query(m_database); \
query.prepare("INSERT INTO " + m_tableName + " ("#Column1","#Column2") VALUES (:"#Column1",:"#Column2")"); \
query.bindValue(":"#Column1, Column1); \
query.bindValue(":"#Column2, Column2); \
execQuery(query, true);

#define sSINGLE_TABLE_DATABASE_SELECT_IMPLEMENTATION_1(ReturnType, ReturnColumn, SelectColumn) \
QSqlQuery query(m_database); \
    query.prepare("SELECT * FROM test");\
    query.exec();\
    if (query.isActive()) {\
        query.next();\
        qDebug() << "active" << query.value(0);\
    } else {\
        query.lastError().text();\
    }
/*
#define SINGLE_TABLE_DATABASE_SELECT_IMPLEMENTATION_1(ReturnType, ReturnColumn, SelectColumn) \
QSqlQuery query(m_database); \
query.prepare("SELECT * FROM test"); \
execQuery(query, true); \
if (query.isActive()) { \
    query.next(); \
    if (query.isValid() == false) \
        qDebug() << "sql SELECT exec failure:" << query.lastQuery() << query.lastError().text();\
    return query.value(0).to##ReturnType(); \
} \
return Q##ReturnType();
*/
#endif // SINGLETABLEDATABASE_H
