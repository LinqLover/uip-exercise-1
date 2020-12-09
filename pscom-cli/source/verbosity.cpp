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
bool _interactive = false;
QString _messagePattern = nullptr;

void setInteractive(bool interactive) {
    _interactive = interactive;
    _messagePattern = nullptr;
}

VerbosityLevel getVerbosityLevel() {
    return verbosityLevel;
}

QString getMessagePattern() {
    if (_messagePattern == nullptr) {
        QString pattern =
            "%{appname}: %{type}: "
            "%{if-category}%{category}: %{endif}";
        if (_interactive) {
            pattern = "\e[4m" + pattern + "\e[24m";
        }
        pattern += "%{message}";
        if (verbosityLevel >= VerbosityLevel::Trace) {
            pattern += QObject::tr(
                "\nIn: %1:%2 in function %3"
                "\n  Backtrace:"
                "\n    %4"
            ).arg(
                "%{file}", "%{line}", "%{function}",
                "%{backtrace separator=\"\n    \"}");
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
    QString color = nullptr;

    switch (type) {
        case QtDebugMsg:
            if (verbosityLevel < VerbosityLevel::Debug) {
                return;
            }
            color = "\033[;;90m"; // grey
            break;
        case QtInfoMsg:
            if (verbosityLevel < VerbosityLevel::Info) {
                return;
            }
            color = "\033[;;94m"; // blue
            break;
        case QtWarningMsg:
            if (verbosityLevel < VerbosityLevel::Warning) {
                return;
            }
            color = "\033[;1;33m"; // bold yellow
            break;
        case QtCriticalMsg:
            color = "\033[;;31m"; // red
            if (verbosityLevel < VerbosityLevel::Error) {
                return;
            }
            break;
        case QtFatalMsg:
            color = "\033[;1;31m"; // bold red
            break;
    }

    if (_interactive && color != nullptr) {
        stream << color;
    }
    stream << qFormatLogMessage(type, context, message) << Qt::endl;
    if (_interactive && color != nullptr) {
        stream << "\033[0m";
    }
}
