#ifndef GLOBALSETTINGS_H
#define GLOBALSETTINGS_H

#include <QtGui>

class GlobalSettings
{
public:
    GlobalSettings();
    static GlobalSettings *instance();

    QString workspaceLocation;

    bool mockSlowOperations;
private:

    static GlobalSettings *s_instance;
};

#endif // GLOBALSETTINGS_H
