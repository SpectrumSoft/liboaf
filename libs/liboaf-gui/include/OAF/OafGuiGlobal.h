/**
 * @file
 * @brief Определения, необходимые для сборки библиотеки под Windows
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#ifndef __OAFGUIGLOBAL_H
#define __OAFGUIGLOBAL_H

#include <QtCore/QtGlobal>

#if defined(OAFGUI_LIBRARY)
#  define OAFGUI_EXPORT Q_DECL_EXPORT
#else
#  define OAFGUI_EXPORT Q_DECL_IMPORT
#endif

#endif /* __OAFGUIGLOBAL_H */
