#include <QtCore>

struct ProcessResult{
    ProcessResult() {}
    ProcessResult(bool success, QByteArray output) 
        : success(success), output(output) {}
    bool success;
    QByteArray output;
};

ProcessResult runProcess(const QString &executable, const QStringList &arguments, const QString workingDir = QString());

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


class HistoricalBuildOptions
{
public:
    QString basePath;
    QString stagePath;
    QString sourcePath;
    bool storeFullCopy;
    int commitCount;
    bool dryRun;
};

void buildHistorical(HistoricalBuildOptions options);
