#include <QtCore>
#include <QtGui>
#include <QtTest>
#include "database.h"
#include "jsongenerator.h"
#include "webgenerator.h"
#include "reports.h"
#include "livesearch.h"

class tst_benchmarker : public QObject
{
Q_OBJECT
private slots:
    void createDatabase();
    void addData();
    void multipleValues();
    void tableToJson();
    void webtable();
    void nestedTables();
    void search();
};

void tst_benchmarker::createDatabase()
{
    qDebug() << "Hello";

    {
        Database test("test.sqlite");
    }

    {
        Database test("test.sqlite");
        test.deleteDatabase();
    }
}

void tst_benchmarker::addData()
{
    Database test("test.sqlite");
    qDebug() << test.tables();

    BenchmarkTable multiplication(&test, "multiplication");
    qDebug() << test.tables();

    for (int i = 0; i < 20; ++i) {
        multiplication.setDimention("Factor2", QString::number(i));
        for (int j = 0; j < 20; ++j) {
            multiplication.setDimention("Factor1", QString::number(j));
            multiplication.setValue(i * j);
        }
    }

    qDebug() << test.tables();
    test.displayTable("multiplication");
    //test.displayTable("BenchmarkTables");
}

void tst_benchmarker::multipleValues()
{
    Database test("test.sqlite");

    BenchmarkTable maths(&test, "maths");

    for (int i = 1; i < 2; ++i) {
        maths.setDimention("Factor2", QString::number(i));
        for (int j = 1; j < 2; ++j) {
            maths.setDimention("Factor1", QString::number(j));
            maths.setValue("multiplication", i * j);
            maths.setValue("division", i / j);
            maths.setValue("concatenation", QString::number(i) + QString::number(j));
        }
    }
    //test.displayTable("MetaTable");
    //test.displayTable("BenchmarkTables");
    test.displayTable("maths");
}


void tst_benchmarker::tableToJson(){
    Database test("test.sqlite");

    BenchmarkTable multiplication(&test, "multiplication2");

    for (int i = 1; i < 20; ++i) {
        multiplication.setDimention("Factor2", QString::number(i));
        for (int j = 1; j < 20; ++j) {
            multiplication.setDimention("Factor1", QString::number(j));
            multiplication.setValue(i * j);
        }
    }

    // test.displayTable("multiplication2");


    JsonGenerator jsonGenerator(&test);
    QByteArray json = jsonGenerator.generateJson(&multiplication);

    qDebug() << "Json:" << json;
    qDebug() << "";
}

void tst_benchmarker::webtable()
{
    Database test("test.sqlite");

    BenchmarkTable multiplication(&test, "multiplication2");

    test.database().transaction();

    for (int i = 1; i < 20; ++i) {
        multiplication.setDimention("Factor2", QString::number(i));
        for (int j = 1; j < 20; ++j) {
            multiplication.setDimention("Factor1", QString::number(j));
            multiplication.setValue(i * j);
        }
    }

    test.database().commit();

    // test.displayTable("multiplication2");

    JsonGenerator jsonGenerator(&test);
    QByteArray json = jsonGenerator.generateJson(&multiplication);

    WebGenerator webGenerator;
    QByteArray tabelHtml = webGenerator.instantiateSelfContainedDev("table.html", json, "../../src");
    writeFile("table.html" , tabelHtml);
}

void tst_benchmarker::nestedTables()
{
/*
    Database test("test.sqlite");

    BenchmarkTable tests(&test, "tests");
    for (int i = 0; i < 5; ++i) {
        tests.setDimention("TestCase", "TestCase" + QString::number(i));
        BenchmarkTable multiplication(&test);
        for (int i = 0; i < 20; ++i) {
            multiplication.setDimention("Factor2", QString::number(i));
            for (int j = 0; j < 20; ++j) {
                multiplication.setDimention("Factor1", QString::number(j));
                multiplication.setValue(i * j);
            }
        }
    }
*/
}

void tst_benchmarker::search()
{
    Database test("search.sqlite");

    QFile wordlist("english.0");
    wordlist.open(QIODevice::ReadOnly);
    QByteArray fileContents = wordlist.readAll();
    QList<QByteArray> words = fileContents.split('\n');
    qDebug() << "wordcount" << words.count();

    BenchmarkTable search(&test, "search");
    test.database().transaction();

    //for (int i =0; i < 20; ++i) {
    //    QByteArray word = words.at(i);
    foreach (const QByteArray &word, words) {
        search.setDimention("Key", QLatin1String(word.simplified()));
        search.setValue("Value", QLatin1String(word.simplified()));
    }
    test.database().commit();

    LiveSearch liveSearch(&test, "search");
    liveSearch.generateIndexFiles("./searchindex/");

    WebGenerator webGenerator;
    QByteArray tabelHtml = webGenerator.instantiateSelfContainedDev("search.html", QByteArray(), "../../src");
    writeFile("search.html" , tabelHtml);
    test.deleteDatabase();
}

QTEST_MAIN(tst_benchmarker);

#include "tst_benchmarker.moc"
