#include "globalsettings.h"

GlobalSettings::GlobalSettings()
{
    mockSlowOperations = false;
}

GlobalSettings *GlobalSettings::instance()
{
    if (!GlobalSettings::s_instance)
        GlobalSettings::s_instance = new GlobalSettings;
    return s_instance;
}

GlobalSettings *GlobalSettings::s_instance = 0;
