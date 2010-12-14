#ifndef BENCHMARKDIMENTION_H
#define BENCHMARKDIMENTION_H

#include <database.h>
#include <QtCore>


/*
    A BenchmarkDimention object stores a dimention in a database table.

    A dimention has a name, a sub-dimention hiearchy and several instances.

    Dimention examples: (Name: Hiearchy)
    Test: TestCase - TestFunction - TestData
    QtVersion : MajorMinorVersion - PatchVersion - sha1
    Platform : PlatformFamily - SpesificPlatform

    Instance examples:
    Test: QtWidget - setGeometry - zeroGeometry
    QtVersion: 4.7 - 0 - sha1
    Platform X11 - Maemo

    This has the following correspondances on the database level:
    Name -> Table Name
    Sub-Dimentions -> Columns
    Instances -> Rows

    In addition to the sub-dimention columns the table also has an instance id column.

 */
class BenchmarkDimention
{
public:
    BenchmarkDimention(Database *database, const QString &dimentionName);

    void setDimention(const QString &dimentionName, const QString dimentionInstance);
    void commitInstance();

private:
    BenchmarkTable *m_benchmarkTable;

};

#endif // BENCHMARKDIMENTION_H
