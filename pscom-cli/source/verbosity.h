#pragma once

#include <qlogging.h>


enum VerbosityLevel {
    Error,
    Warning,
    Info,
    Debug,
    Trace
};

VerbosityLevel getVerbosityLevel();
void setVerbosityLevel(VerbosityLevel);

/**
 * @brief VerbosityHandler - http://doc.qt.io/qt-5/qtglobal.html#qInstallMessageHandler
 * ToDo: perhaps use this approach to format console output and handle ouput verbosity...
 */
void VerbosityHandler(QtMsgType type, const QMessageLogContext & context, const QString & message);
