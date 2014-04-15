#ifndef SIMPLECM_EXPORT_H
#define SIMPLECM_EXPORT_H

#include <QtCore/QtGlobal>

#if defined(SIMPLECM_LIBRARY)
#define SIMPLECM_EXPORT Q_DECL_EXPORT
#else
#define SIMPLECM_EXPORT Q_DECL_IMPORT
#endif

#endif
