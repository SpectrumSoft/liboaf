/**
 * @file
 * @brief Определения, необходимые для сборки библиотеки под Windows
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#ifndef __OAFOQLGLOBAL_H
#define __OAFOQLGLOBAL_H

#include <QtCore/QtGlobal>

#if defined(OAFOQL_LIBRARY)
#  define OAFOQL_EXPORT Q_DECL_EXPORT
#else
#  define OAFOQL_EXPORT Q_DECL_IMPORT
#endif

#endif /* __OAFOQLGLOBAL_H */
