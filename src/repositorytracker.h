#ifndef REPOSITORYTRACKER_H
#define REPOSITORYTRACKER_H

#include <QtCore>

class RepositoryTracker
{
public:
    RepositoryTracker(const QString &basePath);
    void addBuild(const QString &url, const QString &branch);
    void setTestMode(bool enable);

    void maintainBuilds();
    QList<QByteArray> scrapeGitorousHtmlForQtRepositories(const QByteArray &html);
private:
    QString m_basePath;
};

#endif // REPOSITORYTRACKER_H
