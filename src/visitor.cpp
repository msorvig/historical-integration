#include "visitor.h"

Visitor::Visitor()
{
    m_visitorMode = VisitAllVisitor;
    m_canCacheResults = true;
}

Visitor::~Visitor()
{

}

void Visitor::setVisitorMode(VisitorMode visitorMode)
{
    m_visitorMode = visitorMode;
}

void Visitor::setCanCacheResults(bool enable)
{
    m_canCacheResults = enable;
}

Visitor::VisitResponse Visitor::isPass(const QByteArray &result, const QString &key)
{
    return Skip;
}

QByteArray Visitor::createHtmlReport(const QByteArray result, const QString&resourceStorePath)
{
    return QByteArray();
}

