
#include <pscom.h>

#include <QCoreApplication>
#include <QCommandLineOption>
#include <QCommandLineParser>
#include <QDebug>
#include <QRegExp>

#include <QTextStream>

#include "verbosity.h"


/* PLEASE NOTE
 * - be carful using the pscom library, it may irreversible delete actual data if used without care.
 * - try to rely on pscom library exclusively in order to fulfill the user stories / tasks.
 * - use qt only to drive the library and control command language aspects and CLI related features, e.g.,
 *   progress, logging, verbosity, undo, help, etc.
 * - prefer qDebug, qInfo, qWarning, qCritical, and qFatal for verbosity and console output.
 */


/* TODOs
 * - adjust the application version using QCoreApplication::setApplicationVersion before creating an application object.
 * - make yourself familiar with semantic versioning, e.g., here https://semver.org/, since it can useful for CLIs.
 *   For example, it allows to detach the version number of the CLI from the version of the library used and may
 *   retain major version bumps (same CLI for different capabilities or naming of the used library).
 */


int main(int argc, char *argv[])
{
    qInstallMessageHandler(VerbosityHandler);

    // Setting the application name is not required, since, if not set, it defaults to the executable name.
    // QCoreApplication::setApplicationName("pscom-cli");

    QCoreApplication::setApplicationVersion("0.0.0");

    QCoreApplication a(argc, argv);


    /* ToDo - have fun! */

	const auto vi = pscom::vi();
    QTextStream stream(stdout);

    stream << QObject::tr("version: ") << vi << Qt::endl;
    stream.flush();

    return a.exec();
}
