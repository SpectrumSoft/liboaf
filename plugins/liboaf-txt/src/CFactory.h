/**
 * @file
 * @brief Интерфейс фабрики объектов компонента
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#ifndef __OAF_TXT_CFACTORY_H
#define __OAF_TXT_CFACTORY_H

#include <QtCore>

#include <idl/IUnknown.h>
#include <idl/IGenericFactory.h>

#include <OAF/CUnknown.h>

#include "CCommon.h"

namespace OAF
{
	namespace TXT
	{
		/**
		 * @brief Фабрика объектов компонента
		 */
		class CFactory :
			//
			// Импортируемые реализации
			//
			public CUnknown,
			public CCommon,
			//
			// Экспортируемые интерфейсы
			//
			virtual public OAF::IUnknown,
			virtual public OAF::IGenericFactory
		{
			/**
			 * @brief Глобальный объект фабрики
			 */
			static CFactory* m_instance;

			/**
			 * @brief Список уже открытых файлов
			 */
			QMap<QString, OAF::IUnknown*> m_opened_files;

			CFactory ();

		public:
			~CFactory ();

			/**
			 * @name Реализация интерфейса OAF::IGenericFactory
			 */
			/** @{*/
			OAF::URef<OAF::IUnknown> createObject (const QString& _cid);
			/** @}*/

			/**
			 * @brief Зарегистрировать файл с заданным именем
			 */
			void add (const QString& _name, OAF::IUnknown* _file);

			/**
			 * @brief Убрать файл с заданным именем
			 */
			void remove (const QString& _name);

			/**
			 * @brief Найти файл по его имени
			 */
			OAF::IUnknown* get (const QString& _name);

			/**
			 * @brief Создать объект фабрики
			 */
			static OAF::IUnknown* instance ();
		};
	}
}

#endif /* __OAF_TXT_CFACTORY_H */
