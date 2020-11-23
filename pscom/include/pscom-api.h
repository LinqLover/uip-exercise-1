#pragma once

#include <QtCore/qglobal.h>

#if defined(PSCOM_LIBRARY)
#  define PSCOMSHARED_EXPORT Q_DECL_EXPORT
#else
#  define PSCOMSHARED_EXPORT Q_DECL_IMPORT
#endif
