#include <QtCore>
#include <QtGui>
#include <QtTest>

#include <persistentqueue.h>

class tst_persistentqueue : public QObject
{
Q_OBJECT
private slots:
    void queueing();
};

void tst_persistentqueue::queueing()
{
    // Add items
    {
        PersistentQueue persistentQueue("test", QDir::currentPath() + "/db.sqlite");
        persistentQueue.clear();

        persistentQueue.enqueue("A");
        persistentQueue.enqueue("B");
    }

    // Check out from a different object
    {
        PersistentQueue persistentQueue("test", QDir::currentPath() + "/db.sqlite");

        QCOMPARE(persistentQueue.checkout(), QString("A"));
        QCOMPARE(persistentQueue.checkout(), QString("B"));
        QCOMPARE(persistentQueue.checkout(), QString(""));
    }

    // Check out again. This should give the same items since we
    // didn't complete them.
    {
        PersistentQueue persistentQueue("test", QDir::currentPath() + "/db.sqlite");

        QCOMPARE(persistentQueue.checkout(), QString("A"));
        QCOMPARE(persistentQueue.checkout(), QString("B"));
        QCOMPARE(persistentQueue.checkout(), QString(""));

        persistentQueue.complete("A");

        QCOMPARE(persistentQueue.checkout(), QString(""));
    }

    {
        PersistentQueue persistentQueue("test", QDir::currentPath() + "/db.sqlite");
        QCOMPARE(persistentQueue.checkout(), QString("B"));
        QCOMPARE(persistentQueue.checkout(), QString(""));

        persistentQueue.complete("B");
    }


    // Finally, the queue should be empty:
    {
        PersistentQueue persistentQueue("test", QDir::currentPath() + "/db.sqlite");
        QCOMPARE(persistentQueue.checkout(), QString(""));
    }
}

QTEST_MAIN(tst_persistentqueue);

#include "tst_persistentqueue.moc"
