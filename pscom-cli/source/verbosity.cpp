
#include "verbosity.h"

#include <QString>
#include <QByteArray>


/* ToDo: this is only placeholder implementation showcasing how messages using qFatal, qCritical, qWarning, qInfo, and
 * qDebug could be handled. This might be a good starting point for verbosity handling and arbitrary output formatting.
 */
void VerbosityHandler(QtMsgType type, const QMessageLogContext & /*context*/, const QString & message)
{
    auto prefix = QByteArray();
    auto stream = stdout;

    switch (type) {
    case QtDebugMsg:
        break;
    case QtInfoMsg:
        break;
    case QtWarningMsg:
        break;
    case QtCriticalMsg:
        break;
    case QtFatalMsg:
        break;
    }

    const auto local = message.toLocal8Bit();
    fprintf(stream, "%s\n", local.constData());
}
