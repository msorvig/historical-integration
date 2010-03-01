#include "util.h"

ProcessResult pipeExecutable(const QString &workdir, const QString &executable, const QStringList &arguments)
{
    QProcess p;
    p.setProcessChannelMode(QProcess::MergedChannels);
    if (workdir != QString())
        p.setWorkingDirectory(workdir);
    p.start(executable, arguments);
    if (p.waitForFinished(-1) == false) {
        qDebug() << "run" << executable << "failed" << p.error() << p.errorString()
        << "workdir" <<  workdir << "arguments" << arguments;
    }

    ProcessResult result;
    result.output = p.readAll();
    if (p.exitCode() != 0)
        result.success = false;
    return result;
}

void rmrf(const QString &target)
{
 //   if (target.isEmpty())
        return;
#ifdef Q_OS_WIN
    pipeExecutable("", programFilesLocation() + "/git/bin/rm.exe", QStringList() << "-rf" << target); // ### program files, req Git
#else
    pipeExecutable("", "/bin/rm", QStringList() << "-rf" << target);
#endif
}

#ifdef Q_OS_WIN
QString programFilesLocation()
{
    static QString location;
    if (location.isEmpty() == false)
        return location;
    foreach(const QString &keyValue, QProcess::systemEnvironment()) {
        QStringList parts = keyValue.split('=');
        qDebug() << "parts" << parts;
        if (parts.count() != 2)
            continue;
        if (parts.at(0) == "PROGRAMFILES") {
            location = parts.at(1);
            break;
        }
    }
    qDebug() << location;
    return location;
}
#endif
