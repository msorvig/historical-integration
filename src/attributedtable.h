#ifndef ATTRIBUTEDTABLE_H
#define ATTRIBUTEDTABLE_H

#include "database.h"
#include <QtCore>

class AttributedTable
{
public:
    AttributedTable(Database *database, const QString &tableName);
    void setTableScema(const QStringList &columnNames, const QStringList &columnTypes);

    // Known Attributes:
    // Title: Chart/ Table title
    // [Column]Title : Column/Dimention title
    // chartMarkers : chart value markers
    // timeChart : configure for time-series data chart.
    void setAttribute(const QString &key, const QString &value);

    QVariantList selectList(const QString &query, const QVariantList &values);
    QVariant select(const QString &query, const QVariantList &values);
    void exec(const QString &query, const QVariantList &values);
    QString insertTableName(const QString &query);

    QString tableName();
    QString attributeTableName();
    QStringList columnNames();
private:
    Database *m_database;
    QString m_tableName;
    QString m_attributeTableName;
};

#endif // ATTRIBUTEDTABLE_H
