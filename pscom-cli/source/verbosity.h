#pragma once
// Logging module.

#include <qlogging.h>


enum VerbosityLevel {
    Error,
    Warning,
    Info,
    Debug,
    Trace
};

void setInteractive(bool interactive = true);
VerbosityLevel getVerbosityLevel(void);
void setVerbosityLevel(VerbosityLevel);

/**
 * @brief VerbosityHandler - http://doc.qt.io/qt-5/qtglobal.html#qInstallMessageHandler
 */
void VerbosityHandler(
    QtMsgType type,
    const QMessageLogContext & context,
    const QString & message);
