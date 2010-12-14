#include <QtCore>
#include <QTest>

#include <testrunner.h>
#include <testmanager.h>
#include <persistentqueue.h>
#include <database.h>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QString qtPath = "c:/depot/qt";

    QStringList arguments = a.arguments();

    Database benchmarkDatabase(QDir::currentPath() + "/results.sqlite");
    BenchmarkTable runtimeResults(&benchmarkDatabase, "testFunctionRuntime");

    PersistentQueue queue("tests", QDir::currentPath() + "/queue.sqlite");

    if (arguments.contains("init")) {
        qDebug() << "init";
        TestManager manager(qtPath);
        QStringList tests = manager.tests();
    
        qDebug() << "tests" << tests.at(0);
                
        queue.clear();
        queue.enqueue(tests);
    }

    if (arguments.contains("run")) {
        QString test = queue.checkout();
        while(test.isEmpty() == false) {

            TestRunner runner(test, qtPath);
            qDebug() << "";
            qDebug() << "Test" << test;

            QString testName = QDir(test).dirName();

            runtimeResults.setDimention("TestCase", testName);

            PersistentQueue testFunctionsQueue("tests" + test , QDir::currentPath() + "/db.sqlite");
            if (testFunctionsQueue.isEmpty()) {
                qDebug() << "init";
                QStringList testFunctions = runner.testFunctions();
                testFunctionsQueue.enqueue(testFunctions);
            }

            QString testFunction = testFunctionsQueue.checkout();
            while (testFunction.isEmpty() == false) {
                qDebug() << "testFunction" << test << ":" << testFunction;
                runtimeResults.setDimention("TestFunction", testFunction);

                int runtime = runner.runTestFunction(testFunction).runTime;
                qDebug() << "runtime" << runtime;
                runtimeResults.setValue("Runtime", runtime);

                testFunctionsQueue.complete(testFunction);
                testFunction = testFunctionsQueue.checkout();
            }

            queue.complete(test);
            test = queue.checkout();
        }
    }
    // return a.exec();
}
