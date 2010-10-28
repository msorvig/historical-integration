#ifndef PROCESS_H
#define PROCESS_H

#include <QProcess>
#include <QString>

class Process : public QProcess
{
public:
    Process();
    void setWatchFiles(bool enable);
    void addEnvironmentPath(const QString &path);

private:
    bool m_watchFiles;

};

#endif // PROCESS_H
