#include <QtCore>
#include <QtSql>

#include "util.h"
#include "gitclient.h"
#include "builddatabase.h"

ProcessResult runProcess(const QString &executable, const QStringList &arguments, const QString workingDir = QString());
QList<QByteArray> findCommits(const QString &path);

class QtSource
{
public:
    QtSource() {};
    QtSource(const QString &sourcePath, const QString &version = QString(" "))
    : path(sourcePath), version(version) { }


    QString path;
    QString version;
};

class QtDepotSource : public QtSource
{
public:
    QtDepotSource(QString depotPath, QString version = QString(" "));
    bool p4sync(QDateTime date = QDateTime::currentDateTime());
    bool p4sync(int change);
    void gitCheckout(const QString &change);
};

class QtBuild
{
public:
    QtBuild(const QtSource &source, const QString &buildPath)
    : source(source), buildPath(buildPath) {}

    QtBuild(const QtSource &source)
    : source(source), buildPath(source.path) {}

    QStringList supportedConfigureOptions();
    ProcessResult configure();
    ProcessResult configure(const QStringList &options);

    ProcessResult syncqt();

    ProcessResult make();
    ProcessResult make(const QStringList &arguments);

    void removeBuild();
    void removeNonessentialFiles();
    bool isValid();

    QtSource source;
    QString buildPath;
    QStringList configureOptions;
    QStringList makeArguments;
    static QStringList nonessentialFiles;
};


class ProjectBuilder
{
public:
    virtual ~ProjectBuilder();
    virtual ProcessResult buildProject(const QString &sourcePath, const QString &buildPath);
};

class QmakeProjectBuilder
{
public:
    virtual ProcessResult buildProject(const QString &sourcePath, const QString &buildPath);
};

class ProjectHistoryBuilder
{
public:
    ProjectHistoryBuilder(const QString &sourceUrl, const QString &basePath);
    QString basePath;
    QString workPath;
    QString sourceUrl;

    bool useShadowBuild;
    bool storeFullCopy;
    int commitCount;
    bool dryRun;

    enum ProjectType { QmakeBased, Qt };
    ProjectType projectType;

    QString projectRoot;

    GitClient *gitClient; // git-only for now.
    ProjectBuilder *projectBuilder;
    BuildDatabase buildDatabase;

    void build(int revisionCount);
    void buildHistory(int revisionCount);
private:
    QStringList revisions;
};



