/**
 * @file
 * @brief Реализация перевода строк через механизм Qt в С библиотеке libgit2
 * @author Kirill Suetnov <kyrie@yandex.ru>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#include <QCoreApplication>

extern "C" const char*
qt_translate (const char* _str)
{
	QByteArray ba = QCoreApplication::translate ("libgit2", _str, "libgit2", QCoreApplication::UnicodeUTF8).toUtf8 ();

	return ba.data ();
}
