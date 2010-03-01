#include "buildlogic.h"
#include "buildbot.h"
#include "ui_mainwindow.h"
#include <QtCore>

BuildLogic::BuildLogic(Ui::MainWindow *ui)
:ui(ui)
{

}

void BuildLogic::initializeWorkDirectory()
{
    repositoryPath = QDir::fromNativeSeparators(ui->lineEdit_gitRepository->text());
    workPath = QDir::fromNativeSeparators(ui->lineEdit_workDirectory->text());
    qDebug() << "initializeWorkDirectory" << repositoryPath << workPath;
    if (repositoryPath.isEmpty() || workPath.isEmpty()) {
        return;
    }

    GitClient client(repositoryPath, workPath);
    client.sync();

    openDatabase("database");

    //emit workDirectoryInitialized();
    ui->pushButton_build->setEnabled(true);
    qDebug() << "initializeWorkDirectory done!";
}

void BuildLogic::buildQtRevisions()
{
    ProjectHistoryBuilder builder(repositoryPath, workPath);
    builder.build(ui->spinBox_revisionCount->value());
}
