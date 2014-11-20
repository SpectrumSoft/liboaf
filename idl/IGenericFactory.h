/**
 * @file
 * @brief Фабрика объектов
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#ifndef __IGENERIC_FACTORY_H
#define __IGENERIC_FACTORY_H

//
// Функции createObject и dllCanUnloadNow для MSVS-компилятора должны быть
// объявлены с этим ключевым словом. Для остальных компиляторов преобразуем
// данное объявление в пустую строку
//
#ifndef _MSC_VER
#define __declspec(x)
#endif

#include <QString>

#include <idl/IInterface.h>
#include <idl/IUnknown.h>

namespace OAF
{
	/**
	 * @brief Фабрика объектов
	 */
	struct IGenericFactory : virtual public OAF::IInterface
	{
		/**
		 * @brief Получить ссылку на объект заданного класса
		 *
		 * Реализация фабрики не ограничена в способе создания объектов. Это может быть как
		 * создание нового объекта, так и извлечение ранее созданного объекта из кэша или
		 * глобальной переменной (sigleton).
		 *
		 * @return Ссылка на созданный объект
		 */
		virtual OAF::URef<OAF::IUnknown> createObject (const QString& _cid) = 0;
	};
}

#endif /* __IGENERIC_FACTORY_H */
