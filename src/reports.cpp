#include "reports.h"
#include "log.h"

void updateWeb()
{
    writeFile("report.html", readFile(":report.html"));
}

QByteArray readFile(const QString &fileName)
{
    QFile f(fileName);
    if (f.exists() == false) {
        Log::addError(QString("readFile: File Not Found %1").arg(fileName));
    }
    f.open(QIODevice::ReadOnly);
    return f.readAll();
}
