#ifndef GITCLIENT_H
#define GITCLIENT_H

#include "util.h"

class VCSClient
{
public:
    virtual ~VCSClient(){};
    virtual ProcessResult sync() = 0;
    virtual QStringList revisions() = 0;
    virtual ProcessResult syncToRevision(const QString &revision) = 0;
};

class GitClient : public VCSClient
{
public:
    static GitClient *cloneFromLocalPath(const QString &sourceUrl, const QString &workDir);
    static GitClient *cloneFromUrl(const QString &sourcePath, QString workPath);

    GitClient(const QString &repositoryPath = QString());

    ProcessResult clone();
    ProcessResult sync();
    ProcessResult syncBack(int revisions);
    ProcessResult checkout(const QString &branch);
    QStringList revisions();
    QString currentRevision();
    ProcessResult syncToRevision(const QString &revision);
    QString projectPath() { return m_projectPath; }
    void setMockRevisionSync(bool enable);
//private:
    QString m_sourceUrl;

    // Given "C:\builds\foo_project", then:
    QString m_projectName; // "foo_project"
    QString m_projectPath; // "C:\builds\foo_project"
    QString m_workPath;    // "C:\builds\"

    QStringList m_revisions;
    bool m_mockRevisionSync;
    QString m_mockedRevision;
};

#endif // GITCLIENT_H
