#include <QtCore>

struct ProcessResult{
    ProcessResult() {}
    ProcessResult(bool success, QByteArray output) 
        : success(success), output(output) {}
    bool success;
    QByteArray output;
};

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

class VCSClient
{
public:
    virtual ProcessResult sync() = 0;
    virtual QStringList revisions() = 0;
    virtual ProcessResult syncToRevision(const QString &revision) = 0;
};

class GitClient : public VCSClient
{
public:
    GitClient (const QString &sourceUrl);
    ProcessResult sync();
    QStringList revisions();
    ProcessResult syncToRevision(const QString &revision);
    QString projectPath() { return m_projectPath; }
private:
    QString m_sourceUrl;
    QString m_projectName;
    QString m_projectPath;
    QStringList m_revisions;
};

class ProjectBuilder
{
public:
    ~ProjectBuilder();
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
    ProjectHistoryBuilder();
    QString workPath;
    QString sourceUrl;

    bool storeFullCopy;
    int commitCount;
    bool dryRun;

    enum ProjectType { QmakeBased, Qt };
    ProjectType projectType;

    QString projectRoot;

    GitClient *vcsClient; // git-only for now.
    ProjectBuilder *projectBuilder;

    void build();
private:
    void buildHistory();
    QStringList revisions;
};



//void buildHistorical(HistoricalBuildOptions options);

class Visitor
{
public:
    virtual ~Visitor() {};
    enum VisitResponse { Pass, Fail, Skip };
    virtual VisitResponse visit(const QString &) { return Pass; };
    virtual QString firstSha1() = 0;
    virtual QString nextSha1() = 0;

    bool isBuilt(const QString &sha1);
    void stage(const QString &sha1);

    void performVisit(const QList<QByteArray> &commits);

    QList<QByteArray> m_commits;
    QList<QByteArray> m_skipped;
};

class LinearSearchVisitor : public Visitor
{
public:
    QString firstSha1()
    {
        index = 0;
        return m_commits.at(index);
    }

    QString nextSha1()
    {
        ++index;
        if (index > m_commits.count())
            return QString();
        return m_commits.at(index);
    }

    int index;
};


