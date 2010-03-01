#ifndef BUILDLOGIC_H
#define BUILDLOGIC_H

#include <QObject>
#include "mainwindow.h"

class BuildLogic : public QObject
{
Q_OBJECT
public:
    BuildLogic(Ui::MainWindow *ui);
    Ui::MainWindow *ui;
    QString repositoryPath;
    QString workPath;

    public slots: void initializeWorkDirectory();
    signals: void workDirectoryInitialized();

    public slots:
    void buildQtRevisions();
public:


};

#endif // BUILDLOGIC_H
