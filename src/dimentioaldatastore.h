#ifndef DIMENTIOALDATASTORE_H
#define DIMENTIOALDATASTORE_H

#include <QtCore>

/*


CellsSW
Aggregated Dimentions


*/

class DimentioalDataStore
{
public:
    DimentioalDataStore();

    void addDimention(const QStringList &name, const QStringList &subdimentions);

    void beginTransaction();
    void endTransaction();

    void updateDatapoint(const QStringList dimentions, const QStringList &values, int datapoint);

    // callback
    void processDatapoint(const QStringList dimentions, const QStringList &values);

    //
    // A hash of dimention specifiers:
    //
    // The hash key is a display name, the strings in the list
    // corresponds to columns in the database.
    //
    // A simple dimention: QtVersion -> QtVersion
    // A hiearchial dimention: QtVersion -> QtMinorVersion, QtPatchVersion
    //                         Benchmarks -> TestCase, TestFunction, TestData
    //                         Platform -> MajorPlatform, MinorPlatform
    //
    QHash<QString, QStringList> dimentions;
};

#endif // DIMENTIOALDATASTORE_H
