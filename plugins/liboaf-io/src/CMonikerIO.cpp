/**
 * @file
 * @brief Реализация моникера устройств ввода/вывода
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#include <idl/IIODevice.h>

#include <OAF/OAF.h>

#include "CMonikerIO.h"

using namespace OAF::IO;

CMonikerIO::CMonikerIO (CFactory* _factory) : CUnknown (moniker_cid), m_factory (_factory)
{}

CMonikerIO::~CMonikerIO ()
{}

OAF::URef<OAF::IUnknown>
CMonikerIO::resolve (const QString& _interface)
{
	//
	// Запрос на поиск OAF-класса объектов, расширяющих интерфейс устройства ввода/вывода
	// заданного типа
	//
	static QString query = "repo_ids.has_all(['OAF::IIODeviceExtender','%1']) AND mime_types.has('%2') ORDER BY ifnull(priority, 0) LIMIT 1";

	//
	// Полные данные по объекту
	//
	QString path = prefix () + suffix ();

	//
	// Идентификатор класса устройства
	//
	QString cid;

	//
	// Моникер file:
	//
	if (prefix () == "file:")
		cid = device_file_cid;
	//
	// Моникер raw:
	//
	else if (prefix () == "raw:")
		cid = device_buffer_cid;
	//
	// Моникер git:
	//
	else if (prefix () == "git:")
		cid = device_git_cid;

	//
	// Если удалось определить класс устройства
	//
	if (!cid.isEmpty ())
	{
		//
		// Создаём базовый объект устройства
		//
		if (OAF::URef<OAF::IUnknown> o = m_factory->createObject (cid))
		{
			//
			// Преобразуем его к интерфейсу OAF::IIODevice
			//
			if (OAF::URef<OAF::IIODevice> d = o.queryInterface<OAF::IIODevice> ())
			{
				//
				// Устанавливаем для него данные
				//
				d->setInfo (OAF::IIODevice::PATH, path);

				//
				// Если запрашивается базовый интерфейс, то возвращаем ссылку на
				// базовый объект
				//
				if (_interface == OAF::interfaceName<OAF::IIODevice> ())
					return o;

				//
				// Запрашиваем у базового объекта устройства список его MIME-типов
				//
				QStringList mime_types = d->getInfo (OAF::IIODevice::CONTENT_TYPE).value<QStringList> ();

				//
				// Проходим по всем MIME-типам в поисках OAF-класса объекта-адаптера с нужным интерфейсом
				//
				for (QStringList::const_iterator m = mime_types.begin (); m != mime_types.end (); ++m)
				{
					//
					// Запрашиваем список OAF-классов, объекты которых могут обработать устройства
					// такого типа и поддерживают заданный интерфейс
					//
					QStringList cids;
					if (OAF::query (cids, query.arg (_interface).arg (*m)) > 0)
					{
						//
						// Создаём объект найденного OAF-класса
						//
						if (OAF::URef<OAF::IUnknown> oe = OAF::createFromCID (cids[0]))
						{
							//
							// Преобразуем объект к интерфейсу OAF::IIODeviceExtender и возвращаем ссылку
							// на объект, созданный из данных заданного устройства
							//
							if (OAF::URef<OAF::IIODeviceExtender> de = oe.queryInterface<OAF::IIODeviceExtender> ())
								return de->setIODevice (d);
						}
					}
				}
			}
		}
	}

	//
	// Увы, но подходящий объект создать не удалось
	//
	return OAF::URef<OAF::IUnknown> ();
}
