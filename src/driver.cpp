#include <QtCore>
#include "driver.h"
#include "globalsettings.h"
#include "builddatabase.h"

Driver::Driver(QObject *parent) :
    QObject(parent)
{

}

void Driver::setArguments(const QStringList &arguments)
{
    m_arguments = arguments;
    m_executableName = m_arguments.takeFirst();
}

void Driver::buildStatus()
{
    BuildDatabase buildDatabase;
    buildDatabase.writeReport();

    exit();
}

void Driver::selectCommand()
{
    if (m_arguments.isEmpty()) {
        qDebug() << "Usage: driver [option(s)] command(s)";
        qDebug() << "Available Commands:";
        qDebug() << "    - buildStatus: Creates a build database status report";
        qDebug() << "Available Options:";
        qDebug() << "    -workspace: Main data storage location. Builds, databases, web output etc go here. ";
    }

    const char *command = 0;

    // parse command line
    int index = 0;
    while (index < m_arguments.count()) {
        QString argument = m_arguments.at(index);
        ++index;
        if (argument == QLatin1String("-workspace")) {
            if (index >= m_arguments.count()) {
                qDebug() << "-workspace requires an argument";
                qApp->quit();
            }
            QString workspace = m_arguments.at(index);
            ++index;
            QString fullWorkspacePath = QDir::cleanPath(QDir::currentPath() + "/" + QDir::fromNativeSeparators(workspace));
            qDebug() << "setting workspace to" << QDir::toNativeSeparators(fullWorkspacePath);

            GlobalSettings::instance()->workspaceLocation = fullWorkspacePath;
        } else if (argument.toLower() == QLatin1String("-buildstatus")) {
            command = SLOT(buildStatus());
        } else {
            qDebug() << "Unknown option" << argument;
        }
    }

    if (command)
        QTimer::singleShot(0, this, command);
    else
        exit();
}

void Driver::exit()
{
    qDebug() << "Exiting.";
    ::exit(0); // Crash-only software!
}
