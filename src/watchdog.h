#ifndef WATCHDOG_H
#define WATCHDOG_H

#include <QtCore>
#include <gitclient.h>

class WatchDog : public QObject
{
public:
    void setRepositoryUrl(const QString &url);
    void setBasePath(const QString &path);
    void setWatchedExecutable(const QString &executable);
    void run();
private:
    QString m_repositoryUrl;
    QString m_path;
    QString m_executable;
    GitClient *gitClient;
};

void launchAndGuard(const QString &basePath, const QString &executable);

#endif
