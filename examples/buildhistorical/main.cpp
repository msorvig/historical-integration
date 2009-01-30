#include <buildbot.h>
#include <QtCore>

int main(int arc, char **argv)
{
    HistoricalBuildOptions options;
    options.basePath = "/Users/msorvig/build/daily";
    options.stagePath = "/Users/msorvig/build/daily/stage/";
    options.sourcePath = "/Users/msorvig/code/buildbot/qt"; 
    options.storeFullCopy = false;
    options.commitCount = 500;
    options.dryRun = false;

    if (QDir(options.basePath).exists() == false) {
        qDebug() << "path not found:" << options.basePath;
        qDebug() << "This example requires custom configuration! Edit main.cpp to set paths";
    }

    buildHistorical(options);
}
