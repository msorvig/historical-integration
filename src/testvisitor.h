#ifndef TESTVISITOR_H
#define TESTVISITOR_H

#include "visitor.h"

class TestVisitor : public Visitor
{
    TestVisitor(const QStringList &commits);
    QString visitorKey();
    QStringList setupTests(const QString &qtSourcePath);
    QStringList subKeys(const QString key);
    QByteArray runTest(const QString &qtBuildPath, const QByteArray &sha1, const QString &key);
    VisitResponse isPass(const QByteArray &result, const QString &key);
    QByteArray createHtmlReport(const QString &key, const QByteArray result, const QString&resourceStorePath);

    QStringList m_commits;
};

#endif // TESTVISITOR_H
