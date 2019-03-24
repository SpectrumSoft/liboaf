/**
 * @file
 * @brief Интерфейс устройства ввода/вывода для объектов git-репозитория
 * @author Alexander Kamyshnikov <axill777@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#ifndef __OAF_IO_CDEVICEGIT_H
#define __OAF_IO_CDEVICEGIT_H

#include <QtCore>

#include <idl/IUnknown.h>
#include <idl/IIODevice.h>

#include <OAF/CUnknown.h>
#include <OAF/CGitUrl.h>

#include "CCommon.h"
#include "CFactory.h"

struct git_repository;

namespace OAF
{
	namespace IO
	{
		/**
		 * @brief Устройство ввода/вывода из системы контроля версий GIT
		 */
		class CDeviceGIT : public QObject,
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
			 * @brief Путь к файлу в git-репозитории в виде URL
			 */
			CGitUrl m_git_url;

			/**
			 * @brief Бинарные/текстовые данные объекта из git-репозитория
			 */
			QByteArray m_data;

			/**
			 * @brief Буфер для ввода/вывода, где будем хранить данные git-объекта
			 */
			QBuffer* m_buffer;

		public:
			CDeviceGIT (CFactory* _factory);
			~CDeviceGIT ();

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

#endif /* __OAF_IO_CDEVICEGIT_H */
