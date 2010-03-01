#ifndef MAINLINECHANGEDETECTOR_H
#define MAINLINECHANGEDETECTOR_H

class MainlineChangeDetector
{
public:
    MainlineChangeDetector();
    void setTestMode(bool enable);
    void detectChanges();
private:
    bool m_testMode;
};

#endif // MAINLINECHANGEDETECTOR_H
