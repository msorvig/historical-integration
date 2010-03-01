#include "singletabledatabase.h"

#include <QtCore>

// converts a "Column1 type1, Column2 type2" type schema string
// into a Column -> Type hash.
QMap<QString, QString> schemaToMap(const QString &schema)
{
    QMap<QString, QString> columns;
    foreach (const QString &part, schema.split(QLatin1Char(','), QString::SkipEmptyParts)) {
        QStringList parts = part.trimmed().split(QLatin1Char(' '), QString::SkipEmptyParts);
        columns.insert(parts.at(0), parts.at(1));
    }

    return columns;
}

bool hasConflicts(const QMap<QString, QString> &a, const QMap<QString, QString> &b)
{
    QMapIterator<QString, QString> it(a);
    while(it.hasNext()) {
        it.next();
        if (b.contains(it.key()) && b.value(it.key()) != it.value())
            return false;
    }
    return false;
}

QString mapToSchema(const QMap<QString, QString> &columns)
{
    QString schema;
    QMapIterator<QString, QString> it(columns);
    while(it.hasNext()) {
        it.next();
        schema+= it.key() + QLatin1String(" ") + it.value() + ", ";
    }
    schema.chop(2); // remove last ", "
    return schema;
}

QMap<QString, QString> add(const QMap<QString, QString> &a, const QMap<QString, QString> &b)
{
    QMap<QString, QString> dest = b;
    QMapIterator<QString, QString> it(a);
    while(it.hasNext()) {
        it.next();
        if (dest.contains(it.key()) == false)
            dest.insert(it.key(), it.value());
    }
    return dest;
}

// returns a - b
QMap<QString, QString> sub(const QMap<QString, QString> &a, const QMap<QString, QString> &b)
{
    QMap<QString, QString> dest;
    QMapIterator<QString, QString> it(a);
    while(it.hasNext()) {
        it.next();
        if (b.contains(it.key()) == false)
            dest.insert(it.key(), it.value());
    }
    return dest;
}

SingleTableDatabase::SingleTableDatabase(const QString &location, const QString &tableSpec, const QString &tableName)
{
    m_databaseLocation = location;
    m_tableSpec = tableSpec;
    m_tableName = tableName;
}

SingleTableDatabase::~SingleTableDatabase()
{
    m_database.close();
}


QString SingleTableDatabase::getDatabaseTableSpec()
{
    return selectStringQuery("SELECT Spec FROM TableSpec");
}

void SingleTableDatabase::setDatabaseTableSpec(const QString &tableSpec)
{
    qDebug() << "setDatabaseTableSpec" << tableSpec;
    QSqlQuery query(m_database);
    query.prepare("INSERT INTO TableSpec (Spec) VALUES (:Spec)");
    query.bindValue(":Spec", tableSpec);

}

void SingleTableDatabase::addColumns(const QMap<QString, QString> &columns)
{
    qDebug() << "addColumns" << columns;
    QMapIterator<QString, QString> it(columns);
    while (it.hasNext()) {
        it.next();
        qDebug() << "add columsn" << it.key() << it.value();
        QSqlQuery query(m_database);
        query.prepare("ALTER TABLE " + m_tableName + " ADD COLUMN " + it.key() + " " + it.value());
        execQuery(query, true);
    }
}

void SingleTableDatabase::createNewDatabase()
{
    qDebug() << "createNewDatabase" << m_databaseLocation;
    m_database.setDatabaseName(m_databaseLocation);
    bool ok = m_database.open();
    if (!ok)
        qDebug() << "FAIL: could not create database" << m_databaseLocation;

    execQuery("PRAGMA encoding = \"UTF-8\";", true);

    execQuery("CREATE TABLE TableSpec (Spec varchar)");

    QSqlQuery query(m_database);
    query.prepare("INSERT INTO TableSpec (Spec) VALUES (:Spec)");
    query.bindValue(":Spec", m_tableSpec);
    execQuery(query, true);
    execQuery("CREATE TABLE " +m_tableName + " (" + m_tableSpec + ")");
}

void SingleTableDatabase::connectToDatabase()
{
    m_database.setDatabaseName(m_databaseLocation);
    bool ok = m_database.open();
    if (!ok)
        qDebug() << "FAIL: could not open database" << m_databaseLocation;
}

bool SingleTableDatabase::databaseExists()
{
    return QFile(m_databaseLocation).exists();
}

bool SingleTableDatabase::tryUpdateDatabaseTableSchema()
{
    QMap<QString, QString> newColumnsSet = schemaToMap(m_tableSpec);
    QMap<QString, QString> databaseColumnsSet = schemaToMap(getDatabaseTableSpec());

    qDebug() << "tryUpdateDatabaseTableSchema" << newColumnsSet << databaseColumnsSet;

    if (hasConflicts(newColumnsSet, databaseColumnsSet))
        return true;

    QMap<QString, QString> newColumns = sub(newColumnsSet, databaseColumnsSet);
    addColumns(newColumns);
    setDatabaseTableSpec(mapToSchema(add(newColumnsSet, databaseColumnsSet)));

    return false;
}

void SingleTableDatabase::closeDatabase()
{
    m_database.close();
}

void SingleTableDatabase::deleteDatabase()
{
    m_database.close();
    QFile::remove(m_databaseLocation);
}

void SingleTableDatabase::openDatabase()
{
    qDebug() << "openDatabase" << m_databaseLocation << m_tableName;
    if (m_database.isValid() == false)
        m_database = QSqlDatabase::addDatabase("QSQLITE");

    if (m_database.isOpen())
        return;

    if (databaseExists() == false) {
        createNewDatabase();
    } else {
        connectToDatabase();
        bool hadConflicts = tryUpdateDatabaseTableSchema();
        if (hadConflicts) {
            deleteDatabase();
            createNewDatabase();
        }
    }
/*
    QMap<QString, QString> columns = schemaToMap(m_tableSpec);
    qDebug() << columns;
    qDebug() << normalizedSchema(columns);

    QString databaseSchema = selectStringQuery("SELECT Spec from TABLE TableSpec");
    QMap<QString, QString> databaseColumns = schemaToMap(databaseSchema);
    qDebug() << databaseColumns;

    qDebug() << "to add " << sub(columns, databaseColumns);
    qDebug() << "new shcema" << normalizedSchema(add(columns, databaseColumns));

    qDebug() << "open database at" << m_databaseLocation;
    m_database = QSqlDatabase::addDatabase("QSQLITE");
    qDebug() << "db" << m_database.isValid() << QSqlDatabase::isDriverAvailable("QSQLITE");

    bool create  = false;
    if (QFile(m_databaseLocation).exists() == false) {
        create = true;
    }

    m_database.setDatabaseName(m_databaseLocation);
    bool ok = m_database.open();
    if (!ok)
        qDebug() << "FAIL: could not open database";


    if (create) {
        execQuery("PRAGMA encoding = \"UTF-8\";", true);
        execQuery("CREATE TABLE TableSpec Spec varchar");
        execQuery("DROP TABLE Builds", false);
        execQuery("CREATE TABLE " +m_tableName + " (" + m_tableSpec + ")");
    }
  //      createTables();
*/
}

QSqlTableModel *SingleTableDatabase::sqlTableModel()
{
    QSqlTableModel *model = new QSqlTableModel(0, m_database);
    model->setTable(m_tableName);
    model->select();
    return model;
}

QSqlTableModel *SingleTableDatabase::sqlTableSpecModel()
{
    QSqlTableModel *model = new QSqlTableModel(0, m_database);
    model->setTable("TableSpec");
    model->select();
    return model;
}

QString SingleTableDatabase::selectStringQuery(const QString &queryString)
{
    QSqlQuery query(m_database);
    query.prepare(queryString);
    execQuery(query, true);

    if (query.isActive()) {
        query.next();
        return query.value(0).toString();
    }
    return QString();
}

void SingleTableDatabase::execQuery(QSqlQuery query, bool warnOnFail)
{
    bool ok = query.exec();
    if (!ok && warnOnFail) {
        qDebug() << "sql query exec failure:" << query.lastQuery() << query.lastError().text();
    }
}

QSqlQuery SingleTableDatabase::execQuery(const QString &spec, bool warnOnFail)
{
    QSqlQuery query(m_database);
    bool ok = query.prepare(spec);
    if (!ok && warnOnFail) {
        qDebug()  << "sql query prepare failure: " << query.lastError();
        qDebug()  << "sql query is: " << query.lastQuery();
    }
    execQuery(query, warnOnFail);
    return query;
}
