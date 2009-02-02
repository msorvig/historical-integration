#include <buildbot.h>
#include <QtCore>

QString source = "/Users/msorvig/code/buildbot/qt"; 
QString stageDir = "/Users/msorvig/build/daily/stage";

class PrintInfoVisitor : public LinearSearchVisitor
{
    virtual Visitor::VisitResponse visit(const QString &sha1)
    {
        bool built = isBuilt(sha1);
        qDebug() << "commit" << sha1 << "built?" << built;
        if (built) {
            stage(sha1);
            qDebug() << "core size" << QFile(stageDir + "/lib/QtCore.framework/QtCore").size();
            qDebug() << "gui size" << QFile(stageDir + "/lib/QtGui.framework/QtGui").size();
        }

        return Visitor::Pass;
    }
};


int main(int arc, char **argv)
{

    QList<QByteArray> commits = findCommits(source);
    PrintInfoVisitor visitor;
    visitor.performVisit(commits);
}
