/**
 * @file
 * @brief Интерфейс моникера устройств ввода/вывода
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#ifndef __OAF_IO_CMONIKERIO_H
#define __OAF_IO_CMONIKERIO_H

#include <idl/IUnknown.h>
#include <idl/IMoniker.h>

#include <OAF/CUnknown.h>
#include <OAF/CMoniker.h>

#include "CCommon.h"
#include "CFactory.h"

namespace OAF
{
	namespace IO
	{
		/**
		 * @brief Моникер устройств ввода/вывода
		 */
		class CMonikerIO :
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
			CMonikerIO (CFactory* _factory);
			~CMonikerIO ();

			/**
			 * @name Реализация интерфейса OAF::IMoniker
			 */
			/** @{*/
			OAF::URef<OAF::IUnknown> resolve (const QString& _interface);
			/** @}*/
		};
	}
}

#endif /* __OAF_IO_CMONIKERIO_H */
