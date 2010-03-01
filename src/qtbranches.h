#ifndef QTBRANCHES_H
#define QTBRANCHES_H

#include <QtCore>

class GitClient;
class QtBranches
{
public:
    QtBranches(const QString &workdir);
    GitClient *mainBranch();
    QList<GitClient *> releaseBranches();

public: // backend/test API
    void setMainBranch(const QString &path);

};

#endif // QTBRANCHES_H
