#include "buildbot.h"
#include <QtSql>
#include <QtGui>

// -----------

QString buildsTable = QString("(Revision varchar, Status varchar)");

void execQuery(QSqlQuery query, bool warnOnFail)
{
    bool ok = query.exec();
    if (!ok && warnOnFail) {
        qDebug() << "sql query exec failure:" << query.lastQuery() << query.lastError().text();
    }
}

QSqlQuery execQuery(const QString &spec, bool warnOnFail = false);
QSqlQuery execQuery(const QString &spec, bool warnOnFail)
{
    QSqlQuery query;
    bool ok = query.prepare(spec);
    if (!ok && warnOnFail) {
        qDebug()  << "sql query prepare failure: " << query.lastError();
        qDebug()  << "sql query is: " << query.lastQuery();
    }
    execQuery(query, warnOnFail);
    return query;
}


void createTables()
{
    execQuery("PRAGMA encoding = \"UTF-8\";", true);
    execQuery("DROP TABLE Builds", false);
    execQuery("CREATE TABLE Builds" + buildsTable);
//    execQuery("CREATE INDEX testcasetestfunction_index ON Results (TestCase, Testfunction)");
}


/*
    Opens a sqlite database, preserves data.
    Creates a new datatbase if the current one
    does not exist.
*/

bool isOpen = false;
QSqlDatabase openDatabase(const QString &databaseFile)
{
    if (isOpen)
        return QSqlDatabase::database();
    isOpen = true;


    qDebug() << "open database";
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    bool create  = false;
    if (QFile(databaseFile).exists() == false) {
        create = true;
    }

    db.setDatabaseName(databaseFile);
    bool ok = db.open();
    if (!ok)
        qDebug() << "FAIL: could not open database";

    if (create)
        createTables();

    return db;
}

void closeDatabase()
{
    qDebug() << "close database";
    isOpen = false;
    QSqlDatabase::database().close();
    QSqlDatabase::removeDatabase("qt_sql_default_connection");
}

/*
    Creates a sqlite database, erases data
*/
QSqlDatabase createDatabase(const QString &databaseFile)
{
    qDebug() << "create data base";
    QSqlDatabase db = openDatabase(databaseFile);
    createTables();

    return db;
}

void setBuildStatus(const QString &revision, const QString &status)
{
    qDebug() << "setBuildStatus" << revision << status;

    QSqlQuery query;

    query.prepare("INSERT INTO Builds (Revision, Status) VALUES (:Revision, :Status)");

    query.bindValue(":Revision", revision);
    query.bindValue(":Status", status);
    execQuery(query, true);
}

QString getBuildStatus(const QString &revision)
{
    QSqlQuery query;
    query.prepare("SELECT Status FROM Builds WHERE Revision='" + revision + "'");
    bool ok = query.exec();
    if (!ok) {
        qDebug() << "sql query exec failure:" << query.lastQuery() << query.lastError().text();
    }

    if (query.isActive()) {
        query.next();
        return query.value(0).toString();
    }
    return QByteArray();
}


void displayDatabaseTable(const QString &table)
{
    QSqlTableModel *model = new QSqlTableModel();
    model->setTable(table);
    model->select();
    QTableView *view = new QTableView();
    view->setModel(model);
    view->resize(1000, 600);
    view->show();
    view->raise();

    QEventLoop loop;
    loop.exec();
}

//-------------

// global options
QString basePath;
QString stagePath;
QString sourcePath;
bool storeFullCopy;
bool dryRun;

// forwards
void storeAndLinkBuild(const QString &storagePath, const QString &buildPath, const QString &previousStoragePath);


ProcessResult runProcess(const QString &executable, const QStringList &arguments, const QString workingDir)
{
    QProcess process;
    process.setReadChannelMode(QProcess::MergedChannels);

    if (workingDir != QString())
        process.setWorkingDirectory(workingDir);

    process.start(executable, arguments);
    process.waitForFinished(-1);
    
    return ProcessResult(process.exitCode() == 0, process.readAll());
}

ProcessResult pipeExecutable(const QString &workdir, const QString &executable, const QStringList &arguments)
{
    QProcess p;
    p.setProcessChannelMode(QProcess::MergedChannels);
    if (workdir != QString())
        p.setWorkingDirectory(workdir);
    p.start(executable, arguments);
    if (p.waitForFinished(-1) == false) {
        qDebug() << "run" << executable << "failed" << p.error();
    }

    ProcessResult result;
    result.output = p.readAll();
    if (p.exitCode() != 0)
        result.success = false;
    return result;
}

void writeFile(const QString &fileName, const QByteArray &contents)
{
    QFile f(fileName);
    f.open(QIODevice::WriteOnly | QIODevice::Append);
    f.write(contents);
}

ProcessResult gitCommits(const QString &path)
{
    const QString arguments =  "--no-pager log --pretty=oneline";
    return pipeExecutable(path, "git", arguments.split(" "));
}

ProcessResult gitCeckout(const QString &path, const QString &sha1)
{
    QString arguments = "checkout " + sha1;
    return pipeExecutable(path, "git", arguments.split(" "));
}

ProcessResult gitClone(const QString destinationPath, const QString &sourceUrl)
{
    QString arguments = "clone " + sourceUrl;
    return pipeExecutable(destinationPath, "git", arguments.split(" "));
}

ProcessResult gitPull(const QString path)
{
    QString arguments = "pull";

    ProcessResult result = pipeExecutable(path, "git", arguments.split(" "));
    if (result.output.contains("Already up-to-date")) // Git reports this as an error,
        result.success = true;                        // but this is really a normal condition.
    return result;
}

ProcessResult gitStatus(const QString path, const QString &sourceUrl)
{
    QString arguments = "status";
    return pipeExecutable(path, "git", arguments.split(" "));
}


QList<QByteArray> findCommits(const QString &path)
{
    QList<QByteArray> commits;
    QByteArray rawChanges = gitCommits(path).output;
    gitCeckout(path, "master"); // ### assumes Qt mainline
    qDebug() << "searching for changes in" << path;
    foreach (QByteArray line, rawChanges.split('\n')) {
        commits += line.split(' ').at(0);
    }
    return commits;
}



QtDepotSource::QtDepotSource(QString depotPath, QString version)
:QtSource(depotPath, version)
{

}    

bool QtDepotSource::p4sync(QDateTime date)
{
    const QString dateString = date.date().toString("yyyy/MM/dd");
    const QString syncString = path + "/...@" + dateString;
    const QStringList arguments = QStringList() << "sync" << syncString;

    qDebug() << "p4" << arguments;

    return runProcess("p4", arguments).success;
}

bool QtDepotSource::p4sync(int change)
{
    const QString syncString = path + "/...@" + QString::number(change);
    const QStringList arguments = QStringList() << "sync" << syncString;

    qDebug() << "p4" << arguments;

    return runProcess("p4", arguments).success;
}

void QtDepotSource::gitCheckout(const QString &sha1)
{
    QString arguments = "checkout " + sha1;
    qDebug() << this->path << arguments;
    qDebug() << pipeExecutable(this->path, "git", arguments.split(" ")).output;
}

////////////////////////

ProcessResult QtBuild::configure(const QStringList &options)
{
    configureOptions = options;
    return configure();
}

/*
    Returns some of the options configure supports
*/
QStringList QtBuild::supportedConfigureOptions()
{
    const QString str = QString().fromLocal8Bit(configure(QStringList() << "-help").output);
//    qDebug() << str;
    QRegExp rx("\\[(-[-|\\w+]+)\\]"); // pull out substrings between [] that contains a '-'
    QStringList list;
    int pos = 0;
    while ((pos = rx.indexIn(str, pos)) != -1) {
        const QString opt = rx.cap(1);

        // Leave out certain options (they require a parameter, or are special).
        if ( (opt.contains("string") == false) && (opt.contains("pch") == false) 
         && (opt.contains("release") == false) && (opt.contains("debug") == false)) {
            list << opt; 
        }
        pos += rx.matchedLength();
    }
        
    return list;
}

ProcessResult QtBuild::configure()
{
    return runProcess(source.path + "/configure", configureOptions, buildPath);
}

ProcessResult QtBuild::syncqt()
{
    return runProcess(buildPath + "/bin/syncqt", QStringList());
}

ProcessResult QtBuild::make(const QStringList &arguments)
{
    makeArguments = arguments;
    return make();
}

ProcessResult QtBuild::make()
{
    return runProcess("/usr/bin/make", makeArguments, buildPath);
}


void QtBuild::removeBuild()
{
    const QString toDelete = buildPath;

    qDebug() << "deleting" << toDelete;
    //sleep(4);

    runProcess("/bin/rm", QStringList() << "-rf" << toDelete, buildPath);

    QDir().mkpath(buildPath);
}
QStringList QtBuild::nonessentialFiles = QStringList() <<  "/src" << "/demos" << "/doc" << "/examples" << "/tests" << "/tools";
void QtBuild::removeNonessentialFiles()
{
    QStringList toDelete;
    toDelete.append(buildPath + "/src");
    toDelete.append(buildPath + "/demos");
    toDelete.append(buildPath + "/doc");
    toDelete.append(buildPath + "/examples");
    toDelete.append(buildPath + "/tests");
    toDelete.append(buildPath + "/tools");

    qDebug() << "deleting" << toDelete;
//    sleep(4);

    QStringList args;
    args.append("-rf");
    args+=toDelete;
    
    runProcess("/bin/rm", args, buildPath);

    QDir().mkpath(buildPath);
}

bool QtBuild::isValid()
{
    return QFile::exists(source.path) && QFile::exists(buildPath);
}

void hardLink(const QString &target, const QString link)
{
    pipeExecutable("", "/bin/ln", QStringList() << target << link);
}

void symLink(const QString &target,  QString link)
{
 //   qDebug() << "symlink" << target << link;
    if (link.endsWith('/'))
        link.chop(1);
#ifdef Q_OS_WIN
    pipeExecutable("", "c:/Programfiler/git/bin/ln.exe", QStringList() << "-s" << target << link); // ### program files, req Git
#else
    pipeExecutable("", "/bin/ln", QStringList() << "-s" << target << link);
#endif
}


void move(const QString &source, const QString &target)
{
#ifdef Q_OS_WIN
    pipeExecutable("", "c:/Programfiler/git/bin/mv.exe", QStringList() << source << target); // ### program files, req Git
#else
    pipeExecutable("", "/bin/mv", QStringList() << source << target);
#endif
}

void rmrf(const QString &target)
{
    if (target.isEmpty())
        return;
#ifdef Q_OS_WIN
    pipeExecutable("", "c:/Programfiler/git/bin/rm.exe", QStringList() << "-rf" << target); // ### program files, req Git
#else
    pipeExecutable("", "/bin/rm", QStringList() << "-rf" << target);
#endif
}

void copyPreserveDate(const QString &source, const QString &target)
{
    pipeExecutable("", "/bin/cp", QStringList() << "-p" << source << target);
}

void resetBuildStatus(QString buildDir)
{
    QDir buildPath(buildDir);
    buildPath.rmdir("OK");
    buildPath.rmdir("FAILED");
}

bool buildQt(QtBuild build)
{
    ProcessResult result;
    qDebug() << "make sub-src";
    result = build.make(QStringList() << "sub-src");
    if (result.success == false) {
        qDebug() << "make failed";
        qDebug() << result.output;
            writeFile(build.buildPath + "/log", result.output);

        // handle common errors:
        if (result.output.contains("one or more PCH files were found, but they were invalid")) {
            qDebug() << "invalid pch detected, make clean and amke again";
            build.make(QStringList() << "clean");
            result = build.make(QStringList() << "sub-src");
            if (result.success == false) {
                qDebug() << "make again after invalid pch failed";
                qDebug() << result.output;
                        writeFile(build.buildPath + "/log", result.output);
            } else{
               QDir(build.buildPath).mkdir("FAILED");
               return false;
            }
       } else if (result.output.contains("No rule to make target")) {
            QString configString = "-fast -pch -optimized-qmake -release -no-webkit -no-xmlpatterns -nomake examples -nomake demos -nomake tests";
            qDebug() << "configure" << configString;
            result = build.configure(configString.split(' '));
            if (result.success == false) {
                qDebug() << "configure failed";
                qDebug() << result.output;
                writeFile(build.buildPath + "log", result.output);
                QDir(build.buildPath).mkdir("FAILED");
                return false;
             }
            qDebug() << "make sub-src";
            result = build.make(QStringList() << "sub-src");
            if (result.success == false) {
                qDebug() << "make again after configure failed";
                qDebug() << result.output;
                writeFile(build.buildPath + "/log", result.output);
            } else{
               QDir(build.buildPath).mkdir("FAILED");
               return false;
            }
       } else {
            QDir(build.buildPath).mkdir("FAILED");
            return false;
        }
    }

    qDebug() << "make test lib";
    result = build.make(QStringList() << "-f" << "tools/qtestlib/Makefile");
    if (result.success == false) {
        qDebug() << "dirName";
        qDebug() << "make failed";
        qDebug() << result.output;
                writeFile(build.buildPath + "/log", result.output);

        QDir(build.buildPath).mkdir("FAILED");
        return false;
    }
    return true;
}

bool syncBuild(QString change, QDir buildPath, QtDepotSource source, QtBuild build)
{
    source.gitCheckout(change);
    build.syncqt();

    return buildQt(build);
}

bool incrementalBuildQtAt(QString change, QString buildDir)
{
    QDir buildPath(buildDir);


    if (buildPath.exists() == false)
        buildPath.mkpath("dummy");

    resetBuildStatus(buildDir);

    qDebug() << "build path" << buildDir;

    if (buildPath.exists("FAILED")) {
        qDebug() << "skipping failed build at" << buildDir;
        return false;
    }

    QtDepotSource source(sourcePath);
    QtBuild build(source, buildDir);

    if (buildPath.exists("OK") == false) {
        if (syncBuild(change, buildDir, source, build) == false)
            return false;
    }

    buildPath.mkdir("OK");
    return true;
}

void ProjectHistoryBuilder::buildHistory()
{
    QString basePath = QDir::currentPath();
    QString stagePath = basePath +"/stage";
    QString tempStorePath = basePath + "/tempstore";


    bool storeFullCopy = true;
    dryRun = false;

    int maxIndex = revisions.count();

    for (int listIndex = 0; listIndex < maxIndex; ++listIndex) {
        QString revision = revisions.at(listIndex);
        QString status = getBuildStatus(revision);
        qDebug () << "at revision"  << revision << "status" << status;


        QString buildStorePath(basePath + "/" + revision);
        QString previousStorePath = (listIndex > 0) ? (basePath + "/" + revisions.at(listIndex - 1)) : QString();

        if (status == "OK") {
            qDebug() << "Already exists:" << revision << "skipping";
            continue;
        }

        rmrf(tempStorePath);
        QDir().mkpath(tempStorePath);

       //qDebug() << "rebuild at" << listIndex << commits.at(listIndex);
       if (!dryRun) {

            // sync the project to the current revsion.
            ProcessResult result = vcsClient->syncToRevision(revision);
            if (result.success == false) {
                qDebug() << "vcs sync to revision" << revision << "failed";
                qDebug() << result.output;
            }

            // A sucessfull build passes through three locations. It is first
            // (incrementally) built in stagePath.

            const QString projectRootPath = vcsClient->projectPath() + "/" + projectRoot;
            result = projectBuilder->buildProject(projectRootPath, stagePath);
            if (result.success) {
                // Then, the build output files are either copied or hard
                // linked from a previous build into tempStorePath.
                storeAndLinkBuild(tempStorePath, stagePath, previousStorePath);

                // Finally tempStorePath is renamed to buildStorePath,
                // this path name contains the commit sha1 for the build.
                move(tempStorePath, buildStorePath);
                setBuildStatus(revision, "OK");
            } else {
                setBuildStatus(revision, "FAIL");
                qDebug() << "build failed.";
            }
        }
    }
}

/*
    compares two files as cheaply as possible.
*/
bool compareFiles(const QString &file1, const QString &file2)
{
    QFile f1(file1);
    QFile f2(file2);
    if (f1.exists() == false || f2.exists() == false)
        return false;

    // Check size.
    if (f1.size() != f2.size())
        return false;

    // If the modifed dates match, assume the files are equal.
    if (QFileInfo(f1).lastModified() == QFileInfo(f2).lastModified())
        return true;

    // finally read the contents and compare. This happens when the file
    // sizes are identical but the last-modified date isn't -- a rare event.
    f1.open(QIODevice::ReadOnly);
    f2.open(QIODevice::ReadOnly);
    const bool equal = (f1.readAll() == f2.readAll());
/*
    if ((QFileInfo(f1).lastModified() == QFileInfo(f2).lastModified()) && !equal) {
        qWarning("timestamp is equal, but file contents differ");
        qDebug() << "read all" << file1 << "dates" << QFileInfo(f1).lastModified() << QFileInfo(f2).lastModified();
    }

*/
    return equal;
}

void storeAndLinkBuild(const QString &storagePath, const QString &buildPath, const QString &previousStoragePath)
{
    QDirIterator it(buildPath, QDirIterator::Subdirectories);

    qDebug() << "storeLink" << storagePath << buildPath << previousStoragePath;

    while (it.hasNext()) {
        it.next();
        QString fileName = it.fileName();
        QString relativeFilePath = it.filePath().remove(0, it.path().count());
        QString relativePath = relativeFilePath;
        relativePath.chop(fileName.count());
        QString storageFilePath = storagePath + "/" + relativeFilePath;
        QString linkFilePath = relativeFilePath.isEmpty() ? QString() : previousStoragePath + "/" + relativeFilePath;
        QString sourceFilePath = buildPath + "/" + relativeFilePath;
/*
        qDebug() << fileName;
        qDebug() << relativeFilePath;
        qDebug() << relativePath;
        qDebug() << storageFilePath;
        qDebug() << linkFilePath;
*/
        QFile sourceFile(sourceFilePath);

        QRegExp skipFiles("(^src|^demos|^doc|^examples|^tests|^tools|^qmake)");

        if (skipFiles.indexIn(relativeFilePath) != -1) {
//            qDebug() << "regexp skip" << relativeFilePath;
            continue;
        }

        if (QFileInfo(sourceFile.fileName()).isDir())
            continue;

        if (sourceFile.exists() == false)
            continue;

        if (relativePath.isEmpty() == false)
            QDir(storagePath).mkpath(relativePath);

        QFile linkFile(linkFilePath);

        if (storeFullCopy) {
            sourceFile.copy(storageFilePath);
        } else if (sourceFile.symLinkTarget().isEmpty() == false) {
//            qDebug() << "create symlink" << storageFilePath << "target" << sourceFile.symLinkTarget();
            QFile::link(sourceFile.symLinkTarget(), storageFilePath);
        } else if (linkFile.exists() && compareFiles(sourceFile.fileName(), linkFile.fileName()))  {
//            qDebug() << "linkfile" << storageFilePath  << "to" << linkFile.fileName();
            hardLink(linkFile.fileName(), storageFilePath);
        } else {
            qDebug() << "copyfile" << sourceFile.fileName() << "to" << storageFilePath;
            qDebug() << "source file info: size " << sourceFile.size() << QFileInfo(sourceFile).lastModified();
            qDebug() << "link file info: size " << linkFile.size() << QFileInfo(linkFile).lastModified();

            copyPreserveDate(sourceFilePath, storageFilePath);
        }
    }
}

GitClient::GitClient(const QString &sourceUrl)
{
    m_sourceUrl = sourceUrl;
    QStringList parts = sourceUrl.split('/');
    m_projectName = parts.at(parts.count() - 1);
    m_projectName.chop(4); // remove ".git";
    m_projectPath = QDir::currentPath() + "/" + m_projectName;

//    qDebug() << m_projectName;
}

ProcessResult GitClient::sync()
{
    if (QDir().exists(m_projectName)) {
        ProcessResult result = gitPull(m_projectPath);
        if (result.success == false) {
            if (result.output.contains("You asked me to pull without telling me which branch")) {
                // The source tree is busted, delete and pull again.
                rmrf(m_projectPath);
                return gitClone(QDir::currentPath(), m_sourceUrl);
            }
        }
        return result;
    }
    return gitClone(QDir::currentPath(), m_sourceUrl);
}

QStringList GitClient::revisions()
{
    if (m_revisions.isEmpty()) {
        ProcessResult result = gitCommits(m_projectPath);
        if (result.success) {
            foreach (QByteArray line, result.output.split('\n')) {
                QByteArray revision = line.split(' ').at(0);
                if (revision.isEmpty() == false)
                    m_revisions += revision;
            }
        } else {
            qDebug() << result.output;
        }
    }
    return m_revisions;
}

ProcessResult GitClient::syncToRevision(const QString &revision)
{
    return gitCeckout(m_projectPath, revision);
}

ProcessResult ProjectBuilder::buildProject(const QString &sourcePath, const QString &buildPath)
{
    qDebug() << "building source" << sourcePath << "at" << buildPath;
    ProcessResult result;
    result.success = true;
    return result;
}

ProcessResult QmakeProjectBuilder(const QString &sourcePath, const QString &buildPath)
{
   // qDebug() << "building source" << sourcePath << "at" << buildPath;


    ProcessResult result;
    result.success = false;
    return result;
}


ProjectHistoryBuilder::ProjectHistoryBuilder()
{
    this->workPath = QDir::currentPath() + "history";
}

void ProjectHistoryBuilder::build()
{
    openDatabase("database");
//    displayDatabaseTable("Builds");

    vcsClient = new GitClient(this->sourceUrl);
    projectBuilder = new ProjectBuilder;
    qDebug() << "syncing project" << sourceUrl;
    ProcessResult result = vcsClient->sync();
    if (result.success == false) {
        qDebug() << "Git sync error" << result.output;
        return;
    }
    qDebug() << "finding project revisions";
    revisions = vcsClient->revisions();
    if (revisions.isEmpty()) {
        qDebug() << "no revisions found";
        return;
    }
    qDebug() << "found" <<  revisions.count() << "revisions";
    buildHistory();
}

void Visitor::performVisit(const QList<QByteArray> &commits)
{
    m_commits = commits;
    QString sha1 = firstSha1();
    bool stop = (visit(sha1) == Fail);
    while (!stop) {
        sha1 = nextSha1(
);
        if (sha1.isEmpty())
            break;
        stop = (visit(sha1) == Fail);
    }
}

bool Visitor::isBuilt(const QString &sha1)
{
    return QDir(basePath + "/" + sha1).exists();
}

void Visitor::stage(const QString &sha1)
{
    QString stagePath2 = stagePath;
    stagePath2.chop(1);

    rmrf(stagePath2);
    symLink(basePath + "/" + sha1, stagePath);
}


