#include "testrunner.h"
#include "log.h"

TestRunner::TestRunner(const QString &testPath, const QString &qtBuildPath)
{
    qDebug() << "running test at" << testPath << "using qt at" << qtBuildPath;

    findTestExecutable(testPath);
    m_qtLibsPath = findQtLibs(qtBuildPath);

    m_process.setWorkingDirectory(m_testWorkDir);
    m_process.addEnvironmentPath(m_qtLibsPath);

    /* Perhaps try to remove all other Qt paths from "Path"
       to make sure we use our qtPath or fail.
    QStringList env = QProcess::systemEnvironment();
    for (QStringList::iterator it = env.begin(); it != env.end(); ++it) {
        if (it->startsWith("Path")) {
            qDebug() << "path";
            QStringList pathParts = it->split(";");
            qDebug() << "pathParts" << pathParts;

            it->replace(QRegExp("^PATH=(.*)", Qt::CaseInsensitive),
                                 "PATH=" + qtPath + ";\\1");

        }
    }
    m_process.setEnvironment(env);

    */

    m_process.addEnvironmentPath(qtBuildPath);
}

bool TestRunner::isBuilt()
{
    return QFile(m_testExecutablePath).exists();
}

void TestRunner::findTestExecutable(const QString &testPath)
{
    QDir dir(QDir(testPath).absolutePath());
    QString dirName = dir.dirName();
    QString executable = "tst_" + dirName;
#ifdef Q_WS_MAC
    executable = executable + ".app/Contents/MacOS/" + executable;
#elif defined(Q_OS_WIN32)
    executable = "/release/" + executable + ".exe";
#endif
    QString fullPath = QDir::cleanPath(testPath + "/" + executable);
    QFile f(fullPath);

    if (f.exists() == false) {
        Log::addError("TestRunner: test executable does not exist: " + m_testExecutablePath);
    }

#if 0
    qDebug() << "test name" << dirName;
    qDebug() << "test exe" << executable;
    qDebug() << "full path" << fullPath;
    qDebug() << "exists?" << f.exists();
#endif
    m_testWorkDir = testPath;
    m_testExecutablePath = fullPath;
}

QString TestRunner::findQtLibs(const QString &qtPath)
{
    return QDir::cleanPath(qtPath + "/lib");
}

int TestRunner::callTestExecutableWait(const QStringList &arguments, int timeout)
{
    QTime time;
    time.start();

    return time.elapsed();
}

QStringList TestRunner::testFunctions()
{
    m_process.start(m_testExecutablePath, QStringList() << "-functions");
    bool ok = m_process.waitForFinished(1000);
    if (!ok) {
        Log::addError("TestRunner::testFunctions failed to run " + m_testExecutablePath + " ");
    }

    if (m_process.exitStatus() == QProcess::CrashExit)
         Log::addError("TestRunner::testFunctions crashed " + m_testExecutablePath + " ");

    QString output = m_process.readAllStandardOutput();
    return parseTestFunctionsOutput(output);
}

QStringList TestRunner::testData(const QString &testFunction)
{
    const QString nonExistantData = "aaaaaaaaaaaaa";
    m_process.start(m_testExecutablePath, QStringList() << (testFunction + ":" + nonExistantData));
    bool ok = m_process.waitForFinished(1000);
    if (!ok) {
        Log::addError("TestRunner::testFunctions failed to run " + m_testExecutablePath + " ");
    }

   // if (m_process.exitStatus() == QProcess::CrashExit)
   //     Log::addError("TestRunner::testFunctions crashed " + m_testExecutablePath + " ");

    QString output = m_process.readAllStandardOutput();
    QString anchor = "Unknown testdata for function";
    int testDataStartIndex = output.indexOf(anchor);
    if (testDataStartIndex == -1)
        return QStringList();

    return parseDataTagOutput(output);
}

int TestRunner::startupTime()
{
    return runTestFunction("thistestfuncitondoesnotexist").runTime;
}

TestRunResult TestRunner::runTestExecutable(const QStringList &arguments)
{
    QStringList argumentsCopy = arguments;
    argumentsCopy << "-xml";

    TestRunResult result;
    QTime time;
    time.start();
    m_process.start(m_testExecutablePath, argumentsCopy);
    result.ranOk = m_process.waitForFinished();
    result.runTime = time.elapsed();
    result.stdOut = m_process.readAllStandardOutput();
    result.stdErr = m_process.readAllStandardError();

    if (!result.ranOk) {
        Log::addError("TestRunner::testFunctions failed to run " + m_testExecutablePath + " ");
    }
    return result;
}

TestRunResult TestRunner::runTestFunction(const QString &test, const QString &data)
{
    QString argument = test;
    if (data.isEmpty() == false) {
        argument += ":" + data;
    }

    return runTestExecutable(QStringList() << argument);
}

TestRunResult TestRunner::runTestFunctions(const QStringList &tests)
{
    return runTestExecutable(tests);
}

TestRunResult TestRunner::runAllTestFunctions()
{
    return runTestExecutable(QStringList());
}

QStringList TestRunner::parseTestFunctionsOutput(const QString &output)
{
    return simplifyStrings(output.split("()", QString::SkipEmptyParts));
}

// parses the output qtestlib produces when listing the
// data tags for a test function.
QStringList TestRunner::parseDataTagOutput(const QString &output)
{
    QString anchor = "Available testdata:";
    int anchorStartIndex = output.indexOf(anchor);
    int dataStartIndex = anchorStartIndex + anchor.count();
    int passIndex = output.indexOf("PASS", dataStartIndex);
    int relevantSize = passIndex - dataStartIndex;

    QString relevantOutput = output.mid(dataStartIndex, relevantSize);
    QStringList testData = relevantOutput.split("\n");
    return simplifyStrings(testData);
}

 // map(testFunctions, simplified).filter(!isEmpty);
QStringList TestRunner::simplifyStrings(const QStringList &strings)
{
    QStringList simplifiedStrings;
    foreach (const QString &string, strings) {
        QString simplified = string.simplified();
        if (simplified.isEmpty() == false)
            simplifiedStrings.append(simplified);
    }
    return simplifiedStrings;
}
