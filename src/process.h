#ifndef PROCESS_H
#define PROCESS_H

#include <QProcess>

class Process : public QProcess
{
public:
    Process();
    void setWatchFiles(bool enable);

private:
    bool m_watchFiles;

};

#endif // PROCESS_H
