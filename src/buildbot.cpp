#include "buildbot.h"

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

QByteArray pipeExecutable(const QString &workdir, const QString &executable, const QStringList &arguments)
{
    QProcess p;
    p.setProcessChannelMode(QProcess::MergedChannels);
    if (workdir != QString())
        p.setWorkingDirectory(workdir);
    p.start(executable, arguments);
    if (p.waitForFinished(-1) == false) {
        qDebug() << "run" << executable << "failed" << p.error();
    }
    return p.readAll();
}

void writeFile(const QString &fileName, const QByteArray &contents)
{
    QFile f(fileName);
    f.open(QIODevice::WriteOnly | QIODevice::Append);
    f.write(contents);
}

QByteArray gitCommits(const QString &path)
{
    const QString arguments =  "--no-pager log --pretty=oneline";
    return pipeExecutable(path, "git", arguments.split(" "));
}

void gitCeckout(const QString &path, const QString &sha1)
{
    QString arguments = "checkout " + sha1;
    pipeExecutable(path, "git", arguments.split(" "));
}

QList<QByteArray> findCommits(const QString &path)
{
    QList<QByteArray> commits;
    QByteArray rawChanges = gitCommits(path);
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
    qDebug() << pipeExecutable(this->path, "git", arguments.split(" "));
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
    sleep(4);

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
    pipeExecutable("", "/bin/ln", QStringList() << "-s" << target << link);
}


void move(const QString &source, const QString &target)
{
    pipeExecutable("", "/bin/mv", QStringList() << source << target);
}

void rmrf(const QString &target)
{
    pipeExecutable("", "/bin/rm", QStringList() << "-rf" << target);
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

void buildHistorical(HistoricalBuildOptions options)
{
    basePath = options.basePath;
    stagePath = options.stagePath;
    sourcePath = options.sourcePath;
    bool storeFullCopy = options.storeFullCopy;
    dryRun = options.dryRun;

    int listIndex = 0;
    qDebug() << "finding commits in" << sourcePath;
    QList<QByteArray> commits = findCommits(sourcePath);

    for (int listIndex = 0; listIndex < options.commitCount; ++listIndex) {
        QByteArray change = commits.at(listIndex);
        QString buildStorePath(basePath + "/" + change);
        QString previousStorePath = (listIndex > 0) ? (basePath + "/" + commits.at(listIndex - 1)) : QString();
        QString tempStorePath = basePath + "/tempstore";

        if (QDir(buildStorePath).exists()) {
            qDebug() << "Already exists:" << change << "skipping";
            continue;
        }

        rmrf(tempStorePath);
        QDir().mkpath(tempStorePath);

       qDebug() << "rebuild at" << listIndex << commits.at(listIndex);
       if (!dryRun) {
            // A sucessfull build passes through three locations. It is first
            // incrementally built in stagePath.
            if (incrementalBuildQtAt(change, stagePath)) {

                // Then, the build output files are either copied or hard
                // linked from a previous build into tempStorePath.
                storeAndLinkBuild(tempStorePath, stagePath, previousStorePath);

                // Finally tempStorePath is renamed to buildStorePath,
                // this path name contains the commit sha1 for the build.
                move(tempStorePath, buildStorePath);
            } else {
                qDebug() << "build failed";
                // mark as failed?
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

void Visitor::performVisit(const QList<QByteArray> &commits)
{
    m_commits = commits;
    QString sha1 = firstSha1();
    bool stop = (visit(sha1) == Fail);
    while (!stop) {
        sha1 = nextSha1();
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


