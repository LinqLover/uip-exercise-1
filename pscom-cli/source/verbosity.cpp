
#include "verbosity.h"

#include <QCoreApplication>
#include <QString>
#include <QTextStream>


void VerbosityHandler(
    QtMsgType type,
    const QMessageLogContext & context,
    const QString & message
) {
#ifdef EXTREMELY_VERBOSE // TODO: Introduce command-line argument for this
    qSetMessagePattern("%{appname}: %{type}: %{if-category}%{category}: %{endif}%{message}\n  In: %{file}:%{line} in function %{function}\n  Backtrace:\n    %{backtrace separator=\"\n    \"}");
#else
    qSetMessagePattern("%{appname}: %{type}: %{if-category}%{category}: %{endif}%{message}");
#endif

    // streaming info messages on stderr does not do big harm, but logging
    // them to stdout would impede batch processing.
    auto stream = QTextStream(stderr);

    // TODO: Use color
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

    stream << qFormatLogMessage(type, context, message) << Qt::endl;
}
