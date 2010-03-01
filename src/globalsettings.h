#ifndef GLOBALSETTINGS_H
#define GLOBALSETTINGS_H

class GlobalSettings
{
public:
    GlobalSettings();
    GlobalSettings *instance();

    bool mockSlowOperations;
private:

    static GlobalSettings *s_instance;
};

#endif // GLOBALSETTINGS_H
