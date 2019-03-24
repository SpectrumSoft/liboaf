/**
 * @file
 * @brief Интерфейс базовой реализации моникера
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#ifndef __CMONIKER_H
#define __CMONIKER_H

#include <QtCore>

#include <idl/IInterface.h>
#include <idl/IUnknown.h>
#include <idl/IMoniker.h>

#include <OAF/OafCoreGlobal.h>

namespace OAF
{
	/**
	 * @brief Базовая реализация моникера
	 */
	class OAFCORE_EXPORT CMoniker :
		//
		// Экспортируемые интерфейсы
		//
		virtual public OAF::IMoniker
	{
		/**
		 * @brief Ссылка на "левый" моникер
		 */
		OAF::URef<OAF::IMoniker> m_left;

		/**
		 * @brief Префикс
		 */
		QString m_prefix;

		/**
		 * @brief Суффикс
		 */
		QString m_suffix;

	public:
		CMoniker ();

		/**
		 * @brief Конструирование моникера на основе другого моникера
		 *
		 * Явная инициализация копий всех интерфейсов нужна чтобы не выдавались
		 * предупреждения о неявной инициализации виртуальных базовых классов
		 */
		CMoniker (const CMoniker& _moniker);

		~CMoniker ();

		/**
		 * @name Реализация методов интерфейса OAF::IMoniker
		 *
		 * Не реализуется только метод OAF::IMoniker::resolve, который специфичен для каждого
		 * вида моникера
		 */
		/** @{*/
		void set (OAF::IMoniker* _left, const QString& _prefix, const QString& _suffix);
		OAF::IMoniker* left ();
		const QString& prefix ();
		const QString& suffix ();
		/** @}*/
	};
}

#endif /* __CMONIKER_H */
