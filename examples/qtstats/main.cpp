#include <QtCore>
#include <QTest>

#include <benchmarker.h>
#include <attributedtable.h>
#include <gitclient.h>
#include <reportgenerator.h>
#include <timegroupcounter.h>

void loadBranchData(Database *database, const QString  branch, const QString &qtPath);
void loadData(Database *database, const QString &qtPath)
{
     GitClient gitClient(qtPath);
     QStringList branches =
             gitClient.runCommand(QStringList() << "branch" << "-r");
     // qDebug() << "branches" << branches;

     // filter branches, allow x.y and "master"
    QStringList filteredBranches;
    foreach (const QString &candidate, branches) {
        if (candidate.simplified() == "origin/1.0.0") // for creator
            filteredBranches.append(candidate.simplified());
        if (candidate.simplified() == "origin/master")
            filteredBranches.append(candidate.simplified());
        if (candidate.count('.') == 1 && candidate.count('-') == 0)
            filteredBranches.append(candidate.simplified());

        //if (filteredBranches.count() > 1)
        //    break;
    }

    qDebug() << "Accepted branches" << filteredBranches;
    foreach (const QString &branch, filteredBranches)
        loadBranchData(database, branch, qtPath);
}

void loadBranchData(Database *database, const QString  branch, const QString &qtPath)
{
    // Get history from git
    QStringList timestamps;

    GitClient gitClient(qtPath);
    timestamps =
            gitClient.runCommand(QStringList() << "log" << branch
                                               << "--no-merges"
                                               << "--pretty=format:%H %ct %at"
                                               << "--shortstat");

    qDebug() << "found commits: " << branch << timestamps.count() / 2;

    database->transaction();

    // Parse and add rows to database;
    for (int i = 0; i < timestamps.count(); i+=2) {
        const QString line = timestamps.at(i);
        const QString line2 = timestamps.at(i + 1);

        // qDebug() << "lines" << i << line << line2;
/*
        // stop when we reach existing commits.

        if (database->selectVariant(
                QString("select Time from Commits where Time=%1 AND Branch='%2'")
                .arg(timestamp).arg(branch)).isValid()) {
            qDebug() << "has walue";
            break;
        }
*/
        // format: sha1 commit-time author-time
        QStringList parts = line.split(" ");
        if (parts.count() < 3) {
            qDebug() << "parse error" << line;
            continue;
        }
        QString sha1 = parts[0];
        QString commitTime = parts[1];
        QString authorTime = parts[2];
        QString branchName = branch.mid(QString("origin/").count(), -1);


        // format: 1 files changed, 1 insertions(+), 1 deletions(-)
        QStringList parts2 = line2.split(",");
        if (parts2.count() < 3) {
            qDebug() << "parse error" << line2;
            continue;
        }

        QString inserted = parts2.at(1).simplified().split(' ').at(0);
        QString deleted = parts2.at(2).simplified().split(' ').at(0);
        int patchSize = inserted.toInt() + deleted.toInt();
      //  qDebug() << "patch size" <<  patchSize;


        database->insertRow("Commits", QStringList() << "Branch" << "Sha1" << "CommitTime" << "PatchSize" ,
                                      QList<QVariant>() << branchName << sha1 << commitTime << patchSize);

    }
    database->commit();
}

// removes commits that appear in earlier branches from later branches.
// for example, commits pushed to 4.6 and integrated to 4.7 counts
// against 4.6 only.
void deleteDuplicates(Database *database, AttributedTable table)
{
    QStringList branches = database->selectDistinct("Branch", "Commits");
    qSort(branches);

    qDebug() << "";
    qDebug() << "Removing duplicates:";

    for (int i = 0; i < branches.count(); ++i) {
        for (int j = i + 1; j < branches.count(); ++j ) {
            QString current = branches.at(i);
            QString lower = branches.at(j);
            qDebug() << "From" << current << "in" << lower;
            table.exec("delete from %table% where Branch=? and Sha1 in (select Sha1 from %table% where Branch =?)",
                        QVariantList() << lower << current);
        }
    }
}

void trimZeroValues(Database *database, AttributedTable table)
{

    qDebug() << "";
    qDebug() << "deleting zero counts";

    // ### will reomve zero-counts from the middle of the series
    // as well, should only touch the edges.

    table.exec("delete from %table% where Count=?", QVariantList() << 0);

}

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    if (a.arguments().count() < 2) {
        qDebug() << "Usage: qtstats path-to-qt";
        return 0;
    }

    const QString qtPath = a.arguments().at(1);
    qDebug() << "Looking at Qt at" << qtPath;

    bool loadData = true;

    if (loadData) {
        Database("qtstats.sqlite").deleteDatabase();
    }
    Database database("qtstats.sqlite");

    AttributedTable commitsTable(&database, "Commits");
    commitsTable.setTableScema(QStringList() << "Branch" << "Sha1" << "CommitTime" << "PatchSize",
                               QStringList() << "VARCHAR" << "VARCHAR" << "INTEGER" << "INTEGER");
    commitsTable.setColumnRoleAttributes(commitsTable.columnNames(),
                               QStringList() << "Index" << "Data" << "TimeIndex" << "Data");

    if (loadData) {
        loadData(&database, qtPath);
    }
    database.execQuery("CREATE INDEX CommitsIndex ON Commits (CommitTime)", true);

    deleteDuplicates(&database, commitsTable);

    TimeGroupCounter counter(&database);
    counter.skipColumn("Sha1");
    AttributedTable aggregatedCommits = counter.aggregate(commitsTable);

    trimZeroValues(&database, aggregatedCommits);

    aggregatedCommits.setAttribute("Title", "Weekly Commit Rate For " + QDir(qtPath).dirName());
    aggregatedCommits.setAttribute("CountTitle", "Commit Count");
    aggregatedCommits.setAttribute("TimeTitle", "Time");
    aggregatedCommits.setAttribute("timeChart", "true");

    // Generate report
    ReportGenerator(&database, aggregatedCommits.tableName()).generateReport(QDir::currentPath());
}
