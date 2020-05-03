#ifndef SIMPLECM_EXPORT_H
#define SIMPLECM_EXPORT_H

#include <QtCore/QtGlobal>

#ifdef SIMPLECM_STATIC
#  define SIMPLECM_EXPORT
#else
#  if defined(BUILD_SIMPLECM_LIB)
#    define SIMPLECM_EXPORT Q_DECL_EXPORT
#  else
#    define SIMPLECM_EXPORT Q_DECL_IMPORT
#  endif
#endif // SIMPLECM_STATIC

#endif // SIMPLECM_EXPORT_H
