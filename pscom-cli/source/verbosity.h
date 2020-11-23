#pragma once

#include <qlogging.h>

/**
 * @brief VerbosityHandler - http://doc.qt.io/qt-5/qtglobal.html#qInstallMessageHandler
 * ToDo: perhaps use this approach to format console output and handle ouput verbosity...
 */
void VerbosityHandler(QtMsgType type, const QMessageLogContext & context, const QString & message);
