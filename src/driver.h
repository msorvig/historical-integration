#ifndef DRIVER_H
#define DRIVER_H

#include <QtCore>

class Driver : public QObject
{
Q_OBJECT
public:
    explicit Driver(QObject *parent = 0);
    void setArguments(const QStringList &arguments);

signals:

public slots:
    void selectCommand();
    void buildStatus();
    void exit();


private:
    QString m_executableName;
    QStringList m_arguments;
};

#endif // DRIVER_H
