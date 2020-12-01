#include "verbosity.h"

#include "support.h"

#include <QCoreApplication>
#include <QString>
#include <QTextStream>


#define VERBOSITY_MIN VerbosityLevel::Error
#define VERBOSITY_MAX VerbosityLevel::Trace

VerbosityLevel verbosityLevel =
#if defined QT_NO_DEBUG || \
        !(defined WE_WANT_TO_CONFUSE_OUR_USERS_WITH_VARIABLE_VERBOSITIES)
    VerbosityLevel::Info
#else
    VerbosityLevel::Debug
#endif
;
QString _messagePattern = nullptr;

VerbosityLevel getVerbosityLevel() {
    return verbosityLevel;
}

QString getMessagePattern() {
    if (_messagePattern == nullptr) {
        QString pattern =
            "%{appname}: %{type}: "
            "%{if-category}%{category}: %{endif}"
            "%{message}";
        if (verbosityLevel >= VerbosityLevel::Trace) {
            pattern +=
                "\nIn: %{file}:%{line} in function %{function}"
                "\n  Backtrace:"
                "\n    %{backtrace separator=\"\n    \"}";
        }
        qSetMessagePattern(_messagePattern = pattern);
    }
    return _messagePattern;
}

void setVerbosityLevel(VerbosityLevel level) {
    if (level < VERBOSITY_MIN) {
        setVerbosityLevel(VERBOSITY_MIN);
        return;
    } elif (static_cast<int>(level) > static_cast<int>(VERBOSITY_MAX)) {
        setVerbosityLevel(VERBOSITY_MAX);
        return;
    }
    verbosityLevel = level;
    _messagePattern = nullptr;
}

void VerbosityHandler(
    QtMsgType type,
    const QMessageLogContext & context,
    const QString & message
) {
    getMessagePattern();

    // streaming info messages on stderr does not do big harm, but logging
    // them to stdout would impede batch processing.
    auto stream = QTextStream(stderr);

    // TODO: Use color
    switch (type) {
    case QtDebugMsg:
        if (verbosityLevel < VerbosityLevel::Debug) {
            return;
        }
        break;
    case QtInfoMsg:
        if (verbosityLevel < VerbosityLevel::Info) {
            return;
        }
        break;
    case QtWarningMsg:
        if (verbosityLevel < VerbosityLevel::Warning) {
            return;
        }
        break;
    case QtCriticalMsg:
    case QtFatalMsg:
        if (verbosityLevel < VerbosityLevel::Error) {
            return;
        }
        break;
    }

    stream << qFormatLogMessage(type, context, message) << Qt::endl;
}
