/**
 * @file
 * @brief Вспомогательные функции для работы с типами
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#ifndef __OAF_TYPE_UTILS_H
#define __OAF_TYPE_UTILS_H

#include <OAF/OafCoreGlobal.h>

#include <typeinfo>

#ifndef _MSC_VER
#include <cxxabi.h>
#endif

#include <QString>

namespace OAF
{
	/**
	 * @brief Извлечение имени интерфейса из информации о типе
	 */
	OAFCORE_EXPORT QString interfaceName (const std::type_info& _i);

	/**
	 * @brief Преобразование C++ интерфейса в его имя
	 */
	template<class _Interface>
	QString interfaceName ()
	{
		return interfaceName (typeid (_Interface));
	};
}

#endif /* __OAF_TYPE_UTILS_H */
