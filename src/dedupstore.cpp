#include "dedupstore.h"

DeDupDatabase::DeDupDatabase(const QString &basePath)
:SingleTableDatabase(basePath + QLatin1String("/dedupdatabase.sqlite"), QLatin1String("FileName varchar, FileHash varchar"), QLatin1String("FileHashes"))
{
    openDatabase();
}

void DeDupDatabase::addHash(const QString &FileName, const QByteArray &FileHash)
{
    SINGLE_TABLE_DATABASE_INSERT_IMPLEMENTATION_2(FileName, FileHash);
}

QByteArray DeDupDatabase::getHash(const QString &FileName)
{
    //SINGLE_TABLE_DATABASE_SELECT_IMPLEMENTATION_1(ByteArray, FileHash, FileName);
    return QByteArray();
}

DeDupStore::DeDupStore()
{
}

void DeDupStore::setDatabaseDirectory(const QString &basePath)
{

}

void DeDupStore::writeFile(const QString &filePath, const QByteArray &fileContents, const QString &duplicationPathHint)
{

}

void DeDupStore::copyfile(const QString &target, const QString &source, const QString &duplicationPathHint)
{

}

void DeDupStore::copyDirectory(const QString &target, const QString &source, const QString &duplicationPathHint)
{

}

