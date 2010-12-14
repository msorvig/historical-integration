#include "persistentqueue.h"
#include <QtCore>

PersistentQueue::PersistentQueue(const QString &name, const QString &databasePath)
{
    m_database = new Database(databasePath);
    m_tableName = Database::scrub("PersistentQueue" + name);
    createTable();
}

PersistentQueue::~PersistentQueue()
{
    delete m_database;
}

void PersistentQueue::clear()
{
    m_database->execQuery("DROP TABLE " + m_tableName, true);


    createTable();
}

bool PersistentQueue::isEmpty()
{
    QSqlQuery query = m_database->execQuery(QString("SELECT COUNT(*) FROM %1").arg(m_tableName), true);
    query.next();
    int count = query.value(0).toInt();
    return (count == 0);
}

void PersistentQueue::createTable()
{
    m_database->updateTableSchema(m_tableName,
                                  QStringList() << "QueueOrder" << "Name",
                                  QStringList() << "INTEGER PRIMARY KEY" << "VARCHAR" );
}

void PersistentQueue::enqueue(const QString &item)
{
    enqueueToDatabase(item);
}

void PersistentQueue::enqueue(const QStringList &items)
{
    m_database->transaction();
    foreach (const QString &item, items) {
        enqueueToDatabase(item);
    }
    m_database->commit();
}

void PersistentQueue::enqueueToDatabase(const QString &item)
{
    qDebug() << "add" << item;
    m_database->insertRow(m_tableName,
                          QStringList() << "Name",
                          QList<QVariant>() << item);
        isEmpty();
}

QString PersistentQueue::checkout()
{
    QMutexLocker lock(&m_queueMutex);
    QSqlQuery query =
        m_database->execQuery(QString("SELECT QueueOrder, Name FROM %1 ORDER BY QueueOrder ASC")
                              .arg(m_tableName), true);
    //qDebug() << "PersistentQueue::checkout";
    while (query.next()) {
    //    qDebug() << "next";
        int queueOrderIndex = query.record().indexOf("QueueOrder");
        int nameIndex = query.record().indexOf("Name");

        int queueOrder = query.value(queueOrderIndex).toInt();
        //qDebug() << "queue order" << queueOrder;

        if (m_checkedOuItems.contains(queueOrder)) {
            continue;
        }
        m_checkedOuItems.insert(queueOrder);

        return query.value(nameIndex).toString();
    }
    return  QString(); // empty;
}

void PersistentQueue::complete(const QString &item)
{
    Q_UNUSED(item);
    m_database->execQuery(QString("DELETE FROM %1 WHERE NAME ='%2'")
                                 .arg(m_tableName).arg(item), true);
}

