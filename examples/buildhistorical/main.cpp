#include <buildbot.h>
#include <QtCore>

int main(int arc, char **argv)
{
    ProjectHistoryBuilder builder;
    builder.sourceUrl = "git@scm.dev.troll.no:personal/msorvigs-qspellchecker.git";
    builder.build();


//    options.basePath = "../work";
//    options. = "/Users/msorvig/code/buildbot/qt";
//    options.storeFullCopy = false;
//    options.commitCount = 500;
//    options.dryRun = false;

//    if (QDir(builder.basePath).exists() == false) {
//        qDebug() << "path not found:" << options.basePath;
//        qDebug() << "This example requires custom configuration! Edit main.cpp to set paths";
//    }

 //   buildHistorical(options);
    return 0;
}
