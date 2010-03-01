#ifndef DEDUPSTORE_H
#define DEDUPSTORE_H

#include <singletabledatabase.h>
#include <QtCore>

class DeDupDatabase : public SingleTableDatabase
{
    DeDupDatabase(const QString &basePath);
    void addHash(const QString &fileName, const QByteArray &hash);
    QByteArray getHash(const QString &fileName);
};


class DeDupStore
{
public:
    DeDupStore();
    void setDatabaseDirectory(const QString &basePath);

    void writeFile(const QString &filePath, const QByteArray &fileContents, const QString &duplicationPathHint = QString());
    void copyfile(const QString &target, const QString &source, const QString &duplicationPathHint = QString());
    void copyDirectory(const QString &target, const QString &source, const QString &duplicationPathHint = QString());

    QString m_databasePath;
    bool m_platformSupportsHardLinks;
};

#endif // DEDUPSTORE_H
