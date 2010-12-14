#include <QtCore>
#include <QTest>

#include <benchmarker.h>

QVector<int> createInts(int n)
{
   QVector<int> ints;
   for (int j = 0; j < n; ++j)
        ints.push_back(rand());
   return ints;
}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    Benchmarker benchmarker("Sorting");

    // warmup
    QVector<int> ints = createInts(10);
    qSort(ints);
    qStableSort(ints);

/*
        int startSize = 10000;
        int maxSize =   20000;
        double incFactor = 1;
        double incTerm = 1000;
*/

    int startSize = 2;
    int maxSize =   20;
    double incFactor = 1;
    double incTerm = 1;


/*
        int startSize = 0000;
        int maxSize =   20000;
        double incFactor = 1;
        double incTerm = 2000;
*/


    benchmarker.setBenchmarkTitle("Sorting Algorithms Performance (Less is better)");
    benchmarker.setDimentionTitle("Algorithm", "Sorting algorithm");
    benchmarker.setDimentionTitle("Size", "Vector size");
    benchmarker.setResultTitle("Run Time (CPU Cycles)");

    benchmarker.setDimention("Algorithm", "qSort");
    for (int i = startSize; i < maxSize; i = i * incFactor + incTerm) {
        benchmarker.setDimention("Size", i);
        QVector<int> ints = createInts(i);

        int r1, r2, r3;

        benchmarker.checkPoint();
        qSort(ints);
        r1 = benchmarker.checkPoint();
        //r2 = benchmarker.checkPoint();
        //r3 = benchmarker.checkPoint();
        benchmarker.setResult(r1);
        //benchmarker.setResult(r2);
        //benchmarker.setResult(r3);
    }

    benchmarker.setDimention("Algorithm", "qStableSort");
    for (int i = startSize; i < maxSize; i= i * incFactor + incTerm) {
        benchmarker.setDimention("Size", i);

        QVector<int> ints = createInts(i);

        benchmarker.checkPoint();
        qStableSort(ints);
        benchmarker.checkPointSetResult();
    }
}
