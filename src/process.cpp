#include "process.h"
#include <QtCore>

Process::Process()
{

}

void Process::setWatchFiles(bool watchFiles)
{

}

void Process::addEnvironmentPath(const QString &path)
{
    QStringList env = QProcess::systemEnvironment();
    env.replaceInStrings(QRegExp("^PATH=(.*)", Qt::CaseInsensitive),
                         "PATH=" + path + ";\\1");
    this->setEnvironment(env);
}

