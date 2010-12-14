#ifndef PERSISTENTQUEUE_H
#define PERSISTENTQUEUE_H

#include <QtCore>
#include <database.h>

class PersistentQueue : public QObject
{
public:
    PersistentQueue(const QString &name, const QString &databasePath);
    ~PersistentQueue();
    void clear();
    bool isEmpty();
    void enqueue(const QString &item);
    void enqueue(const QStringList &items);
    QString checkout();
    void complete(const QString &item);
protected:
    void enqueueToDatabase(const QString &item);
    void createTable();
private:
    Database *m_database;
    QString m_tableName;
    QSet<int> m_checkedOuItems;
    QMutex m_queueMutex;
};

#endif // PERSISTENTQUEUE_H
