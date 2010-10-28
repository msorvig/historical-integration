#include "jsongenerator.h"
#include "database.h"
#include <json.h>

QByteArray serializeAndFree(json_object *object)
{
    QByteArray arr(json_object_to_json_string(object));
    // #### json_object_put(object); //free
    return arr;
}

json_object * toJson(json_object *obj)
{
    return obj;
}

json_object * toJson(const QString &string)
{
    return json_object_new_string(string.toLatin1().data());
}

json_object * toJson(int i)
{
    return json_object_new_int(i);
}

bool isNumber(double x)
{
        return (x == x);
}
bool isFiniteNumber(double x)
{
    return true;
    //   return (x <= DBL_MAX && x >= -DBL_MAX);
}

json_object * toJson(double i)
{
// Enable this code to control presicion.
//    QString str = QString::number(34, 'g', 10);
//    return toJson(str);

//  prevent writing "inf" and the like to the json data.
    if (!isNumber(i) || !isFiniteNumber(i))
        return json_object_new_double(0);

    return json_object_new_double(i);
}

json_object * toJson(const QVariant variant)
{
   // qDebug() << variant.typeName();

    if (variant.type() == QVariant::String)
        return toJson(variant.toString());
    else if (variant.type() == QVariant::Double)
        return toJson(variant.toDouble());
    else
        return toJson(variant.toString());
}

json_object * toJson(const QStringList &strings)
{
    json_object *array = json_object_new_array();
    foreach(const QString &string, strings)
        json_object_array_add(array, toJson(string));
    return array;
}

template <typename T>
json_object * toJson(const QList<T> &list)
{
    json_object *array = json_object_new_array();
    foreach(const T &value, list)
        json_object_array_add(array, toJson(value));
    return array;
}

template <typename T>
json_object * toJson(const QMap<QString, T> &hash)
{
    json_object *parent = json_object_new_object();

    QMapIterator<QString, T> i(hash);
    while (i.hasNext()) {
        i.next();
        json_object_object_add(parent, i.key().toLatin1().data(), toJson(i.value()));
    }

    return parent;
}

template <typename T>
json_object * toJson(const QSet<T> &set)
{
    json_object *array = json_object_new_array();

    QSetIterator<T> i(set);
    while (i.hasNext()) {
        json_object_array_add(array, toJson(i.next()));
    }

    return array;
}


JsonGenerator::JsonGenerator(Database *database)
{
    m_database = database;
}

json_object * JsonGenerator::generateDimentionLabels(const QStringList &columns)
{
    return toJson(columns);
}

json_object * JsonGenerator::generateDimentionValues(const QStringList &columns)
{
    QList<QStringList> values;
    foreach (const QString &column, columns) {
        values.append(lookupDistinctColumnValues(column));
    }
    return toJson(values);
}

json_object * JsonGenerator::generateDataRows(const QStringList &indexColumns, int currentIndex,
                                              const QStringList &outputColumns,
                                              const QStringList &whereLabels, const QStringList &whereValues)
{
    QList<json_object *> list;


    //qDebug() << "";
    //qDebug() << "generateDataRows" << currentIndex;
    //qDebug() << "labels" << whereLabels << whereValues;

    if (currentIndex >= indexColumns.count()) {
        QList<QVariant> output = m_database->selectMultipleWhere(m_tableName, outputColumns, whereLabels, whereValues);
        return toJson(output);
    }

    QStringList labels = lookupDistinctColumnValues(indexColumns.at(currentIndex));
    QStringList newWhereLabels = whereLabels;
    newWhereLabels.append(indexColumns.at(currentIndex));
    foreach (const QString &label, labels) {
        QStringList newWhereValues = whereValues;
        newWhereValues.append(label);
        list.append(
            generateDataRows(indexColumns, currentIndex + 1, outputColumns, newWhereLabels, newWhereValues)
            );
    }

    return toJson(list);
}

json_object * JsonGenerator::generateDataTable(const QStringList &indexColumns, const QStringList &outputColumns)
{
    return generateDataRows(indexColumns, 0, outputColumns, QStringList(), QStringList());
}

QByteArray JsonGenerator::generateJson(BenchmarkTable *benchmarkTable)
{
    QStringList indexDimentions = benchmarkTable->indexDimentions();
    QStringList outputDimentions = benchmarkTable->valueDimentions();

    return generateJson(benchmarkTable->tableName(), indexDimentions, outputDimentions);
}


QByteArray JsonGenerator::generateJson(const QString &tableName, const QStringList &indexDimentions,
                                                                 const QStringList &outputDimentions)
{
    m_tableName = tableName;
    m_columnValues.clear();
    QMap<QString, json_object *> topLevel;

    topLevel.insert(QLatin1String("indexLabels"), generateDimentionLabels(indexDimentions));
    topLevel.insert(QLatin1String("indexValues"), generateDimentionValues(indexDimentions));
    topLevel.insert(QLatin1String("outputLabels"), generateDimentionLabels(outputDimentions));
    topLevel.insert(QLatin1String("dataTable"),    generateDataTable(indexDimentions, outputDimentions));

    QByteArray json = serializeAndFree(toJson(topLevel));

    //QFile test("test");
    //test.open(QIODevice::WriteOnly);
    //test.write(json);

    return json;
}

QByteArray JsonGenerator::generateHierarchyJson(BenchmarkTable *benchmarkTable)
{
    QStringList indexDimentions = benchmarkTable->indexDimentions();
    QStringList outputDimentions = benchmarkTable->valueDimentions();

    return generateHierarchyJson(benchmarkTable->tableName(), indexDimentions, outputDimentions);
}

QByteArray JsonGenerator::generateHierarchyJson(const QString &tableName,
                                                const QStringList &indexDimentions,
                                                const QStringList &outputDimentions)
{
    m_tableName = tableName;
    m_columnValues.clear();

    qDebug() << "generateHierarchyJson for" << tableName << indexDimentions << outputDimentions;
    QMap<QString, json_object *> topLevel;

    topLevel.insert(QLatin1String("indexLabels"), generateDimentionLabels(indexDimentions));
    topLevel.insert(QLatin1String("outputLabels"), generateDimentionLabels(outputDimentions));
    topLevel.insert(QLatin1String("dataTable"),    generateHierarchyDataTable(indexDimentions, outputDimentions));

    QByteArray json = serializeAndFree(toJson(topLevel));
    return json;
}

json_object * JsonGenerator::generateHierarchyDataTable(const QStringList &indexColumns, const QStringList &outputColumns)
{
    return generateHierarchyDataRows(indexColumns, 0, outputColumns, QStringList(), QStringList());
}

json_object * JsonGenerator::generateHierarchyDataRows(const QStringList &indexColumns, int currentIndex,
                                                       const QStringList &outputColumns,
                                                       const QStringList &whereLabels, const QStringList &whereValues)
 {
    qDebug() << "";
    qDebug() << "generateDataRows" << currentIndex;
    qDebug() << "columns" << whereLabels << "Values" <<  whereValues;

    if (currentIndex >= indexColumns.count()) {
        qDebug() << "terminate";
        QList<QVariant> output =
                m_database->selectMultipleWhere(m_tableName, outputColumns, whereLabels, whereValues);
        return toJson(output);
    }

    QStringList labels = lookupDistinctColumnValuesWhere(
                indexColumns.at(currentIndex), whereLabels, whereValues);
    QStringList newWhereLabels = whereLabels;

//    qDebug() << "column" << indexColumns.at(currentIndex) << "labels" << labels;
    newWhereLabels.append(indexColumns.at(currentIndex));

    QMap<QString, json_object *> list;
    foreach (const QString &label, labels) {
        QStringList newWhereValues = whereValues;
        newWhereValues.append(label);
  //      qDebug() << "decend" << newWhereLabels << newWhereValues ;
        list[label] =
                generateHierarchyDataRows(indexColumns, currentIndex + 1, outputColumns, newWhereLabels, newWhereValues);
    }
    return toJson(list);
}

QStringList JsonGenerator::lookupDistinctColumnValues(const QString &columnName)
{
    if (m_columnValues.contains(columnName) == false) {
        m_columnValues.insert(columnName, m_database->selectDistinct(columnName, m_tableName));
    }
    return m_columnValues.value(columnName);
}

QStringList JsonGenerator::lookupDistinctColumnValuesWhere(const QString &columnName,
                                                           const QStringList &whereLabels,
                                                           const QStringList &whereValues)
{
    return m_database->selectDistinctWhere(columnName, m_tableName, whereLabels, whereValues);
}
