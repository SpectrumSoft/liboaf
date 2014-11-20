/**
 * @file
 * @brief Интерфейс стандартного моникера
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#ifndef __OAF_STD_CMONIKERSTD_H
#define __OAF_STD_CMONIKERSTD_H

#include <idl/IUnknown.h>
#include <idl/IMoniker.h>

#include <OAF/CUnknown.h>
#include <OAF/CMoniker.h>

#include "CCommon.h"
#include "CFactory.h"

namespace OAF
{
	namespace STD
	{
		/**
		 * @brief Моникер устройств ввода/вывода
		 */
		class CMonikerSTD :
			//
			// Экспортируемые интерфейсы
			//
			virtual public OAF::IUnknown,
			virtual public OAF::IMoniker,
			//
			// Импортируемые реализации
			//
			virtual public CUnknown,
			virtual public CMoniker,
			virtual public CCommon
		{
			/**
			 * @brief Фабрика устройств
			 */
			OAF::URef<CFactory> m_factory;

		public:
			CMonikerSTD (CFactory* _factory);
			~CMonikerSTD ();

			/**
			 * @name Реализация интерфейса OAF::IMoniker
			 */
			/** @{*/
			OAF::URef<OAF::IUnknown> resolve (const QString& _interface);
			/** @}*/
		};
	}
}

#endif /* __OAF_STD_CMONIKERSTD_H */
