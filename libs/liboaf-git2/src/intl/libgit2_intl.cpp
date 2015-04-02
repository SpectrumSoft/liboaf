/**
 * @file
 * @brief Реализация перевода строк через механизм Qt в С библиотеке libgit2
 * @author Kirill Suetnov <kyrie@yandex.ru>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#include <QtCore>

#include "util.h"
#include "libgit2_intl.h"

extern "C" char*
qt_translate (const char* _str)
{
	return git__strdup (QCoreApplication::translate ("libgit2", _str, "libgit2").toUtf8 ().data ());
}
