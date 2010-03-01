#ifndef VISITOR_H
#define VISITOR_H

#include <QtCore>
#include "buildbot.h"

class Visitor
{
public:
    Visitor();
    virtual ~Visitor();
    enum VisitorMode { PassFailVisitor, DetectChangeVisitor, VisitAllVisitor };
    void setVisitorMode(VisitorMode visitorMode);
    void setCanCacheResults(bool enable);

    virtual QString visitorKey()= 0;
    virtual QStringList setupTests(const QString &qtSourcePath) = 0;
    virtual QStringList subKeys(const QString key) = 0;
    virtual QByteArray runTest(const QString &qtBuildPath, const QByteArray &sha1, const QString &key) = 0;
    enum VisitResponse { Pass, Fail, Skip };
    virtual VisitResponse isPass(const QByteArray &result, const QString &key);
    virtual QByteArray createHtmlReport(const QByteArray result, const QString&resourceStorePath);

    QString m_visitorKey;
    VisitorMode m_visitorMode;
    bool m_canCacheResults;
};

#endif // VISITOR_H
