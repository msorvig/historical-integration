#include <QtCore>
#include <driver.h>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    qDebug() << "Qt BuildBot Driver Program";

    Driver driver;
    driver.setArguments(app.arguments());

    QTimer::singleShot(0, &driver, SLOT(selectCommand()));

    app.exec();
}
