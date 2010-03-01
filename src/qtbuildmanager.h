#ifndef QTBUILDMANAGER_H
#define QTBUILDMANAGER_H

#include <gitclient.h>
#include "builddatabase.h"
#include <QtCore>


class QtBuildManager
{
public:
    QtBuildManager();

    void setGitClient(GitClient *gitClient);
    void setWorkDirectory(const QString &workDirectory);

    void setHistoricalBuildTargetCount(int count); // actively build the last count revisions.
    void setHistoricalBuildStorageLimit(int count); // store this amount of builds.

    void setTestMode(bool enable);
    void maintainBuilds(int maxBuildCountForCall);


private:
    BuildDatabase *m_buildDatabase;
    bool m_testMode;
};

#endif // QTBUILDMANAGER_H
