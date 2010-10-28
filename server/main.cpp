#include <QtCore>
#include <watchdog.h>

int main(int argc, char **argv)
{
    QCoreApplication app(argc, argv);

    WatchDog watchdog;

    bool mortensDevMachine = true;
    if (mortensDevMachine) {
        watchdog.setBasePath(QDir::currentPath() + "/yard");
        watchdog.setRepositoryUrl("c:/depot/qtBuildBot");
        watchdog.setWatchedExecutable(QDir::currentPath() + "/yard/server_impl");
    } else {

    }

    watchdog.run();
}
