/**
 * @file
 * @brief Реализация перевода строк через механизм Qt в С библиотеке libgit2
 * @author Kirill Suetnov <kyrie@yandex.ru>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#ifndef __LIBGIT2_INTL_H
#define __LIBGIT2_INTL_H

#ifdef __cplusplus
extern "C"
{
#endif

char* qt_translate (const char * _str);

#ifdef __cplusplus
}
#endif

#endif /* __LIBGIT2_INTL_H */