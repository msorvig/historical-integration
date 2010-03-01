#ifndef GUIDE_H
#define GUIDE_H

#include <QtCore>
#include "visitor.h"
#include "singletabledatabase.h"

class GuideDatabase : public SingleTableDatabase
{
   GuideDatabase(const QString &location);
   void addResult(const QByteArray &result, const QString& key);
   QByteArray result(const QString &key);
};

class Guide
{
public:
    Guide(const QString &basePath, Visitor *visitor);

    void visitAll();

    // pass-fail mode

    // database
    void dbOpoen();
    QByteArray dbFindResultBefore(const QByteArray &sha1, const QString &key);

};

#endif // GUIDE_H
