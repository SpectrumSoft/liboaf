/**
 * @file
 * @brief Реализация перевода строк через механизм Qt в С библиотеке libgit2
 * @author Kirill Suetnov <kyrie@yandex.ru>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#include <QtCore>

#include "libgit2_intl.h"

/**
 * @brief Кэш переводов
 *
 * Проблема в том, что вернуть необходимо указатель на где-то сохранённые данные. Для
 * того, чтобы не заниматься постоянно распределением и освобождением памяти кэшируем
 * переведённые данные и возвращаем их из кэша. Если в кэше данных нет, то запрашиваем
 * перевод и сохраняем результат в кэше.
 *
 * При этом ключём является адрес исходной строки. Это нормально, так как все строки
 * для перевода распределены статически и их положение в памяти в течении работы программы
 * не меняется.
 */
static QMap<const char*, QByteArray> __translation_cache;

extern "C" const char*
qt_translate (const char* _str)
{
	//
	// Ищем перевод в кэше
	//
	QMap<const char*, QByteArray>::const_iterator t = __translation_cache.find (_str);

	//
	// Если перевода нет, то загружаем его в кэш
	//
	if (t == __translation_cache.end ())
		t = __translation_cache.insert (_str, QCoreApplication::translate ("libgit2", _str, "libgit2").toUtf8 ());

	return t.value ().data ();
}
