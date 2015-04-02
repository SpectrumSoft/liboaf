/**
 * @file
 * @brief Реализация вспомогательных функций для работы с типами
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#include <OAF/TypeUtils.h>

#if defined (__GNUC__)
#include <cxxabi.h>
#endif

QString
OAF::interfaceName (const std::type_info& _i)
{
	const char* interface_name = _i.name ();
#if defined(__GNUC__)
	//
	// Далее GCC-зависимый код, поскольку GCC typeid возвращает внутреннее
	// представление имени класса.
	//
	// Предполагаем, что декодированное имя всегда короче чем зашифрованное + резерв.
	// Если это условие не выполнится, то программа здесь упадёт при попытке сделать
	// free для памяти, распределённой на стеке в функции __cxxabiv1::__cxa_demangle
	//
	// Под Widows не компилируется распределение на стеке массивов переменной длины,
	// поэтому распределяем заведомо достаточный объём.
	//
#if defined(Q_OS_WIN)
	//
	// Под Windows все компиляторы не разрешают создавать на стеке массивы с динамически
	// задаваемыми размерами (как минимум это справедливо для компиляторов gcc и MSVS)
	//
	char buf[1024];
#else
	char buf[strlen (interface_name) + 32];
#endif
	std::size_t len    = sizeof (buf);
	int         status = 0;

	__cxxabiv1::__cxa_demangle (interface_name, buf, &len, &status);
	return ((status == 0) ? buf : interface_name);
#else
	//
	// Под MSVC Windows имя классов манглируется как "struct XXXX"
	//
	return QString (interface_name).replace ("struct ", "").simplified ();
#endif
}
