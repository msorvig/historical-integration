#include "log.h"
#include <QtCore>

Log::Log()
{
}

void Log::addError(const QString &message)
{
    qDebug() << "Error: " << message;
}

void Log::addInfo(const QString &message)
{
    qDebug() << "Log: " << message;
}

