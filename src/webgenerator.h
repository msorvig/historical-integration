#ifndef WEBGENERATOR_H
#define WEBGENERATOR_H

#include <QObject>

void writeFile(const QString &fileName, const QByteArray &contents);

class WebGenerator
{
public:
    WebGenerator();

    QByteArray instantiateSelfContained(const QString &templateFileName,
                                        const QByteArray &jsonData);
    QByteArray instantiateSelfContainedDev(const QString &templateFileName,
                                           const QByteArray &jsonData);

    enum Mode { SelfContained, SelfContainedDev };
    QByteArray performReplacements(const QByteArray &html,
                                   const QByteArray &jsonData,
                                   const QString &pathToSrc, Mode mode);
};

#endif // WEBGENERATOR_H
