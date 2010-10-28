#include "guide.h"

Guide::Guide(const QString &basePath, Visitor *visitor)
{

}

GuideDatabase::GuideDatabase(const QString &location)
:SingleTableDatabase(location + "/guidedb.sqlite", "Key varchar, Result varchar",QLatin1String("Tests"))
{
    openDatabase();
}

void GuideDatabase::addResult(const QByteArray &result, const QString& key)
{

}

QByteArray result(const QString &key)
{
    return QByteArray();
}
