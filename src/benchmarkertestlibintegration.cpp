#include "benchmarkertestlibintegration.h"
#include <QtCore>
#include <QtTest>

#include "webgenerator.h"
#include "jsongenerator.h"
#include "dartabasewalker.h"

const char * databaseName = "benchmarker.sqlite";
const char * rootTableName = "TestCase";

void writeFile(const QString &fileName, const QByteArray &contents);

Q_GLOBAL_STATIC_WITH_ARGS(Database, g_database, (databaseName));
Q_GLOBAL_STATIC_WITH_ARGS(BenchmarkTable, g_testCaseTable, (g_database(), rootTableName));
Q_GLOBAL_STATIC(TestCaseReportGenerator, g_testCaseReportGenerator);
bool reportGeneratorInitilized = false;

static int i = 1;

BenchmarkerTestlibIntegration::BenchmarkerTestlibIntegration()
{
    g_testCaseTable()->setDimention("TestFunction", QString::fromAscii(QTest::currentTestFunction()));
    g_testCaseTable()->setDimention("DataTag", QString::fromAscii(QTest::currentDataTag()));

    g_testCaseTable()->setValue(i++);

    if (reportGeneratorInitilized == false) {
        reportGeneratorInitilized = true;
        qDebug() << "init reportGenerator";
        TestCaseReportGenerator * testCaseReportGenerator = g_testCaseReportGenerator();
        connect(QTest::testObject(), SIGNAL(destroyed()),
                testCaseReportGenerator, SLOT(createReport()));
    }
}

Database *BenchmarkerTestlibIntegration::database()
{
    return g_database();
}

BenchmarkTable *BenchmarkerTestlibIntegration::testCaseTable()
{
    return g_testCaseTable();
}

void TestCaseReportGenerator::createReport()
{
    qDebug() << "create report";
    g_database()->commit();

    JsonGenerator jsonGenerator(g_database());
    QByteArray json = jsonGenerator.generateFlatJson(g_testCaseTable());

    DatabaseWalker walker(g_database());
    walker.printWalk(g_testCaseTable());

    writeFile("report.json", json);

    WebGenerator webGenerator;
    QByteArray report = webGenerator.instantiateSelfContainedDev("singletable", json);
    writeFile("report.html", report);

    // Close the database:
    g_database()->database().close();
    QSqlDatabase::removeDatabase(g_database()->database().connectionName());
    qDebug() << "create report done";
}
