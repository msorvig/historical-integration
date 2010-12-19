#include "benchmarkertable.h"

BenchmarkTable::BenchmarkTable(Database *database, const QString &tableName)
{
    m_database = database;
    m_sqlDatabase = database->database();
    m_tableName = tableName;
    m_attributeTableName = tableName + "Attributes";
    m_setValueCalled = false;
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


    if (m_indexDimentionOrder.contains(name) == false) {
        m_indexDimentionTypes[name] = QLatin1String("TEXT");
        m_indexDimentionOrder.append(name);
    }
}

QString variantToSqlType(const QVariant &value)
{
    if (qstrcmp(value.typeName(), "int") == 0) {
        return QLatin1String("INTEGER");
    } else if (qstrcmp(value.typeName(), "double") == 0) {
        return QLatin1String("REAL");
    } else if (qstrcmp(value.typeName(), "float") == 0) {
        return QLatin1String("REAL");
    }
    return QLatin1String("TEXT");
}


void BenchmarkTable::setDimention(const QString &name, const QVariant &value)
{
   m_indexDimentions[name] = value;

    if (m_indexDimentionOrder.contains(name) == false) {
        m_indexDimentionTypes[name] = variantToSqlType(value);
        m_indexDimentionOrder.append(name);
    }
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

void BenchmarkTable::setAttribute(const QString &key, const QString &value)
{
    // ### merge into AttributedTable
    // Create the attribute table if necessary.
    if (m_database->tables().contains(m_attributeTableName) == false) {
        const QString queryString =
            QString("CREATE TABLE %1 (Key VARCHAR, Value VARCHAR)")
                    .arg(m_attributeTableName);
        m_database->execQuery(queryString, true);
    }

    m_database->insertRow(m_attributeTableName,
                          QStringList() << "Key" << "Value",
                          QList<QVariant>() << key << value);
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
    if (m_valueDimentionTypes.keys().contains(valueDimentionName) == false) {
        m_valueDimentionTypes.insert(valueDimentionName, sqlDimentionType);
        m_valueDimentionOrder.append(valueDimentionName);
    }

    updateBenchmarkTables(m_indexDimentionOrder, m_valueDimentionOrder);
   // qDebug() << "updateBenchmarkTables done";

    // The database keeps an record of the schema for all tables.
    // Update it. This also changes the table itself if neccesary.
    // ### replace with PRAGMA table_info(table-name);
    QStringList columnNames = m_indexDimentionOrder + m_valueDimentionOrder;
    QStringList columnTypes;
    foreach (const QString &name, m_indexDimentionOrder) {
        columnTypes.append(m_indexDimentionTypes.value(name));
    }
    foreach (const QString &name, m_valueDimentionOrder) {
        columnTypes.append(m_valueDimentionTypes.value(name));
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

QString BenchmarkTable::attributeTableName()
{
    return m_attributeTableName;
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

BenchmarkTable BenchmarkTable::filtered(const QString &query)
{
    // filter/copy table data
    QString newTableName = m_database->filterTable(m_tableName, query);
    BenchmarkTable newTable(m_database, newTableName);

    // update/copy meta-data.
    newTable.updateBenchmarkTables(indexDimentions(), valueDimentions());
    // TODO attributes
    //m_database->copyTable(m_attributeTableName, newTable.attributeTableName)

    return newTable;
}
