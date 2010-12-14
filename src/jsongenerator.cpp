#include "jsongenerator.h"
#include "database.h"
#include <json.h>

uint qHash(const QVariant &variant)
{
    return qHash(variant.toString());
}

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

json_object * toJson(qlonglong i)
{
    return json_object_new_int(i);
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
    else if (variant.type() == QVariant::LongLong)
        return toJson(variant.toLongLong());
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


QByteArray JsonGenerator::generateFlatJson(const QString &tableName)
{
    BenchmarkTable table(m_database, tableName);
    return generateFlatJson(&table);
}

QByteArray JsonGenerator::generateFlatJson(BenchmarkTable *benchmarkTable)
{
    QStringList indexDimentions = benchmarkTable->indexDimentions();
    QStringList outputDimentions = benchmarkTable->valueDimentions();

    qDebug() << "## generateFlatJson" << indexDimentions << outputDimentions;

    QByteArray data = generateFlatJson(benchmarkTable->tableName(),
                                       indexDimentions, outputDimentions);

    QByteArray attributes = generateFlatJson(benchmarkTable->attributeTableName(),
                                             QStringList() << "Key", QStringList() << "Value");

    return QString("{ \"data\" : %1 , \"attributes\" : %2 }")
           .arg(QString(data)).arg(QString(attributes)).toUtf8();
}

QByteArray JsonGenerator::generateFlatJson(const QString &tableName, const QStringList &indexDimentions,
                                                                     const QStringList &outputDimentions)
{
    //
    // Format Example:
    //
    // indexColumns : [A, B]
    // dataColumns : [C]
    // indexValues : [[a1, a2], [b1, b2]]
    // data : [
    //         [ 1, 1, data1-a1-b1],
    //         [ 1, 2, data1-a1-b2] ,
    //       ]
    //

    m_tableName = tableName;
    m_columnValues.clear();

    QStringList indexColumns = indexDimentions;
    QStringList dataColumns = outputDimentions;

    // For each index coloumn, find and index all possible values.
    QList<QHash<QVariant, int> > indexValuesList;
    QList<QList<QVariant> > valuesList;
    foreach (const QString &indexColumn, indexColumns) {
        const QList<QVariant> values = lookupDistinctColumnValues(indexColumn);
        valuesList.append(values);

        QHash<QVariant, int> indexValues;
        int valueIndex = 0;
        foreach (const QVariant &value, values) {
            if (indexValues.contains(value) == false) {
                indexValues[value] = valueIndex++;
            }
        }
        indexValuesList.append(indexValues);
    }

    // Stream the indexing part to json
    QMap<QString, json_object *> topLevel;
    topLevel.insert(QLatin1String("indexColumns"), toJson(indexColumns));
    topLevel.insert(QLatin1String("dataColumns"), toJson(dataColumns));
    topLevel.insert(QLatin1String("indexValues"), toJson(valuesList));


    // Stream the (data) rows.
    QString queryString = QString("SELECT %1, %2 FROM %3").arg(
                           Database::scrub(indexColumns.join(",")),
                           Database::scrub(dataColumns.join(",")),
                           Database::scrub(m_tableName));

    //qDebug() << "query" << queryString;

    QList<json_object *> rows;
    QSqlQuery query = m_database->execQuery(queryString, true);

    while (query.next()) {
        QList<json_object *> columns;
        int columnIndex = 0;
        foreach (const QString &index, indexColumns) {
            // look up and stream the numerical index for each column value.
            const QString column = query.value(columnIndex).toString();
            const int i = indexValuesList[columnIndex][column];
            columns.append(toJson(i));
            ++columnIndex;
        }
        foreach (const QString &data, dataColumns) {
            // Stream the data directly
            columns.append(toJson(query.value(columnIndex)));
            ++columnIndex;
        }
        rows.append(toJson(columns));
    }

    topLevel.insert(QLatin1String("dataTable"), toJson(rows));

    QByteArray json = serializeAndFree(toJson(topLevel));

    //QFile test("test");
    //test.open(QIODevice::WriteOnly);
    //test.write(json);

    return json;
}

QList<QVariant> JsonGenerator::lookupDistinctColumnValues(const QString &columnName)
{
    if (m_columnValues.contains(columnName) == false) {
        m_columnValues.insert(columnName, m_database->selectDistinctVariants(columnName, m_tableName));
    }
    return m_columnValues.value(columnName);
}
