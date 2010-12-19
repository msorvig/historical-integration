#include "database.h"
#include <QtCore>
#include <QtSql>
#include <QtGui>
#include "log.h"


Database::Database()
{
    m_path = "qtbuildbot.sqlite";
    m_transactionCount = 0;
    m_tempTableIndex = 0;
    openDatabase();
}

Database::Database(const QString &path)
{
    m_path = path;
    m_transactionCount = 0;
    openDatabase();
}

Database::~Database()
{
    closeDatabase();
}

QSqlDatabase Database::database()
{
    return m_database;
}


QStringList Database::tables()
{
    return selectStringList("SELECT name FROM sqlite_master "
                            "WHERE type='table' "
                            "ORDER BY name;");
}

QStringList Database::columnNames(const QString &tableName)
{
    // NB: sqlite PRAGMA used here
    QString queryString = QString("PRAGMA table_info(%1);").arg(tableName);

    QSqlQuery query(m_database);
    query.prepare(queryString);
    execQuery(query, true);

    QStringList columnNames;

    while (query.isActive()) {
        if (query.next() == false)
            break;

        columnNames.append(query.value(1).toString());
    }

    return columnNames;
}

QString Database::schema(const QString &tableName)
{
    // NB: sqlite PRAGMA used here
    QString queryString = QString("PRAGMA table_info(%1);").arg(tableName);

    QSqlQuery query(m_database);
    query.prepare(queryString);
    execQuery(query, true);

    QStringList columnNames;
    QStringList columnTypes;

    while (query.isActive()) {
        if (query.next() == false)
            break;

        columnNames.append(query.value(1).toString());
        columnTypes.append(query.value(2).toString());
    }

    return schemaFromColumns(columnNames, columnTypes);
}

void Database::transaction()
{
    ++m_transactionCount;
    if (m_transactionCount == 1)
        m_database.transaction();
}

void Database::commit()
{
    --m_transactionCount;
    if (m_transactionCount == 0)
        m_database.commit();
    if (m_transactionCount < 0)
        m_transactionCount = 0;
}

QString Database::schemaFromColumns(const QStringList &columnNames, const QStringList &columnTypes)
{
    QString schema; // Format: (ColumnName1 TYPE, ColumnName2 TYPE)
    schema.append("(");
    int i = 0;
    foreach (const QString &columnName, columnNames) {
        schema.append(columnName + " " + columnTypes.at(i++));
        schema.append(", ");
    }
    schema.chop(2);

    schema.append(")");
    return schema;
}

bool Database::updateTableSchema(const QString &tableName, const QStringList &a_columnNames, const QStringList &columnTypes)
{
    QString schema = schemaFromColumns(a_columnNames, columnTypes);

    // qDebug() << "existing tables" << tables();

    // qDebug() << "update schema for " << tableName << schema;

    // Meta: the MetaTable contains info about each table:
    // - Columns: ordered list of dimentions/columns.
    if (tables().contains("MetaTable") == false) {
        execQuery("CREATE TABLE MetaTable (TableName varchar, Columns varchar)", true);
    }

    // Create table if it does not exist.
    if (tables().contains(tableName.simplified()) == false) {
        // qDebug() << "create table" << tableName << schema;
        execQuery("CREATE TABLE " + tableName + " " + schema, true);
        insertRow("MetaTable",
                  QStringList() << "TableName" << "Columns",
                  QList<QVariant>() << tableName << a_columnNames.join(" "));
    }

    QSet<QString> currentColumns = columnNames(tableName).toSet();
    QSet<QString> newColumns = a_columnNames.toSet();

    // qDebug() << "currentColumns" << currentColumns << "newColumns" << newColumns;

    // slip update if there are no changes.
    if (newColumns == currentColumns)
        return true;

    QSet<QString> missingColumns = newColumns - currentColumns;

    foreach(const QString &columnName, missingColumns) {
        QSqlQuery query(database());
        QString columnType = columnTypes.at(a_columnNames.indexOf(columnName));
        query.prepare("ALTER TABLE" + scrub(tableName) + "ADD" + scrub(columnName) + scrub(columnType));
        execQuery(query, true);
    }

    updateCell("MetaTable", "Columns", a_columnNames, "TableName", tableName);
    return false;
}

QString Database::createWhereClause(const QStringList &whereColumns)
{
    QString whereClause;
    if (whereColumns.isEmpty())
        return QString();

    whereClause += "WHERE ";
    int i = 0;
    foreach(const QString &whereColumn, whereColumns) {
        whereClause += scrub(whereColumn)
                     + " = :WhereColumnValue" + QString::number(i++)
                     + " AND ";
    }
    whereClause.chop(5);
    return whereClause;
}

void Database::bindWhereValues(QSqlQuery * query, const QStringList &whereValues)
{
    int i = 0;
    foreach(const QString &whereValue, whereValues) {
        query->bindValue(":WhereColumnValue" + QString::number(i++), whereValue);
    }
}

QString Database::selectString(const QString &queryString)
{
    QVariant var = selectVariant(queryString);
    if (var.isValid())
        return var.toString();
    return QString();
}

QVariant Database::select(const QString &queryString, const QVariantList &values)
{
    QVariantList results = selectList(queryString, values);
    if (results.isEmpty())
        return QVariant();
    return results.at(0);
}

QVariantList Database::selectList(const QString &queryString, const QVariantList &values)
{
    if (m_queryCache.contains(queryString) == false) {
        QSqlQuery query(m_database);
        query.prepare(queryString);
        m_queryCache.insert(queryString, query);
    }
    QSqlQuery query = m_queryCache[queryString];

    foreach (const QVariant &value, values) {
       query.addBindValue(value);
    }

    execQuery(query, true);

    QVariantList results;
    while (query.isActive()) {
        if (query.next() == false)
            return results;
        results.append(query.value(0));
    }
    return results;
}


QVariant Database::selectVariant(const  QString &queryString)
{
    QSqlQuery query(m_database);
    query.prepare(queryString);
    execQuery(query, true);

    if (query.isActive()) {
        query.next();
        return query.value(0);
    }
    return QVariant();
}

QStringList Database::selectStringList(const QString &queryString)
{
    QSqlQuery query(m_database);
    query.prepare(queryString);
    execQuery(query, true);

    QStringList strings;
    while (query.isActive()) {
        if (query.next() == false)
            return strings;
        strings.append(query.value(0).toString());
    }
    return strings;
}

QStringList Database::selectDistinct(const QString &field, const QString &tableName)
{
    const QList<QVariant> values = selectDistinctVariants(field, tableName);
    QStringList stringValues;
    foreach (const QVariant value, values) {
        stringValues.append(value.toString());
    }
    return stringValues;
}

QList<QVariant> Database::selectDistinctVariants(const QString &field, const QString &tableName)
{
    QSqlQuery query(m_database);
    QString key = "selectDistinct" + field + tableName;
    if (m_queryCache.contains(key)) {
        query = m_queryCache.value(key);
    } else {
        query.prepare("SELECT DISTINCT " + field +" FROM " + tableName);
    //    m_queryCache.insert(key, query);
    }

    bool ok  = query.exec();
    Q_UNUSED(ok);
//    if (!ok)
//        qDebug() << "select unique ok" << ok;

    QList<QVariant> values;
    while (query.next()) {
        values += query.value(0);
    }
    return values;
}

QStringList Database::selectDistinctWhere(const QString &field, const QString &tableName,
                                const QStringList &whereColumns, const QStringList &whereValues)
{
    QSqlQuery query(m_database);
    QString key = "selectDistinct" + field + tableName + whereColumns.join("");
    if (m_queryCache.contains(key)) {
        query = m_queryCache.value(key);
    } else {
        QString queryString = "SELECT DISTINCT " + scrub(field) +" FROM " + scrub(tableName);
        queryString += createWhereClause(whereColumns);
        query.prepare(queryString);
    //    m_queryCache.insert(key, query);
    }

    bindWhereValues(&query, whereValues);

    bool ok  = query.exec();
    Q_UNUSED(ok);
//    if (!ok)
//        qDebug() << "select unique ok" << ok;

    QStringList values;
    while (query.next()) {
        values += query.value(0).toString();
    }
    return values;
}

void Database::displayTable(const QString &table)
{
    QSqlTableModel *model = new QSqlTableModel();
    model->setTable(table);
    model->select();
    QTableView *view = new QTableView();
    view->setModel(model);
    view->resize(1000, 600);
    view->show();
    view->raise();

    QEventLoop loop;
    loop.exec();
}

// Todo: prevent sql injection attacks here.
QString Database::scrub(const QString &string)
{
    QString out = string;
    out.prepend(" ");
    out.append(" ");
    out.replace(";", "");
    out.replace(":", "");
    out.replace("/", "");
    return out;
}

QVariant Database::selectCell(const QString &tableName,
                              const QString &selectColumnName,
                              const QString &whereColumnName, const QVariant &whereColumnValue)
{
    QSqlQuery query(database());

   // qDebug() << "selectCell" << whereColumnValue;

    query.prepare("SELECT" + scrub(selectColumnName) +
                  "FROM" + scrub(tableName) +
                  "WHERE" + scrub(whereColumnName) + " = :WhereColumnValue");

    query.bindValue(":WhereColumnValue", whereColumnValue);

    execQuery(query, true);

    if (query.isActive()) {
//        qDebug() << "selectCell ran" <<  query.executedQuery();
        if (query.next())
            return query.value(0);
    }

    // qDebug() << "selectCell ran (invalid)" <<  query.executedQuery();

    return QVariant();

}

QList<QVariant> Database::selectMultipleWhere(const QString &tableName, const QStringList &selectColumns,
                                              const QStringList &whereColumns, const QStringList &whereValues)
{
    const QString key = "selectMultipleWhere" + tableName + selectColumns.join("") + whereColumns.join("");

    if (m_queryCache.contains(key) == false) {
        QSqlQuery query(database());
        //QString selectString = foldl(map(selectColumns, [const QString &string]{ return scrub(string)}))
        QString selectString = "SELECT";
        foreach (const QString &selectColumn, selectColumns) {
            selectString.append(scrub(selectColumn));
        }

        selectString += "FROM" + scrub(tableName);
        selectString += createWhereClause(whereColumns);
        query.prepare(selectString);

        m_queryCache.insert(key, query);
    }
    QSqlQuery query = m_queryCache.value(key);

    bindWhereValues(&query, whereValues);

    execQuery(query, true);
    //qDebug() << "selectMutly" << query.lastQuery();

    int valueCount = selectColumns.count();

    QList<QVariant> results;
    if (query.isActive()) {
        if (query.next()) {
            for (int i =0; i < valueCount; ++i) {
                results.append(query.value(i));
            }
        }
    }

    // qDebug() << "selectCell ran (invalid)" <<  query.executedQuery();
    query.finish();

    return results;
}

void Database::insertRow(const QString &tableName,
                         const QStringList &insertColumnNames, const QList<QVariant> &insertColumnValues)
{
    QString key = "insertRow" + insertColumnNames.join("") + tableName;

    if (m_queryCache.contains(key) == false) {
        QSqlQuery query(database());
        QString insertString = "INSERT INTO "+ scrub(tableName);

        insertString += " (";
        for (int i = 0; i < insertColumnNames.count(); ++i) {
            insertString += scrub(insertColumnNames.at(i)) + ", ";
        }
        insertString.chop(2); // remove last ", "
        insertString += ")";

        insertString += " VALUES (";
        for (int i = 0; i < insertColumnNames.count(); ++i) {
            insertString += ":ColumnValue" + QString::number(i) + ", ";
        }
        insertString.chop(2); // remove last ", "
        insertString += ") ";
        query.prepare(insertString);

        m_queryCache.insert(key, query);
    }

    QSqlQuery query = m_queryCache.value(key);

    for (int i = 0; i < insertColumnValues.count(); ++i) {
        query.bindValue(":ColumnValue" + QString::number(i), insertColumnValues.at(i));
    }


    execQuery(query, true);
    //qDebug() << "insertRow" << query.lastQuery() << insertColumnValues;
}

void Database::insertCell(const QString &tableName,
                          const QString &insertColumnName, const QVariant &insertColumnValue)
{
   // qDebug() << "insertCell" << tableName << insertColumnName << insertColumnValue;
    insertRow(tableName, QStringList() << insertColumnName, QList<QVariant>() << insertColumnValue);

}

void Database::updateCell(const QString &tableName,
                          const QString &insertColumnName, const QVariant &insertColumnValue,
                          const QString &whereColumnName, const QVariant &whereColumnValue)

{
    QVariant value = selectCell(tableName, whereColumnName, whereColumnName, whereColumnValue);
    // qDebug() << "updateCell for" << insertColumnName
    //         <<  "    insert in column" << insertColumnName << "value" << insertColumnValue
    //         << "     wehere          " << whereColumnName <<  whereColumnValue
    //         << "fond" << value << value.isValid();

    // add row if missing
    if (value.isValid() == false) {
        //qDebug() << "updateCell; insert row";
        insertRow(tableName, QStringList() << whereColumnName << insertColumnName,
                             QList<QVariant>() << whereColumnValue << insertColumnValue);
        return;
    }


    QSqlQuery query(database());
    query.prepare(" UPDATE" + scrub(tableName) +
                  " SET " + scrub(insertColumnName) + " = :InsertColumnValue "
                  " WHERE" + scrub(whereColumnName) + " = :WhereColumnValue ");

    query.bindValue(":InsertColumnValue", insertColumnValue);
    query.bindValue(":WhereColumnValue", whereColumnValue);

    execQuery(query, true);

    //qDebug() << "updateCell: update row" << query.lastQuery();
}

bool Database::databaseExists()
{
    return QFile(m_path).exists();
}

void Database::openDatabase()
{
   // qDebug() << "openDatabase" << m_path << m_database.isValid() << this;
    QLatin1String databaseConnectionNanme("builbot-sqlite");
    if (m_database.isValid() == false) {
        if (QSqlDatabase::connectionNames().contains(databaseConnectionNanme))
            m_database = QSqlDatabase::database(databaseConnectionNanme);
        else
            m_database = QSqlDatabase::addDatabase("QSQLITE", databaseConnectionNanme);
    }

    //qDebug() << "openDatabase isValid" << m_database.isValid();
    if (m_database.isValid() == false) {
        Log::addError("Could not create SQLITE database connection. Missing driver?");
        exit(0);
    }

    if (m_database.isOpen())
        return;

    if (databaseExists() == false) {
        createNewDatabase();
    } else {
        connectToDatabase();
    }
}

void Database::connectToDatabase()
{
   // qDebug() << "connectToDatabase" << m_path;
    m_database.setDatabaseName(m_path);
    bool ok = m_database.open();
    if (!ok)
        qDebug() << "FAIL: could not open database" << m_path;

    executeSqlitePragmas();
}

void Database::createNewDatabase()
{
    // qDebug() << "createNewDatabase" << m_path;
    m_database.setDatabaseName(m_path);

    QDir().mkpath(QFileInfo(m_path).absolutePath());

    bool ok = m_database.open();
    if (!ok)
        qDebug() << "FAIL: could not create database" << m_path;

    executeSqlitePragmas();
}

void Database::executeSqlitePragmas()
{
    execQuery("PRAGMA encoding = \"UTF-8\";", true);
    execQuery("PRAGMA synchronous = NORMAL;", true);
    execQuery("PRAGMA temp_store = MEMORY;", true);
}

void Database::closeDatabase()
{
    m_database.close();
}

void Database::deleteDatabase()
{
    m_database.close();
    QFile::remove(m_path);
}

// Creates a temp table with the schema from the source table
QString Database::createTempTable(const QString &sourceTableName)
{
    QString tempTableName = "temptable" + QString::number(m_tempTableIndex++);
    QString schema = this->schema(sourceTableName);
    execQuery("CREATE TEMP TABLE " + tempTableName + " " + schema, true);
    return tempTableName;
}

void Database::destroyTempTable(const QString &tableName)
{
    if (tableName.startsWith("temptable") == false) {
        Log::addInfo("Database::destroyTempTable called with non-temp table");
        return;
    }

    QString queryString("DROP TABLE " + tableName);
    execQuery(queryString, true);
}

// Creates a temp table with the contents from the source table
// The contents are filtered by filterQuery, which can be of the
// form "WHERE Foo='bar'" or "ORDER BY Foo LIMIT 400" for example.
QString Database::filterTable(const QString &sourceTableName, const QString &filterQuery)
{
    QString newTableName = createTempTable(sourceTableName);
    QString queryQString = QString("INSERT INTO %1 SELECT * FROM %2 %3")
                         .arg(newTableName).arg(sourceTableName).arg(filterQuery);
    execQuery(queryQString, true);
    return newTableName;
}

void Database::execQuery(QSqlQuery query, bool warnOnFail)
{
    bool ok = query.exec();
    if (!ok && warnOnFail) {
        qDebug() << "sql query exec failure:" << query.lastQuery() << query.lastError().text();
    }
}

QSqlQuery Database::execQuery(const QString &spec, bool warnOnFail)
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

QSqlQuery Database::execQuery(const QString &queryString, QVariantList values, bool warnOnFail)
{
    if (m_queryCache.contains(queryString) == false) {
        QSqlQuery query(m_database);
        query.prepare(queryString);
        m_queryCache.insert(queryString, query);
    }
    QSqlQuery query = m_queryCache[queryString];

    foreach (const QVariant &value, values) {
       query.addBindValue(value);
    }

    execQuery(query, warnOnFail);

    return query;
}
