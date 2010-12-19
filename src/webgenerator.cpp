#include "webgenerator.h"
#include "reports.h"
#include "log.h"

/*  The WebGenerator instanciates html templates and is mostly
    used to mix in the required Javascript modules. WebGenerator
    provides several different instantiation modes.

    The main replacement done is to replace "[[Foo]]" with a
    script tag loading "foo.js". In addition, "[[JsonData]]"
    is replaced with a script declaring a variable containing
    the provided json data. (see the ::instantiate* functions.)

    Three instantiation modes are supported:

    - instantiateSelfContained: Creates a self-contained html
    file that includes the json data and inlined javascript modules.
    Note that the template file and javascript modules are
    loaded from resources (prefix :html/), which makes the
    instantiation process itself self-contained.

    -instantiateSelfContainedDev: Creates a self-contained html
    with json data. Scripts are linked in (in other words, loaded
    by the web browser). This mode is useful for javascript
    development, since you can edit&refresh directly and skip the
    instantiation process.

    -[TODO] instantiateStandAlone: Creates a stand-alone html
    file that loads JOSN asynchronously from a server. Javascript
    modules are linked in.
*/

WebGenerator::WebGenerator()
{

}

QByteArray WebGenerator::instantiateSelfContained(const QString &templateFileName,
                                                  const QByteArray &jsonData)
{
    QString resourceFileName = ":html/" + templateFileName;

    QByteArray html = readFile(resourceFileName);
    return performReplacements(html, jsonData, QString(), SelfContained);
}


#define STRING1(arg) #arg
#define STRING(arg) STRING1(arg)
QByteArray WebGenerator::instantiateSelfContainedDev(const QString &templateFileName,
                                                     const QByteArray &jsonData)
{
    QByteArray html;
    const QString templateInCurrentDirectory = QDir::currentPath() + "/" + templateFileName;

    const QString pathToSrc = STRING(SRCPATH); // from buildbot.pri

    qDebug() << "testing" << templateInCurrentDirectory;

    if (QFile::exists(templateInCurrentDirectory)) {
        html = readFile(templateInCurrentDirectory);
    } else {
        html = readFile(pathToSrc + "/" + templateFileName);
    }

    if (html.isEmpty()) {
        Log::addError(QString("WebGenerator::instantiateSelfContainedDev: "
                               "no file found for" + templateFileName));
        exit(0);
    }


    return performReplacements(html, jsonData, pathToSrc, SelfContainedDev);
}

// find tags (which looks like [[Tag]]), replace "Tag"
// with content. Depending on the mode, the inserted
// content can either be a link to a js/css file, or
// the file content itself.
//
// [[JsonData]] is a special tag and gets replaced with
// the content of the passed jsonData byte array.
//
QByteArray WebGenerator::performReplacements(const QByteArray &html, const QByteArray &jsonData,
                                             const QString &pathToSrc, Mode mode)
{
    QByteArray out = html;
    int offset = html.indexOf("[[");
    while (offset >= 0) {
        //qDebug() << "start offset" << offset;
        int length = out.indexOf("]]", offset) - offset + 2;
        //qDebug() << "lenght" << length << "index of end tag" << html.indexOf("]]", offset);
        //qDebug() << out;
        if (length <=4)
            break;

        QByteArray tag = out.mid(offset, length);
        QByteArray name = tag.right(tag.length() -2).toLower();
        name.chop(2);

//         qDebug() << "tag offset"<< offset << "lenth" << length << tag;
        QByteArray replacement;
        if (tag == "[[JsonData]]") {
            replacement = "<script type='text/javascript'> var jsonData = " + jsonData + "</script>";
        } else if (mode == SelfContained) {
            if (name.endsWith("js")) {
                replacement = "<script type='text/javascript'> "
                            + readFile(":html/" + name) + "</script>";
            } else {
                replacement = "<style type'text/css'><!-- "
                            + readFile(":html/" + name) + " --></style>";
            }
        } else if (mode == SelfContainedDev) {
            if (name.endsWith("js")) {
                replacement = "<script type='text/javascript' src ='"
                            + QDir::cleanPath(pathToSrc).toLocal8Bit()
                            + "/" + name + "'></script>";
            } else {
                replacement = "<link rel='stylesheet' type='text/css' href= '"
                            + QDir::cleanPath(pathToSrc).toLocal8Bit()
                            + "/" + name + "'/>";
            }
        }

        out.replace(offset, length, replacement);
        offset = out.indexOf("[[", offset - length + replacement.length());
    }

    return out;
}
