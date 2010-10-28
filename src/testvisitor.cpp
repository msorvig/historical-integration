#include "testvisitor.h"

TestVisitor::TestVisitor(const QStringList &commits)
{
    m_commits = commits;
}

QString TestVisitor::visitorKey()
{
    return QLatin1String("TestVisitor");
}

QStringList TestVisitor::setupTests(const QString &qtSourcePath)
{
    qDebug() << "TestVisitor::setupTests";
    QStringList keys;
    keys.append(QLatin1String("alwaysFail"));
    keys.append(QLatin1String("alwaysPass"));
    return keys;
}

QStringList TestVisitor::subKeys(const QString key)
{
    return QStringList();
}

QByteArray TestVisitor::runTest(const QString &qtBuildPath, const QByteArray &sha1, const QString &key)
{
    qDebug() << "TestVisitor::runTest" << qtBuildPath << sha1 << "key" << key;
    if (key == QLatin1String("alwaysFail"))
        return QByteArray("Fail");
    if (key == QLatin1String("alwaysPass"))
        return QByteArray("Pass");
    return QByteArray("Fail");
}

Visitor::VisitResponse TestVisitor::isPass(const QByteArray &result, const QString &key)
{
    qDebug() << "TestVisitor::isPass" << result << key;
    if (key == QLatin1String("Pass"))
        return Pass;
    if (key == QLatin1String("Fail"))
        return Fail;
    return Fail;
}

QByteArray TestVisitor::createHtmlReport(const QString &key, const QByteArray result, const QString&resourceStorePath)
{
    return result;
}
