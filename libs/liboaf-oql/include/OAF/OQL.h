/**
 * @file
 * @brief Интерфейс парсера OQL-запросов
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#ifndef __OQL_H
#define __OQL_H

#include <OAF/OafOqlGlobal.h>

#include <QString>
#include <QtCore>

#include <idl/IUnknown.h>
#include <idl/IQuery.h>

/**
 * @brief Парсер (O)bject (Q)uery (L)anguage
 */
namespace OQL
{
	/**
	 * @brief Разбор OQL запроса и преобразование его в набор выражений
	 *
	 * @param[in] _query   текстовое представление запроса для разбора
	 * @param[in] _factory фабрика функций
	 */
	OAFOQL_EXPORT OAF::URef<OAF::IQuery> parseQuery (const QString& _query, OAF::IFunctionFactory* _factory);
}

#endif /* __OQL_H */
