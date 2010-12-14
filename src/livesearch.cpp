#include "livesearch.h"
#include "jsongenerator.h"

void writeFile(const QString &fileName, const QByteArray &contents);

// Expected table schema:
// Key TEXT, Value TEXT
//
// The table get split up into files by the first
// two letters in the key: "aa", "ab", "ac", .. "zz"
// Value is optional and can be an empty string. The file
// format is json, as serialized by the JsonGenerator

LiveSearch::LiveSearch(Database *database, const QString &tableName)
{
    m_database = database;
    m_tableName = tableName;
}

void LiveSearch::generateIndexFiles(const QString &path)
{
    QString canonicalPath = QDir(path).absolutePath();
    QDir().mkpath(canonicalPath);

    qDebug() << "writing index files at:" << canonicalPath;


    JsonGenerator jsonGenerator(m_database);

    for (int i = 'a'; i <= 'z'; ++i) {
        for (int j = 'a'; j <= 'z'; ++j) {
        QString keyStart(QChar::fromAscii(i));
        keyStart.append(QChar::fromAscii(j));

        m_database->execQuery("CREATE TEMP TABLE TempLiveSearch (Key TEXT, Value TEXT)", true);
        m_database->execQuery("INSERT INTO TempLiveSearch (Key, Value) "
                              "SELECT * FROM" + m_database->scrub(m_tableName) +
                              "WHERE Key LIKE '" + keyStart +"%'", true);

        int count = m_database->selectVariant("SELECT COUNT(*) FROM TempLiveSearch").toInt();
        if (count) {
           // m_database->displayTable("TempLiveSearch");
            qDebug() << "data at" << keyStart << count;
            QByteArray json = jsonGenerator.generateFlatJson("TempLiveSearch", QStringList() << "Key" << "Value");
            writeFile(canonicalPath + "/" + keyStart, json);
        }

        m_database->execQuery("DROP TABLE TempLiveSearch", true);
        }  // for j
    } // for i
}
