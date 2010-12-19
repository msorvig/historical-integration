#include <QtCore>
#include <QTest>

#include <benchmarker.h>
#include <attributedtable.h>
#include <gitclient.h>
#include <reportgenerator.h>
#include <timegroupcounter.h>

void loadData(Database *database, const QString  branch, const QString &qtPath);
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
    }

    qDebug() << "Accepted branches" << filteredBranches;
    foreach (const QString &branch, filteredBranches)
        loadData(database, branch, qtPath);
}

void loadData(Database *database, const QString  branch, const QString &qtPath)
{
    // Get history from git
    QStringList timestamps;

    GitClient gitClient(qtPath);
    timestamps =
            gitClient.runCommand(QStringList() << "log" << branch
                                               << "--no-merges"
                                               << "--pretty=format:%H %ct %at");

    qDebug() << "found commits: " << branch << timestamps.count();

    database->transaction();

    // Parse and add rows to database;
    foreach (const QString &line, timestamps) {
/*
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
            qDebug() << line;
            continue;
        }
        QString sha1 = parts[0];
        QString timestamp = parts[1];
        QString branchName = branch.mid(QString("origin/").count(), -1);

        database->insertRow("Commits", QStringList() << "Branch" << "Sha1" << "Time" ,
                                      QList<QVariant>() << branchName << sha1 << timestamp);
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
    // as well, shoul only touch the edges.

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

    Database("qtstats").deleteDatabase();
    Database database("qtstats");

    AttributedTable commitsTable(&database, "Commits");
    commitsTable.setTableScema(QStringList() << "Branch" << "Sha1" << "Time",
                               QStringList() << "VARCHAR" << "VARCHAR" << "INTEGER");

    loadData(&database, qtPath);
    database.execQuery("CREATE INDEX CommitsIndex ON Commits (Time)", true);

    deleteDuplicates(&database, commitsTable);

    TimeGroupCounter counter(&database);
    AttributedTable commitRate = counter.count(commitsTable, "Branch", "Time", TimeGroupCounter::Week, "CommitRate");

    trimZeroValues(&database, commitRate);

    commitRate.setAttribute("Title", "Weekly Commit Rate For Qt");
    commitRate.setAttribute("CountTitle", "Commit Count");
    commitRate.setAttribute("TimeTitle", "Time");
    commitRate.setAttribute("timeChart", "true");

    // Generate report
    ReportGenerator(&database, "CommitRate").generateReport(QDir::currentPath());
}
