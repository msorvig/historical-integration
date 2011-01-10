#include "gitclient.h"

QString findGit()
{
    static QString gitLocation;
    if (gitLocation.isEmpty() == false)
        return gitLocation;
    gitLocation = QLatin1String("C:/Program Files/git/bin/git.exe");
    if (QFile::exists(gitLocation))
        return gitLocation;
    gitLocation = "git"; // rely on git in path
    return gitLocation;
}


ProcessResult gitCommits(const QString &path)
{
    const QString arguments =  "--no-pager log --pretty=oneline";
    return pipeExecutable(path, findGit(), arguments.split(" "));
}

ProcessResult gitCeckout(const QString &path, const QString &sha1)
{
    QString arguments = "checkout " + sha1;
    return pipeExecutable(path, findGit(), arguments.split(" "));
}

ProcessResult gitClone(const QString destinationPath, const QString &sourceUrl)
{
    QString arguments = "clone " + sourceUrl;
    return pipeExecutable(destinationPath, findGit(), arguments.split(" "));
}

ProcessResult gitPull(const QString path)
{
    QString arguments = "pull";

    ProcessResult result = pipeExecutable(path, findGit(), arguments.split(" "));
    if (result.output.contains("Already up-to-date")) // Git reports this as an error,
        result.success = true;                        // but this is really a normal condition.
    return result;
}

ProcessResult gitSyncBack(const QString path, int revisions)
{
    QString arguments = QString("reset HEAD~%1 --hard").arg(revisions);

    ProcessResult result = pipeExecutable(path, findGit(), arguments.split(" "));
    return result;
}

ProcessResult gitStatus(const QString path, const QString &sourceUrl)
{
    QString arguments = "status";
    return pipeExecutable(path, findGit(), arguments.split(" "));
}

ProcessResult gitCurrentCommit(const QString &path)
{
    QString arguments = QString("log HEAD --pretty=format:%H -1");
    return pipeExecutable(path, findGit(), arguments.split(" "));
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

GitClient::GitClient(const QString &repositoryPath)
{
    m_projectPath = repositoryPath;
    m_mockRevisionSync = false;
}

// clones a local directoty to workdir.
GitClient * GitClient::cloneFromLocalPath(const QString &sourceDirectory, const QString &workDir)
{
    GitClient *gitClient = new GitClient;
    gitClient->m_sourceUrl = sourceDirectory;
    QStringList parts = sourceDirectory.split('/', QString::SkipEmptyParts);
    gitClient->m_projectName = parts.at(parts.count() - 1);
    gitClient->m_projectPath = workDir + gitClient->m_projectName;
    gitClient->m_workPath = workDir;

    qDebug() << "GitClient" << gitClient->m_sourceUrl << gitClient->m_projectName
             << gitClient->m_projectPath << gitClient->m_workPath;
    return gitClient;
}

// clones a git url to the current directory.
GitClient * GitClient::cloneFromUrl(const QString &sourceUrl, QString workPath = QDir::currentPath())
{
    workPath = QDir(workPath).absolutePath();

    GitClient *gitClient = new GitClient;
    gitClient->m_sourceUrl = sourceUrl;
    QStringList parts = sourceUrl.split('/');
    gitClient->m_projectName = parts.at(parts.count() - 1);
    gitClient->m_projectName.chop(4); // remove ".git";
    gitClient->m_projectPath = workPath + "/" + gitClient->m_projectName;
    gitClient->m_workPath = workPath;
    return gitClient;
//    qDebug() << m_projectName;
}

ProcessResult GitClient::clone()
{
    //gitClone();
    return ProcessResult();
}

ProcessResult GitClient::sync()
{
    qDebug() << "test" << m_projectPath;
    if (QDir().exists(m_projectPath)) {
        qDebug() << "Pull";
        ProcessResult result = gitPull(m_projectPath);
        if (result.success == false) {
            if (result.output.contains("You asked me to pull without telling me which branch")) {
                // The source tree is busted, delete and clone again.
                rmrf(m_projectPath);
                return gitClone(m_workPath, m_sourceUrl);
            }
        }
        return result;
    }
    qDebug() << "clone";
    return gitClone(m_workPath, m_sourceUrl);
}

ProcessResult GitClient::syncBack(int revisions)
{
    return gitSyncBack(m_projectPath, revisions);
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
            qDebug() << "GitClient::revisions failed to run git";
        }
    }
    return m_revisions;
}

QStringList GitClient::runCommand(const QStringList &arguments)
{
    QStringList actualArguments = arguments;
    actualArguments.prepend("--no-pager");

    QStringList output;
    ProcessResult result = pipeExecutable(m_projectPath, findGit(), actualArguments);
    foreach (QByteArray line, result.output.split('\n')) {
        if (line.simplified().isEmpty() == false)
            output.append(line);
    }
    return output;
}

QString GitClient::currentRevision()
{
    if (m_mockRevisionSync) {
        return m_mockedRevision;
    }
    return gitCurrentCommit(m_projectPath).output;
}

ProcessResult GitClient::syncToRevision(const QString &revision)
{
    if (m_mockRevisionSync) {
        m_mockedRevision = revision;
        return ProcessResult(true, QByteArray());
    }
    return gitCeckout(m_projectPath, revision);
}

void GitClient::setMockRevisionSync(bool enable)
{
    m_mockRevisionSync = enable;
}
