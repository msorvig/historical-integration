#include "webgenerator.h"
#include "reports.h"

/*  The WebGenerator instanciates html templates and is mostly
    used to mix in the required Javascript modules. WebGenerator
    provides several different instantiation modes.

    The main replacement done is to replace "[[Foo]]" with a
    script tag loading "foo.js". In addition, "[[JsonData]]"
    is replaced with a script declaring a variable containing
    the provided json data. (see the ::instantiate* functions.)

    Two instantiation modes are supported:

    - instantiateSelfContained: Creates a self-contained html
    file that includes the json data and javascript modules.
    Note that the template file and javascript modules are
    loaded from resources (prefix :html/), which makes the
    instantiation process itself self-contained.

    -instantiateSelfContainedDev: Creates a self-contained html
    with json data. Scripts are linked in (in other words, loaded
    by the web browser). This mode is useful for javascript
    development, since you can edit&refresh directly and skip the
    instantiation process.
*/


WebGenerator::WebGenerator()
{

}

QByteArray WebGenerator::instantiateSelfContained(const QString &templateFileName,
                                                  const QByteArray &jsonData)
{
    QByteArray html = readFile(":html/" + templateFileName);
    return performReplacements(html, jsonData, QString(), SelfContained);
}

QByteArray WebGenerator::instantiateSelfContainedDev(const QString &templateFileName,
                                                     const QByteArray &jsonData,
                                                     const QString &pathToSrc)
{
    QByteArray html = readFile(pathToSrc + "/" + templateFileName);
    return performReplacements(html, jsonData, pathToSrc, SelfContainedDev);
}

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

        qDebug() << "tag offset"<< offset << "lenth" << length << tag;
        QByteArray replacement;
        if (tag == "[[JsonData]]") {
            replacement = "<script type='text/javascript'> var jsonData = " + jsonData + "</script>";
        } else if (mode == SelfContained) {
            replacement = "<script type='text/javascript'> " + readFile(":html/" + tag + ".js") + "</script>";
        } else if (mode == SelfContainedDev) {
            replacement = "<script type='text/javascript' src ='" + pathToSrc.toLocal8Bit() + "/" + name + ".js'></script>";
        }

        out.replace(offset, length, replacement);
        offset = out.indexOf("[[", offset - length + replacement.length());
    }

    return out;
}
