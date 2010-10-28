#include "reports.h"

void updateWeb()
{
    writeFile("report.html", readFile(":report.html"));
}

QByteArray readFile(const QString &fileName)
{
    QFile f(fileName);
    f.open(QIODevice::ReadOnly);
    return f.readAll();
}
