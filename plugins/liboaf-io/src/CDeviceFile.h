/**
 * @file
 * @brief Интерфейс устройства файлового ввода/вывода
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#ifndef __OAF_IO_CDEVICEFILE_H
#define __OAF_IO_CDEVICEFILE_H

#include <QtCore>

#include <idl/IUnknown.h>
#include <idl/IIODevice.h>

#include <OAF/CUnknown.h>

#include "CCommon.h"
#include "CFactory.h"

namespace OAF
{
	namespace IO
	{
		/**
		 * @brief Устройство файлового ввода/вывода
		 */
		class CDeviceFile : public QObject,
			//
			// Импортируемые реализации
			//
			public CUnknown,
			public CCommon,
			//
			// Экспортируемые интерфейсы
			//
			virtual public OAF::IUnknown,
			virtual public OAF::IIODevice
		{
			Q_OBJECT

			/**
			 * @brief Фабрика устройств
			 */
			OAF::URef<CFactory> m_factory;

			/**
			 * @brief Нужна ли копия данных
			 *
			 * Копия данных реализуется через добавление специального URL параметра oaf_copy_id={uuid}
			 * для каждой ссылки. При существующем механизме открытия данных это гарантирует,
			 * что будет открыта копия данных, а не получена ссылка на уже загруженные данные.
			 */
			QUuid m_copy_id;

			/**
			 * @brief Файл для работы
			 */
			QFile* m_file;

		public:
			CDeviceFile (CFactory* _factory);
			~CDeviceFile ();

			/**
			 * @name Реализация интерфейса OAF::IIODevice
			 */
			/** @{*/
			QVariant getInfo (DeviceInfo _what);
			void setInfo (DeviceInfo _what, const QVariant& _v);
			QIODevice* device ();
			bool exists () const;
			/** @}*/
		};
	}
}

#endif /* __OAF_IO_CDEVICEFILE_H */
