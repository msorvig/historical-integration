#ifndef UTIL_H
#define UTIL_H

#include <QtCore>

struct ProcessResult{
    ProcessResult() : success(true) {}
    ProcessResult(bool success, QByteArray output)
        : success(success), output(output) {}
    bool success;
    QByteArray output;
};

ProcessResult pipeExecutable(const QString &workdir, const QString &executable, const QStringList &arguments);
void rmrf(const QString &target);

#ifdef Q_OS_WIN
QString programFilesLocation();
#endif


#endif // UTIL_H
