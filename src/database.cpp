#include "database.h"
#include <QtCore>
#include <QtSql>
#include <QtGui>

BenchmarkTable::BenchmarkTable(Database *database, const QString &tableName)
{
    m_database = database;
    m_sqlDatabase = database->database();
    m_tableName = tableName;
    m_setValueCalled = false;;
}

BenchmarkTable::~BenchmarkTable()
{

}

void BenchmarkTable::updateBenchmarkTables(const QStringList &indexDimentions, const QStringList &valueDimentions)
{

//    qDebug() << "updateBenchmarkTables";
//    qDebug() << "indexDimentions" << indexDimentions;
//    qDebug() << "valueDimentions" << valueDimentions;
//    qDebug() << "current indexDimentions" << this->indexDimentions();
    //qDebug() << "current valueDimentions" << this->valueDimentions();

    if (m_database->tables().contains("BenchmarkTables") == false) {
        m_database->execQuery("CREATE TABLE BenchmarkTables "
                              "(TableName varchar, IndexDimentions varchar, ValueDimentions varchar)", true);
    }

    if (indexDimentions != this->indexDimentions()) {
        m_database->updateCell("BenchmarkTables", "IndexDimentions", indexDimentions.join(" "), "TableName", m_tableName);
    }

    if (valueDimentions != this->valueDimentions()) {
        //qDebug() << "update values" << valueDimentions;
        m_database->updateCell("BenchmarkTables", "ValueDimentions", valueDimentions.join(" "), "TableName", m_tableName);
    }
}

void BenchmarkTable::setDimention(const QString &name, const QString &value)
{
    if (value.isNull())
        m_indexDimentions[name] = QString(""); // We want to insert "", not NULL into the db.
    else
        m_indexDimentions[name] = value;

    if (m_indexDimentionOrder.contains(name) == false)
        m_indexDimentionOrder.append(name);
}

void BenchmarkTable::clearDimention(const QString &name)
{
    m_indexDimentions.remove(name);
    m_indexDimentionOrder.removeAll(name);
}

void BenchmarkTable::setValue(double value)
{
    prepareValueInsertion("Value", "REAL");
    insertValue("Value", QVariant(value));}

void BenchmarkTable::setValue(const QString &name, const QString &value)
{
    prepareValueInsertion(name, "TEXT");
    insertValue(name, QVariant(value));
}

void BenchmarkTable::setValue(const QString &name, double value)
{
    prepareValueInsertion(name, "REAL");
    insertValue(name, QVariant(value));
}

void BenchmarkTable::prepareValueInsertion(const QString &valueDimentionName, const QString sqlDimentionType)
{
    //   qDebug() << "setValue" << value;
    //   qDebug() << "dimentions" << m_indexDimentions;
    //   qDebug() << "dimention order" << m_indexDimentionOrder;

    if (m_setValueCalled)
        return; // No meta-data / database schema updates neccesary.
    m_setValueCalled = true;

    // Store various meta-information about this table.
    if (m_valueDimentions.keys().contains(valueDimentionName) == false) {
        m_valueDimentions.insert(valueDimentionName, sqlDimentionType);
        m_valueDimentionOrder.append(valueDimentionName);
    }

    updateBenchmarkTables(m_indexDimentionOrder, m_valueDimentionOrder);
   // qDebug() << "updateBenchmarkTables done";

    // The database keeps an record of the schema for all tables.
    // Update it. This also changes the table itself if neccesary.
    // ### replace with PRAGMA table_info(table-name);
    QStringList columnNames = m_indexDimentionOrder + m_valueDimentionOrder;
    QStringList columnTypes;
    foreach (const QString &foo, m_indexDimentionOrder) {
        columnTypes.append("TEXT");
    }
    foreach (const QString &name, m_valueDimentionOrder) {
        columnTypes.append(m_valueDimentions.value(name));
    }

    m_database->updateTableSchema(m_tableName, columnNames, columnTypes);
}

void BenchmarkTable::insertValue(const QString &valueColumnName, const QVariant &value)
{
    QStringList columns = m_indexDimentionOrder;
    columns.append(valueColumnName);

    // qDebug() << "insert in coloumns" << columns;
    QList<QVariant> values;

    foreach (const QString &column, m_indexDimentionOrder) {
        values.append(QVariant(m_indexDimentions.value(column)));
    }

    values.append(value);

    // qDebug() << "values" << values;

    //m_database->insertCell(m_tableName,
    m_database->insertRow(m_tableName, columns, values);
}

QString BenchmarkTable::tableName()
{
    return m_tableName;
}

QStringList BenchmarkTable::indexDimentions()
{
    return m_database->selectCell("BenchmarkTables", "IndexDimentions", "TableName", m_tableName)
            .toString().split(" ", QString::SkipEmptyParts);
}

QStringList BenchmarkTable::valueDimentions()
{
    return m_database->selectCell("BenchmarkTables", "ValueDimentions", "TableName", m_tableName)
            .toString().split(" ", QString::SkipEmptyParts);
}

Database::Database()
{
    m_path = "qtbuildbot.sqlite";
    m_transactionCount = 0;
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

QStringList Database::columns(const QString &tableName)
{
    // ### PRAGMA table_info(table-name);
    QString columns = selectString("SELECT Columns FROM MetaTable WHERE TableName = '" + tableName + "'");
    return columns.split(" ");
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

bool Database::updateTableSchema(const QString &tableName, const QStringList &columnNames, const QStringList &columnTypes)
{
    QString schema = schemaFromColumns(columnNames, columnTypes);

    // qDebug() << "update schema for " << tableName << schema;

    // Meta: the MetaTable contains info about each table:
    // - Columns: ordered list of dimentions/columns.
    if (tables().contains("MetaTable") == false) {
        execQuery("CREATE TABLE MetaTable (TableName varchar, Columns varchar)", true);
    }

    // Create table if it does not exist.
    if (tables().contains(tableName) == false) {
         qDebug() << "create table";
        execQuery("CREATE TABLE " + tableName + " " + schema, true);
        insertRow("MetaTable",
                  QStringList() << "TableName" << "Columns",
                  QList<QVariant>() << tableName << columnNames.join(" "));
    }

    QSet<QString> currentColumns = columns(tableName).toSet();
    QSet<QString> newColumns = columnNames.toSet();

    // qDebug() << "currentColumns" << currentColumns << "newColumns" << newColumns;

    // slip update if there are no changes.
    if (newColumns == currentColumns)
        return true;

    QSet<QString> missingColumns = newColumns - currentColumns;

    foreach(const QString &columnName, missingColumns) {
        QSqlQuery query(database());
        QString columnType = columnTypes.at(columnNames.indexOf(columnName));
        query.prepare("ALTER TABLE" + scrub(tableName) + "ADD" + scrub(columnName) + scrub(columnType));
        execQuery(query, true);
    }

    updateCell("MetaTable", "Columns", columnNames, "TableName", tableName);
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
    QSqlQuery query;
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

    QStringList values;
    while (query.next()) {
        values += query.value(0).toString();
    }
    return values;
}

QStringList Database::selectDistinctWhere(const QString &field, const QString &tableName,
                                const QStringList &whereColumns, const QStringList &whereValues)
{
    QSqlQuery query;
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
    out.replace(";", " ");
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

         qDebug() << "selectMultipleWhere" << key;
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

   // qDebug() << "insertRow" << insertString << insertColumnValues;
    execQuery(query, true);
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
        qDebug() << "updateCell; insert row";
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

    qDebug() << "updateCell: update row" << query.lastQuery();
}

bool Database::databaseExists()
{
    return QFile(m_path).exists();
}

void Database::openDatabase()
{
    //qDebug() << "openDatabase" << m_path << m_database.isValid();

    // This doesn't seem to want to work. I get "multiple connections"
    // errors even when checking for existing connections.
    if (m_database.isValid() == false) {
        if (QSqlDatabase::connectionNames().contains("QSQLITE"))
            m_database = QSqlDatabase::database("QSQLITE");
        else
            m_database = QSqlDatabase::addDatabase("QSQLITE");
    }

    //qDebug() << "openDatabase isValid" << m_database.isValid();

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
    qDebug() << "connectToDatabase";
    m_database.setDatabaseName(m_path);
    bool ok = m_database.open();
    if (!ok)
        qDebug() << "FAIL: could not open database" << m_path;

    executeSqlitePragmas();
}

void Database::createNewDatabase()
{
    qDebug() << "createNewDatabase" << m_path;
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