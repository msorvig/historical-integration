#ifndef LOG_H
#define LOG_H

#include <QObject>
#include <QString>

class Log : public QObject
{
public:
    Log();

    static void addError(const QString &message);
    static void addInfo(const QString &message);
};

#endif // LOG_H
