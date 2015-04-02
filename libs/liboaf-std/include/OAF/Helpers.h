/**
 * @file
 * @brief Вспомогательные функции для создания объектов
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#ifndef __HELPERS_H
#define __HELPERS_H

#include <QtCore>

#include <idl/IIODevice.h>
#include <idl/ITXTPersist.h>
#include <idl/IXMLPersist.h>

#include <OAF/OAF.h>
#include <OAF/ExchangeHelpers.h>
#include <OAF/OafStdGlobal.h>

namespace OAF
{
	/**
	 * @brief Получить список MIME-типов для объекта
	 */
	OAFSTD_EXPORT QStringList mimeForObject (OAF::IUnknown* _o);

	/**
	 * @brief Представление списка MIME-типов как строки
	 */
	OAFSTD_EXPORT QString mimeAsString (const QStringList& _list);

	/**
	 * @brief Представление строки как списка MIME-типов
	 */
	OAFSTD_EXPORT QStringList mimeAsStringList (const QString& _s);

	/**
	 * @brief Функция создания объекта по его интерфейсу и списку MIME-типов
	 */
	template<class _Interface>
	OAF::URef<_Interface> createFromMIME (const QStringList& _mime_types)
	{
		//
		// Запрос на поиск OAF-класса объектов обрабатывающих данные заданного типа и поддерживающих
		// заданный интерфейс
		//
		static QString query = "repo_ids.has('%1') AND mime_types.has('%2') ORDER BY ifnull(priority, 0) LIMIT 1";

		//
		// Проходим по всем заданным MIME-типам в поисках обработчика в порядке увеличения общности
		//
		for (QStringList::const_iterator m = _mime_types.begin (); m != _mime_types.end (); ++m)
		{
			//
			// Запрашиваем OAF-класс, объект которого может обработать данные такого типа
			// и поддерживает заданный интерфейс
			//
			QStringList cids;
			if (OAF::query (cids, query.arg (OAF::interfaceName<_Interface> ()).arg (*m)) > 0)
			{
				//
				// Создаём объект найденного OAF-класса
				//
				if (OAF::URef<OAF::IUnknown> o = OAF::createFromCID (cids[0]))
				{
					//
					// И возвращаем ссылку на его затребованный интерфейс
					//
					if (OAF::URef<_Interface> i = o.queryInterface<_Interface> ())
						return i;
					else
						qWarning ("Can't resolve %s interface for class %s", qPrintable (OAF::interfaceName<_Interface> ()), qPrintable (OAF::interfaceName (typeid (*o))));
				}
				else
					qWarning ("Can't create object from cid %s", qPrintable (cids[0]));
			}
		}

		//
		// Ничего не нашли
		//
		return OAF::URef<_Interface> ();
	};

	/**
	 * @brief Функция создания объекта обёртки с заданным интерфейсом для заданного объекта
	 *
	 * Классы должны быть связаны жёстко через CID объекта, который задан в качестве параметра.
	 * Это связано с тем, что объект обёртка должен быть точно уверен в том, что интерфейс объекта
	 * параметра соответствует его ожиданиям.
	 */
	template<class _Interface>
	OAF::URef<_Interface> createFromObject (OAF::IUnknown* _o)
	{
		//
		// Запрос на поиск OAF-класса объектов расширяющих объекты заданного класса и
		// поддерживающих заданный интерфейс
		//
		static QString query = "repo_ids.has_all(['OAF::IUnknownExtender','%1']) AND extends.has('%2') ORDER BY ifnull(priority, 0) LIMIT 1";

		//
		// Запрашиваем OAF-класс, объект которого может обработать данные такого типа
		// и поддерживает заданный интерфейс
		//
		QStringList cids;
		if (OAF::query (cids, query.arg (OAF::interfaceName<_Interface> ()).arg (_o->cid ())) > 0)
		{
			//
			// Создаём объект найденного OAF-класса
			//
			if (OAF::URef<OAF::IUnknown> o = OAF::createFromCID (cids[0]))
			{
				//
				// Получаем OAF::IUnknownExtender-интерфейс объекта и получаем на него ссылку
				//
				if (OAF::URef<OAF::IUnknownExtender> p = o.queryInterface<OAF::IUnknownExtender> ())
				{
					//
					// Устанавливаем для объекта заданное устройство ввода/вывода
					//
					if (OAF::URef<OAF::IUnknown> po = p->setExtendedObject (_o))
					{
						//
						// И возвращаем ссылку на его затребованный интерфейс
						//
						if (OAF::URef<_Interface> i = po.queryInterface<_Interface> ())
							return i;
						else
							qWarning ("Can't resolve %s interface for class %s", qPrintable (OAF::interfaceName<_Interface> ()), qPrintable (OAF::interfaceName (typeid (*po))));
					}
					else
						qWarning ("Can't set object for class %s", qPrintable (OAF::interfaceName (typeid (*p))));
				}
				else
					qWarning ("Can't resolve OAF::IUnknownExtender interface for class %s", qPrintable (OAF::interfaceName (typeid (*o))));
			}
			else
				qWarning ("Can't create object from cid %s", qPrintable (cids[0]));
		}
		else
			qWarning ("Can't find objects for query %s", qPrintable (query.arg (OAF::interfaceName<_Interface> ()).arg (_o->cid ())));

		//
		// Ничего не нашли
		//
		return OAF::URef<_Interface> ();
	};

	/**
	 * @brief Функция создания объекта из интерфейса устройства ввода/вывода
	 */
	template<class _Interface>
	OAF::URef<_Interface> createFromIO (OAF::IIODevice* _io)
	{
		//
		// Запрос на поиск OAF-класса объектов обрабатывающих данные заданного типа, загружающих
		// их из текстового потока и поддерживающих заданный интерфейс
		//
		static QString query = "repo_ids.has_all(['OAF::IIODeviceExtender','%1']) AND mime_types.has('%2') ORDER BY ifnull(priority, 0) LIMIT 1";

		//
		// Запрашиваем у устройства список его MIME-типов
		//
		QStringList mime_types = _io->getInfo (OAF::IIODevice::CONTENT_TYPE).toStringList ();

		//
		// Проходим по всем заданным MIME-типам в поисках обработчика в порядке увеличения общности
		//
		for (QStringList::const_iterator m = mime_types.begin (); m != mime_types.end (); ++m)
		{
			//
			// Запрашиваем OAF-класс, объект которого может обработать данные такого типа
			// и поддерживает заданный интерфейс
			//
			QStringList cids;
			if (OAF::query (cids, query.arg (OAF::interfaceName<_Interface> ()).arg (*m)) > 0)
			{
				//
				// Создаём объект найденного OAF-класса
				//
				if (OAF::URef<OAF::IUnknown> o = OAF::createFromCID (cids[0]))
				{
					//
					// Получаем OAF::IIODeviceExtender-интерфейс объекта и получаем на него ссылку
					//
					if (OAF::URef<OAF::IIODeviceExtender> p = o.queryInterface<OAF::IIODeviceExtender> ())
					{
						//
						// Устанавливаем для объекта заданное устройство ввода/вывода
						//
						if (OAF::URef<OAF::IUnknown> po = p->setIODevice (_io))
						{
							//
							// И возвращаем ссылку на его затребованный интерфейс
							//
							if (OAF::URef<_Interface> i = po.queryInterface<_Interface> ())
								return i;
							else
								qWarning ("Can't resolve %s interface for class %s", qPrintable (OAF::interfaceName<_Interface> ()), qPrintable (OAF::interfaceName (typeid (*po))));
						}
						else
							qWarning ("Can't set IO device for class %s", qPrintable (OAF::interfaceName (typeid (*p))));
					}
					else
						qWarning ("Can't resolve OAF::IIODeviceExtender interface for class %s", qPrintable (OAF::interfaceName (typeid (*o))));
				}
				else
					qWarning ("Can't create object from cid %s", qPrintable (cids[0]));
			}
		}

		//
		// Ничего не нашли
		//
		return OAF::URef<_Interface> ();
	};

	/**
	 * @brief Функция создания объекта из текстового потока по его интерфейсу и списку MIME-типов
	 */
	template<class _Interface>
	OAF::URef<_Interface> createFromTXT (QTextStream& _is, const QStringList& _mime_types)
	{
		//
		// Запрос на поиск OAF-класса объектов обрабатывающих данные заданного типа, загружающих
		// их из текстового потока и поддерживающих заданный интерфейс
		//
		static QString query = "repo_ids.has_all(['OAF::ITXTPersist','%1']) AND mime_types.has('%2') ORDER BY ifnull(priority, 0) LIMIT 1";

		//
		// Проходим по всем заданным MIME-типам в поисках обработчика в порядке увеличения общности
		//
		for (QStringList::const_iterator m = _mime_types.begin (); m != _mime_types.end (); ++m)
		{
			//
			// Запрашиваем OAF-класс, объект которого может обработать данные такого типа
			// и поддерживает заданный интерфейс
			//
			QStringList cids;
			if (OAF::query (cids, query.arg (OAF::interfaceName<_Interface> ()).arg (*m)) > 0)
			{
				//
				// Создаём объект найденного OAF-класса
				//
				if (OAF::URef<OAF::IUnknown> o = OAF::createFromCID (cids[0]))
				{
					//
					// Получаем OAF::ITXTPersist-интерфейс объекта
					//
					if (OAF::URef<OAF::ITXTPersist> p = o.queryInterface<OAF::ITXTPersist> ())
					{
						//
						// Загружаем данные из потока в объект
						//
						p->load (_is, _mime_types);

						//
						// И возвращаем ссылку на его затребованный интерфейс
						//
						if (OAF::URef<_Interface> i = p.queryInterface<_Interface> ())
							return i;
						else
							qWarning ("Can't resolve %s interface for class %s", qPrintable (OAF::interfaceName<_Interface> ()), qPrintable (OAF::interfaceName (typeid (*p))));
					}
					else
						qWarning ("Can't resolve OAF::ITXTPersist interface for class %s", qPrintable (OAF::interfaceName (typeid (*o))));
				}
				else
					qWarning ("Can't create object from cid %s", qPrintable (cids[0]));
			}
		}

		//
		// Ничего не нашли
		//
		return OAF::URef<_Interface> ();
	};

	/**
	 * @brief Функция создания объекта из XML-потока по его интерфейсу и списку MIME-типов
	 */
	template<class _Interface>
	OAF::URef<_Interface> createFromXML (QXmlStreamReader& _is, const QStringList& _mime_types)
	{
		//
		// Запрос на поиск OAF-класса объектов обрабатывающих данные заданного типа, загружающих
		// их из XML-потока и поддерживающих заданный интерфейс
		//
		static QString query = "repo_ids.has_all(['OAF::IXMLPersist','%1']) AND mime_types.has('%2') ORDER BY ifnull(priority, 0) LIMIT 1";

		//
		// Проходим по всем заданным MIME-типам в поисках обработчика в порядке увеличения общности
		//
		for (QStringList::const_iterator m = _mime_types.begin (); m != _mime_types.end (); ++m)
		{
			//
			// Запрашиваем OAF-класс, объект которого может обработать данные такого типа
			// и поддерживает заданный интерфейс
			//
			QStringList cids;
			if (OAF::query (cids, query.arg (OAF::interfaceName<_Interface> ()).arg (*m)) > 0)
			{
				//
				// Создаём объект найденного OAF-класса
				//
				if (OAF::URef<OAF::IUnknown> o = OAF::createFromCID (cids[0]))
				{
					//
					// Получаем OAF::IXMLPersist-интерфейс объекта
					//
					if (OAF::URef<OAF::IXMLPersist> p = o.queryInterface<OAF::IXMLPersist> ())
					{
						//
						// Загружаем данные из потока в объект
						//
						p->load (_is, _mime_types);

						//
						// И возвращаем ссылку на его затребованный интерфейс
						//
						if (OAF::URef<_Interface> i = p.queryInterface<_Interface> ())
							return i;
						else
							qWarning ("Can't resolve %s interface for class %s", qPrintable (OAF::interfaceName<_Interface> ()), qPrintable (OAF::interfaceName (typeid (*p))));
					}
					else
						qWarning ("Can't resolve OAF::IXMLPersist interface for class %s", qPrintable (OAF::interfaceName (typeid (*o))));
				}
				else
					qWarning ("Can't create object from cid %s", qPrintable (cids[0]));
			}
		}

		//
		// Ничего не нашли
		//
		return OAF::URef<_Interface> ();
	};
}

#endif /* __HELPERS_H */
