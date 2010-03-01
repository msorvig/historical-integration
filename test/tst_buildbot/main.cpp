#include <QtCore>
#include <QtGui>
#include <QtTest>
#include <singletabledatabase.h>
#include <repositorytracker.h>
#include <gitclient.h>
#include <process.h>
#include "qtbuildmanager.h"

class tst_BuildBot : public QObject
{
Q_OBJECT
private slots:
    void singleTableDatabase();
    void repositoryTracker();
    void walkQtHistory();
    void writeFiles();
    void fileWatcherProcess();
    void buildManager();
};

class TestDatabase : public SingleTableDatabase
{
public:
    TestDatabase()
    :SingleTableDatabase("test.sqlite", "Key varchar, Value varchar", "test")
    {
    }

    void insert(QByteArray Key, QByteArray Value)
    {
        SINGLE_TABLE_DATABASE_INSERT_IMPLEMENTATION_2(Key, Value);
    }

    QByteArray value(QByteArray Key)
    {
         //SINGLE_TABLE_DATABASE_SELECT_IMPLEMENTATION_1(ByteArray, Value, Key);
        return QByteArray();
    }
};

void tst_BuildBot::singleTableDatabase()
{
    TestDatabase foo;
    foo.deleteDatabase();
    foo.openDatabase();

   // QCOMPARE(foo.value("ice"), QByteArray());

    foo.insert("ice", "waesel");
    foo.insert("fire", "fox");

    qDebug() << "done instert";

    QSqlQuery query(foo.m_database);
    if (query.prepare("SELECT Value FROM test WHERE Key='ice'") == false)
        qDebug() << "prepare error:" << query.lastError().text();
    query.exec();
    if (query.isActive()) {
        query.next();
        qDebug() << "active" << query.value(0);
    } else {
        query.lastError().text();
    }

    qDebug() << "value";

    QTableView * tableView = new QTableView();
    tableView->setModel(foo.sqlTableModel());
    tableView->show();
    QTest::qWait(10000);

    QCOMPARE(foo.value("ice"), QByteArray("weasel"));
    QCOMPARE(foo.value("fire"), QByteArray("fox"));
}

void tst_BuildBot::repositoryTracker()
{
    RepositoryTracker tracker(QDir::currentPath() + "/tracker");

    QFile f("../qt.htm");
    f.open(QIODevice::ReadOnly);
    QByteArray contents = f.readAll();
    QVERIFY(contents.isEmpty() == false);

    QList<QByteArray> repositories = tracker.scrapeGitorousHtmlForQtRepositories(contents);
    qDebug() << repositories.count();

    tracker.addBuild("c:\work\qt", "master");
}

void tst_BuildBot::walkQtHistory()
{
    QString path = "c:/work/qt";
    QVERIFY(QFile::exists(path));

    // fake
    bool mock = true;
    int revsToTest = 100;

    // real
    //bool mock = false;
    //int revsToTest = 5;

    GitClient gitClient(path);
    gitClient.setMockRevisionSync(mock);
    QStringList revisions = gitClient.revisions();
    for (int i = 0; i < revsToTest; ++i) {
        gitClient.syncToRevision(revisions.at(i));
    //    qDebug() << gitClient.currentRevision();
        QCOMPARE(gitClient.currentRevision(), revisions.at(i));
    }
}

// not a real test, used by fileWatcherProcess
void tst_BuildBot::writeFiles()
{
    QFile testFile("test.deleteme");
    testFile.open(QIODevice::WriteOnly);
    testFile.write(QByteArray("fo oo"));
}

void tst_BuildBot::fileWatcherProcess()
{
    Process process;
    QString pwd = QDir().currentPath();
}

void tst_BuildBot::buildManager()
{
    QString path = "c:/work/qt";
    QVERIFY(QFile::exists(path));

    GitClient gitClient(path);
    gitClient.setMockRevisionSync(true);

    QtBuildManager manager;
 //   manager.setGitClient(&gitClient);
 //   manager.setTestMode(true);

 //   manager.setWorkDirectory(QDir::currentPath() + "/tst_buildManager");
}


QTEST_MAIN(tst_BuildBot);

#include "main.moc"
