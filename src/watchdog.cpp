#include "watchdog.h"
#include "gitclient.h"

#include <QtCore>

void WatchDog::setRepositoryUrl(const QString &url)
{
    m_repositoryUrl = url;
}

void WatchDog::setBasePath(const QString &path)
{
    m_path = path;
}

void WatchDog::setWatchedExecutable(const QString &executable)
{
    m_executable = executable;
}

void WatchDog::run()
{
    qDebug() << "WatchDog: Synching from" << m_repositoryUrl;
    qDebug() << "WatchDog: Base path" << m_path;
    qDebug() << "WatchedExecutable is:" << m_executable;

    QDir().mkpath(m_path);

    gitClient = GitClient::cloneFromUrl(m_repositoryUrl, m_path);
    ProcessResult result = gitClient->sync();
    qDebug() << result.output;
}

/*
    Launches a program with the given basePath as the working directory.
    Returns whether the program should be re-launched on exit.
*/
bool launchProgram(const QString &basePath, const QString &executable)
{
    QString path = basePath + "" + executable;
    qDebug() << "Watchdog: launch" << path << "on" << QDateTime::currentDateTime().toString();
    QProcess p;
    p.setProcessChannelMode(QProcess::ForwardedChannels);
//    p.setWorkingDirectory(basePath);
    if (QFile::exists(path) == false) {
        qFatal((QString("File not found: ") + path).toLocal8Bit().constData());
    }

    p.start(path);

    if (p.waitForFinished(-1) == false)
        return false; // try again
    if (p.exitStatus() == QProcess::CrashExit)
        return false;

    qDebug() << "Watchdog: exit code" << p.exitCode() << "on" << QDateTime::currentDateTime().toString();

    return (p.exitCode() == 0); // quit on code 0.
}

/*
    syncs path and returns true if any files were updated.
*/
/*
bool p4sync(const QString &path)
{
    qDebug() << "Watchdog: p4 sync ... ";
    QByteArray output = pipeP4sync(path);
    qDebug() << output;
    return !(output.contains("up-to-date"));
}
*/

bool cleanMake(const QString &path)
{
    //qmake(path, "qmake");
    //runMake(path, "clean");
    //runMake(path);
    return true;
}


/*
    cd basePath
    p4 sync ...
    make clean; make
*/
bool syncCompile(const QString &basePath)
{
  /*  if (gitPull(basePath) == false) {
//        cleanMake(basePath);
        qDebug() << runMake(basePath).output;
        return true;
    }

    if (cleanMake(basePath) == false) {
        qDebug() << "Watchdog: make clean; make failed on" << basePath << QDateTime::currentDateTime().toString();
        return false;
    }

    return true;
*/
}

/*
    Sync and compiles in a loop .. or not
*/
void syncCompileLoop(const QString &basePath)
{
//    bool success =
     syncCompile(basePath);
/*
    while (success == false) {
        QTest::qWait(1000 * 60 * 10);
        success = syncCompile(basePath);
    }
*/
}

/*
    Launches a program with the given basePath as the working directory.
    Syncs and recompiles it on crash / exit with non-zero code.
*/
void launchAndGuard(const QString &basePath, const QString &executable)
{
    QString exe = executable;
#ifdef Q_OS_WIN
    exe.append(".exe");
#endif
    bool exit;
    do {
        syncCompileLoop(basePath);
        exit = launchProgram(basePath, exe);
    } while (!exit);
}

