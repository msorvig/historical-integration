#include "attributedtable.h"

AttributedTable::AttributedTable(Database *database, const QString &tableName)
{
    m_database = database;
    m_tableName = tableName;
    m_attributeTableName = tableName + "Attributes";

    // Create the attribute table if necessary.
    if (m_database->tables().contains(m_attributeTableName) == false) {
        const QString queryString =
            QString("CREATE TABLE %1 (Key VARCHAR, Value VARCHAR)")
                    .arg(m_attributeTableName);
        m_database->execQuery(queryString, true);
    }
}

void AttributedTable::setTableScema(const QStringList &columnNames, const QStringList &columnTypes)
{
    m_database->updateTableSchema(m_tableName, columnNames, columnTypes);
}

void AttributedTable::setAttribute(const QString &key, const QString &value)
{
    m_database->insertRow(m_attributeTableName,
                          QStringList() << "Key" << "Value",
                          QList<QVariant>() << key << value);
}

QVariantList AttributedTable::selectList(const QString &query, const QVariantList &values)
{
    return m_database->selectList(insertTableName(query), values);
}

QVariant AttributedTable::select(const QString &query, const QVariantList &values)
{
    return m_database->select(insertTableName(query), values);
}

void AttributedTable::exec(const QString &query, const QVariantList &values)
{
    m_database->execQuery(insertTableName(query), values, true);
}

QString AttributedTable::insertTableName(const QString &query)
{
    QString copy = query;
    return QString(query).replace("%table%", m_tableName);
}

QString AttributedTable::tableName()
{
    return m_tableName;
}

QString AttributedTable::attributeTableName()
{
    return m_attributeTableName;
}

QStringList AttributedTable::columnNames()
{
    return m_database->columnNames(m_tableName);
}

