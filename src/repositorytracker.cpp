#include "repositorytracker.h"

RepositoryTracker::RepositoryTracker(const QString &basePath)
{

}

void RepositoryTracker::addBuild(const QString &url, const QString &branch)
{

}

void RepositoryTracker::maintainBuilds()
{

}

QList<QByteArray> RepositoryTracker::scrapeGitorousHtmlForQtRepositories(const QByteArray &html)
{
    QList<QByteArray> repositories;
    qDebug() << "got bytes" << html.count();

    // excract the repo path from lines that look like
    //  "            <a href="/projects/qt/repos/history">history</a>                    </li>"
    QByteArray lineStart("<a href=\"/projects/qt/");
    QList<QByteArray> lines = html.split('\n');
    foreach (const QByteArray &line, lines) {
        if (line.trimmed().startsWith(lineStart)) {
            QList<QByteArray> parts = line.split('"');
            if (parts.count() > 1 && parts.at(1).startsWith("/projects"))
                repositories.append(parts.at(1));
        }
    }

    return repositories;
}
