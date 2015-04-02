/**
 * @file
 * @brief Интерфейс компонентной подсистемы OAF/Qt4
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#ifndef __OAF_H
#define __OAF_H

#include <QtCore>

#include <idl/IUnknown.h>
#include <idl/IPropertyBag.h>
#include <idl/IMoniker.h>
#include <idl/IQuery.h>

#include <OAF/OafGlobal.h>
#include <OAF/TypeUtils.h>

/**
 * @brief Object Activation Framework (OAF/Qt4)
 */
namespace OAF
{
	/**
	 * @brief Получить описание класса по его CID
	 */
	OAF_EXPORT const OAF::IPropertyBag* getClassInfo (const QString& _cid);

	/**
	 * @brief Отбор классов, соответствующих заданным критериям в заданном порядке
	 *
	 * @param[out] _out список CID, упорядоченный в соответствии с @a _req
	 * @param[in]  _req требования, записанные в виде OQL-запроса
	 *
	 * @return количество CID подходящих OAF-классов
	 */
	OAF_EXPORT std::size_t query (QStringList& _out, const QString& _req);

	/**
	 * @brief Создание объекта по идентификатору класса
	 *
	 * @param[in] _cid идентификатор класса
	 */
	OAF_EXPORT OAF::URef<OAF::IUnknown> createFromCID (const QString& _cid);

	/**
	 * @brief Создать моникер с заданными параметрами
	 */
	OAF_EXPORT OAF::URef<OAF::IMoniker> createMoniker (OAF::IMoniker* _left, const QString& _prefix, const QString& _suffix);

	/**
	 * @brief Преобразование имени объекта в цепочку моникеров
	 */
	OAF_EXPORT OAF::URef<OAF::IMoniker> parseName (const QString& _name);

	/**
	 * @brief Получение имени объекта из заданной цепочки моникеров
	 */
	OAF_EXPORT QString unparseName (OAF::IMoniker* _moniker);

	/**
	 * @brief Создание объекта по его имени и требуемому интерфейсу
	 *
	 * Выполняется преобразование имени в цепочку моникеров, создание объекта и
	 * преобразование его к нужному интерфейсу.
	 */
	template<class _Interface>
	OAF::URef<_Interface> createFromName (const QString& _name)
	{
		//
		// Создаём цепочку моникеров по заданному имени
		//
		if (OAF::URef<OAF::IMoniker> m = OAF::parseName (_name))
		{
			//
			// С её помощью создаём объект с нужным интерфейсом
			//
			if (OAF::URef<OAF::IUnknown> o = m->resolve (interfaceName<_Interface> ()))
			{
				//
				// И возвращаем ссылку на заданный интерфейс
				//
				if (OAF::URef<_Interface> i = o.queryInterface<_Interface> ())
					return i;
				else
					qWarning ("Can't resolve %s interface for class %s", qPrintable (OAF::interfaceName<_Interface> ()), qPrintable (OAF::interfaceName (typeid (*o))));
			}
			else
				qWarning ("Can't resolve moniker %s to interface %s", qPrintable (_name), qPrintable (interfaceName<_Interface> ()));
		}
		else
			qWarning ("Can't parse moniker name %s", qPrintable (_name));

		return OAF::URef<_Interface> ();
	};

	/**
	 * @brief Выгрузить из памяти все неиспользуемые компоненты
	 */
	OAF_EXPORT void unloadUnusedLibraries ();

	/**
	 * @brief Выгрузить из памяти все описания классов
	 */
	OAF_EXPORT void unloadClassInfo ();

	/**
	 * @brief Фабрика функций для OAF
	 *
	 * Данная фабрика вытащена наружу, чтобы её можно было использовать в качестве
	 * основы и не перепрограммировать каждый раз все функции заново при необходимости
	 * реализовать дополнительные функции OQL в какой-либо из подсистем
	 */
	OAF_EXPORT OAF::IFunctionFactory* functionFactory ();
}

#endif /* __OAF_H */
