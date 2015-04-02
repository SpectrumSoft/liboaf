/**
 * @file
 * @brief Поисковые интерфейсы
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#ifndef __ISEARCH_H
#define __ISEARCH_H

#include <QtCore>

#include <idl/IInterface.h>

namespace OAF
{
	/**
	 * @brief Интерфейс полнотекстового поиска
	 */
	struct IMatchable : virtual public OAF::IInterface
	{
		/**
		 * @brief Определение попадание объекта под условия поиска
		 *
		 * @param[in] _pattern строка для сравнения
		 * @param[in] _cs      флаг Qt::CaseSensitivity
		 *
		 * @return Значение больше 0 - объекта попадает под условия поиска
		 *
		 */
		virtual int match (const QString& _pattern, Qt::CaseSensitivity _cs = Qt::CaseInsensitive) = 0;
	};
}

#endif /* __ISEARCH_H */
