#ifndef REPORTS_H
#define REPORTS_H

#include <QtCore>

void updateWeb();
QByteArray readFile(const QString &fileName);
void writeFile(const QString &fileName, const QByteArray &contents);

void createTestTable(const QString &databaseTableName, int dimentions);

class Reports
{
    Reports(const QString &databaseTableName);
    QByteArray generateTableData(const QString &databaseTableName);
    QByteArray generateTablePage(const QString &databaseTableName);
};


#endif // REPORTS_H
