#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include <QtCore>

class TaskManager;
class Task : public QObject
{
Q_OBJECT
public:
    TaskManager *taskManager;
    virtual void start() = 0;
public slots:
    virtual void done() = 0;
    void log(const QByteArray &message){};
};

class TestTask
{
public:
    TaskManager *taskManager;
    void start();
};

class ProcessTask
{
public:
    QProcess *process;
    void start();
    void done();
};

class TaskManager : public QObject
{
public:
    TaskManager();
    void appendTask(Task *task);

private:
    QQueue<Task *> tasks;
    bool isRunning;
};

#endif // TASKMANAGER_H
