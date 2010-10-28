#include <QtCore/QCoreApplication>
#include <QTest>

#include <testrunner.h>
#include <testmanager.h>
#include <persistentqueue.h>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QString qtPath = "c:/depot/qt";

    QStringList arguments = a.arguments();

    PersistentQueue queue("tests", QDir::currentPath() + "/db.sqlite");

    if (arguments.contains("init")) {
        qDebug() << "init";
        TestManager manager(qtPath);
        QStringList tests = manager.tests();

        queue.clear();
        queue.enqueue(tests);
    }

    if (arguments.contains("run")) {
        QString test = queue.checkout();
        while(test.isEmpty() == false) {

            TestRunner runner(test, qtPath);
            qDebug() << "";
            qDebug() << "Test" << test;
            qDebug() << runner.testFunctions();
            QStringList testFunctions = runner.testFunctions();

            PersistentQueue testFunctionsQueue("tests" + test , QDir::currentPath() + "/db.sqlite");




            //runner.runAllTestFunctions();


            //QTest::qWait(100);


            queue.complete(test);
            test = queue.checkout();
        }
    }
    // return a.exec();
}
