#include "buildbot.h"
#include <QtSql>
#include <QtGui>
#include "singletabledatabase.h"

// -----------

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

// forwards
void storeAndLinkBuild(const QString &storagePath, const QString &buildPath, const QString &previousStoragePath);


ProcessResult runProcess(const QString &executable, const QStringList &arguments, const QString workingDir)
{
    QProcess process;
#ifdef Q_OS_WIN
    // ### less hardcoding.
    QStringList env = QProcess::systemEnvironment();
    env.replaceInStrings(QRegExp("^PATH=(.*)", Qt::CaseInsensitive),
                         "PATH=\\1;"
                         "C:\\Program Files\\Microsoft Visual Studio 8\\VC\\bin;"
                         "C:\\Program Files\\Microsoft Visual Studio 8\\Common7\\IDE;"
                         "C:\\Program Files\\Microsoft Visual Studio 8\\VC\\BIN;"
                         "C:\\Program Files\\Microsoft Visual Studio 8\\Common7\\Tools;"
                         "C:\\Program Files\\Microsoft Visual Studio 8\\SDK\\v2.0\\bin"
                         "C:\\WINDOWS\\Microsoft.NET\\Framework\\v2.0.50727;"
                         "C:\\Program Files\\Microsoft Visual Studio 8\\VC\\VCPackages"
                         "C:\\Program Files\\Microsoft Platform SDK\\Bin;"
                         "C:\\Program Files\\Microsoft Platform SDK\\Bin\\WinNT");
    env << "Include=C:\\Program Files\\Microsoft Visual Studio 8\\VC\\INCLUDE;"
           "C:\\Program Files\\Microsoft Platform SDK\\Include;";
    env << "Lib=C:\\Program Files\\Microsoft Visual Studio 8\\VC\\LIB;"
           "C:\\Program Files\\Microsoft Visual Studio 8\\SDK\\v2.0\\lib;"
           "C:\\Program Files\\Microsoft Platform SDK\\Lib;";
    process.setEnvironment(env);
#endif
    process.setReadChannelMode(QProcess::MergedChannels);

    if (workingDir != QString())
        process.setWorkingDirectory(workingDir);

    process.start(executable, arguments);
    process.waitForFinished(-1);

    return ProcessResult(process.exitCode() == 0, process.readAll());
}

void writeFile(const QString &fileName, const QByteArray &contents)
{
    qDebug() << "writing to" << fileName;

    QDir().mkpath(QFileInfo(fileName).absolutePath());

    QFile f(fileName);
    f.open(QIODevice::WriteOnly);
    f.write(contents);
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
#ifdef Q_OS_WIN
    return runProcess("C:/Program Files/Microsoft Visual Studio 8/VC/BIN/nmake.exe", makeArguments, buildPath);
#else
    return runProcess("/usr/bin/make", makeArguments, buildPath);
#endif
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
    pipeExecutable("", programFilesLocation() + "/git/bin/ln.exe", QStringList() << "-s" << target << link); // ### program files, req Git
#else
    pipeExecutable("", "/bin/ln", QStringList() << "-s" << target << link);
#endif
}


void move(const QString &source, const QString &target)
{
#ifdef Q_OS_WIN
    qDebug() << "move" << source << "to" << target;
    pipeExecutable("", programFilesLocation() + "/git/bin/mv.exe", QStringList() << source << target); // ### req Git
#else
    pipeExecutable("", "/bin/mv", QStringList() << source << target);
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

bool buildQtMac(QtBuild build)
{
    ProcessResult result;
    qDebug() << "make sub-src";
    result = build.make(QStringList() << "sub-src");
    qDebug() << "made" << result.output;
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
    qDebug() << result.output;

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

bool buildQtWin(QtBuild build)
{
    ProcessResult result;
    qDebug() << "win make sub-src";
    result = build.make(QStringList() << "sub-src");
    qDebug() << "made" << result.output;
    if (result.success == false) {
        qDebug() << "make failed";
        qDebug() << result.output;
        writeFile(build.buildPath + "/log", result.output);
        return false;
    }
    //qDebug() << result.output;
    return true; //
}

bool buildQt(QtBuild build)
{
#ifdef Q_OS_WIN
    return buildQtWin(build);
#elif defined(Q_OS_MAC)
    return buildQtMac(build);
#endif
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

void ProjectHistoryBuilder::buildHistory(int revisionCount)
{
    const QString sourcePath = basePath  + gitClient->m_projectName;
    QString stagePath;
    if (useShadowBuild)
        stagePath = basePath +"/stage";
    else
        stagePath = sourcePath;
    QString tempStorePath = basePath + "/tempstore";

    revisions = gitClient->revisions();
    if (revisions.isEmpty()) {
        qDebug() << "no revisions found";
        return;
    }


    bool storeFullCopy = true;
    Q_UNUSED(storeFullCopy);
    //    dryRun = true;

    int maxIndex = qMin(revisionCount,revisions.count());

    for (int listIndex = 0; listIndex < maxIndex; ++listIndex) {
      //  QString revision = gitClient->currentRevision();
        QString revision = revisions.at(listIndex);
        QString status = buildDatabase.getBuildStatus(revision);
        qDebug() << "";
        qDebug() << "At revision"  << revision << "status" << status;


        QString buildStorePathBase = basePath + "/builds/";
        QDir().mkpath(buildStorePathBase);
        QString buildStorePath(buildStorePathBase + revision);
        QString previousStorePath = (listIndex > 0) ? (basePath + "/" + revisions.at(listIndex - 1)) : QString();

        qDebug() << "buildStorePath" << buildStorePath;

        if (status == "OK") {
            qDebug() << "Already exists:" << revision << "skipping";
            continue;
        }

        rmrf(tempStorePath);
        QDir().mkpath(tempStorePath);

      // qDebug() << "rebuild at" << listIndex << revisions.at(listIndex);
       if (!dryRun) {



            // A sucessfull build passes through three locations. It is first
            // (incrementally) built in stagePath.


            qDebug() << "project root path" << sourcePath << "stage path" << stagePath;


            ProcessResult result = projectBuilder->buildProject(sourcePath, stagePath);
            if (result.success) {
                // Then, the build output files are either copied or hard
                // linked from a previous build into tempStorePath.
                storeAndLinkBuild(tempStorePath, stagePath, previousStorePath);

                // Finally tempStorePath is renamed to buildStorePath,
                // this path name contains the commit sha1 for the build.
                move(tempStorePath, buildStorePath);
                buildDatabase.setBuildStatus(revision, "OK", result.output);
            } else {
                buildDatabase.setBuildStatus(revision, "FAIL", result.output);
                qDebug() << "build failed.";
            }
        }
       gitClient->syncBack(1);
   } // for
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

void recursiveCopy(const QString &source, const QString &target)
{
    QDirIterator it(source, QDirIterator::Subdirectories);
        while (it.hasNext()) {
        it.next();
        QString fileName = it.fileName();
        QString relativeFilePath = it.filePath().remove(0, it.path().count());
        QString relativePath = relativeFilePath;
        relativePath.chop(fileName.count());
        QString storageFilePath = target + relativeFilePath;
      //  QString linkFilePath = relativeFilePath.isEmpty() ? QString() : previousStoragePath + "/" + relativeFilePath;
        QString sourceFilePath = source + relativeFilePath;

      //  qDebug() << "copy" << sourceFilePath << "to" << storageFilePath;
        //QFile::copy(

        /*
        qDebug() << fileName;
        qDebug() << relativeFilePath;
        qDebug() << relativePath;
        qDebug() << storageFilePath;
        qDebug() << linkFilePath;
*/
    }
}

void storeAndLinkBuildWin(const QString &storagePath, const QString &buildPath, const QString &previousStoragePath)
{
    qDebug() << "storeAndLinkBuildWin storagePath (destination)" << storagePath;
    qDebug() << "buildPath (source)"<< buildPath;
    qDebug() << "previousStoragePath (prev. build)" << previousStoragePath;

    // copy lib/*
    QString libSourcePath = buildPath + "/lib";
    QString libTargetPath = storagePath + "/lib";
    QDir("/").mkpath(storagePath);
    recursiveCopy(libSourcePath, libTargetPath);

    // copy qmake
}

void storeAndLinkBuildMac(const QString &storagePath, const QString &buildPath, const QString &previousStoragePath)
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

void storeAndLinkBuild(const QString &storagePath, const QString &buildPath, const QString &previousStoragePath)
{
#ifdef Q_OS_WIN
    storeAndLinkBuildWin(storagePath, buildPath, previousStoragePath);
#elif defined(Q_OS_MAC)
    storeAndLinkBuildMac(storagePath, buildPath, previousStoragePath);
#endif
}


ProjectBuilder::~ProjectBuilder()
{

}

ProcessResult ProjectBuilder::buildProject(const QString &sourcePath, const QString &buildPath)
{
    qDebug() << "building source" << sourcePath << "at" << buildPath;
    QtDepotSource source(sourcePath);
    QtBuild build(source, buildPath);
    buildQt(build);

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


ProjectHistoryBuilder::ProjectHistoryBuilder(const QString &sourceUrl, const QString &basePath)
:buildDatabase(basePath + "/builddatabase.sqlite")
{
    this->sourceUrl = sourceUrl;
    this->basePath = basePath;
    this->workPath = basePath + "/history";
    gitClient = GitClient::cloneFromLocalPath(this->sourceUrl, this->basePath);
    this->dryRun = false;
    this->useShadowBuild = false;
}

void ProjectHistoryBuilder::build(int revisionCount)
{

//    displayDatabaseTable("Builds");

       projectBuilder = new ProjectBuilder;
    qDebug() << "syncing project" << sourceUrl;
    ProcessResult result = gitClient->sync();
    if (result.success == false) {
        qDebug() << "Git sync error" << result.output;
        return;
    }
    qDebug() << "finding project revisions";
    revisions = gitClient->revisions();
    if (revisions.isEmpty()) {
        qDebug() << "no revisions found";
        return;
    }
    qDebug() << "found" <<  revisions.count() << "revisions";
    buildHistory(revisionCount);
}
/*
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
*/

