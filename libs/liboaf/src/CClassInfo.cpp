/**
 * @file
 * @brief Реализация описания OAF-класса
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#include <idl/IGenericFactory.h>

#include <OAF/OAF.h>

#include "CClassInfo.h"

//
// Псевдо-CID для CClassInfo
//
static QString class_info_cid = "OAF/CClassInfo:1.0";

OAF::CClassInfo::ClassInfoMap OAF::CClassInfo::m_class_info_map;

OAF::CClassInfo::CClassInfo (Type _type, const QString& _location) :
	CUnknown (class_info_cid), m_type (_type), m_location (_location)
{
	if (m_type == DLL)
	{
		m_library.setLoadHints (QLibrary::ExportExternalSymbolsHint);
		m_library.setFileName (m_location);
	}
}

OAF::CClassInfo::~CClassInfo ()
{}

OAF::URef<OAF::IUnknown>
OAF::CClassInfo::createObject (const QString& _cid) const
{
	//
	// @todo циклические ссылки пока не обнаруживаются
	//
	switch (m_type)
	{
		case DLL:
			{
				typedef OAF::IUnknown* (*CreateObject) (const QString&);

				if (CreateObject create_object = (CreateObject)m_library.resolve ("createObject"))
					return create_object (_cid);

				qWarning ("Can't resolve 'createObject' symbol with reason: %s", qPrintable (m_library.errorString ()));
			};
			break;

		case FACTORY:
			{
				if (const CClassInfo* c = getClassInfo (m_location))
				{
					if (OAF::URef<OAF::IUnknown> o = c->createObject (m_location))
					{
						if (OAF::URef<OAF::IGenericFactory> f = o.queryInterface<OAF::IGenericFactory> ())
							return f->createObject (_cid);
						else
							qWarning ("Can't resolve OAF::IGenericFactory interface for class %s", qPrintable (OAF::interfaceName (typeid (*o))));
					}
					else
						qWarning ("Can't create object from cid %s", qPrintable (m_location));
				}
				else
					qWarning ("Can't find class info for cid %s", qPrintable (m_location));
			};
			break;

		default:
			break;
	}

	return OAF::URef<OAF::IUnknown> ();
}

void
OAF::CClassInfo::unloadUnusedLibrary () const
{
	if ((m_type == DLL) && m_library.isLoaded ())
	{
		typedef bool (*DllCanUnloadNow) ();

		if (DllCanUnloadNow dll_can_unload_now = (DllCanUnloadNow)m_library.resolve ("dllCanUnloadNow"))
		{
			if (dll_can_unload_now ())
				m_library.unload ();
		}
	}
}

const OAF::CClassInfo::ClassInfoMap&
OAF::CClassInfo::enumClassInfo ()
{
	//
	// Если описания OAF-классов ещё не загружены
	//
	if (m_class_info_map.size () == 0)
	{
		//
		// Пространство имён, в котором определены тэги описания OAF-класса
		//
		static QString oaf = "http://www.spectrumsoft.msk.ru/files/specs/oaf-syntax.dtd";

		//
		// Получаем список директорий, в которых хранятся описания OAF-классов
		//
		QStringList paths = QProcessEnvironment::systemEnvironment ().value ("OAFPATH").split (';');

		//
		// Для UNIX добавляем к списку директорию по умолчанию
		//
#ifndef WIN32
		paths.append ("/usr/lib/oaf-qt4/plugins");
#endif

		//
		// Проходим по всем директориям
		//
		for (QStringList::const_iterator d = paths.begin (); d != paths.end (); ++d)
		{
			//
			// Загружаем список файлов с описаниями OAF-классов из директории
			//
			QFileInfoList infos = QDir (*d, "*.oaf").entryInfoList (QDir::Files);

			//
			// Загружаем все выбранные файлы
			//
			for (QFileInfoList::const_iterator i = infos.begin (); i != infos.end (); ++i)
			{
				QFile info (i->absoluteFilePath ());

				if (info.open (QIODevice::ReadOnly))
				{
					//
					// Потоковый XML-парсер
					//
					QXmlStreamReader xml (&info);

					//
					// Если первый элемент это не oaf:info, то прерываем чтение этого файла
					//
					if (!xml.readNextStartElement () || (xml.namespaceUri () != oaf) || (xml.name () != "info"))
						continue;

					//
					// Пока следующим элементом является "oaf:class"
					//
					while (xml.readNextStartElement () && (xml.namespaceUri () == oaf) && (xml.name () == "class"))
					{
						//
						// Ссылка на создаваемое описание OAF-класса
						//
						OAF::URef<CClassInfo> c;

						//
						// Проверяем обязательные атрибуты OAF-класса
						//
						QXmlStreamAttributes cattrs = xml.attributes ();
						if (cattrs.hasAttribute ("cid") &&
							cattrs.hasAttribute ("type") &&
							cattrs.hasAttribute ("location"))
						{
							//
							// Проверяем способ создания объектов данного OAF-класса и для
							// распознанных способов создаём заготовку описания OAF-класса
							//
							if (cattrs.value ("type") == "dll")
							{
								//
								// Конструируем путь к библиотеке. Если путь к библиотеке задан
								// относительно *.info файла, то конструируем путь к библиотеке
								// из пути к *.info файлу и location
								//
								QFileInfo dll (cattrs.value ("location").toString ());
								if (dll.isRelative ())
									dll.setFile (QFileInfo (info.fileName ()).dir (), dll.filePath ());

								c = new CClassInfo (DLL, dll.absoluteFilePath ());
							}
							else if (cattrs.value ("type") == "factory")
							{
								c = new CClassInfo (FACTORY, cattrs.value ("location").toString ());
							}
						}

						//
						// Пока следующим элементом является oaf:attribute
						//
						while (xml.readNextStartElement () && (xml.namespaceUri () == oaf) && (xml.name () == "attribute"))
						{
							//
							// Проверяем обязательные атрибуты oaf:attribute
							//
							QXmlStreamAttributes aattrs = xml.attributes ();
							if (aattrs.hasAttribute ("name") && aattrs.hasAttribute ("type"))
							{
								//
								// Если это список строк
								//
								if (aattrs.value ("type") == "stringv")
								{
									QStringList stringv;

									//
									// Пока следующим элементом является элемент списка строк
									//
									while (xml.readNextStartElement () && (xml.namespaceUri () == oaf) && (xml.name () == "item"))
									{
										QXmlStreamAttributes iattrs = xml.attributes ();
										if (iattrs.hasAttribute ("value"))
											stringv.append (iattrs.value ("value").toString ());

										//
										// Если ещё не находимся в конце элемента, то пропускаем всё
										// до конца oaf:item
										//
										if (!xml.isEndElement ())
											xml.skipCurrentElement ();
									}

									//
									// Устанавливаем значение соответствующего атрибута
									//
									if (c)
									{
										c->setValue (aattrs.value ("name").toString (), QVariant (stringv));
									}
								}

								//
								// Если это строка
								//
								else if (aattrs.value ("type") == "string")
								{
									//
									// Устанавливаем значение соответствующего атрибута
									//
									if (c && aattrs.hasAttribute ("value"))
									{
										c->setValue (aattrs.value ("name").toString (),
													 aattrs.value ("value").toString ());
									}
								}

								//
								// Если это булевское значение
								//
								else if (aattrs.value ("type") == "boolean")
								{
									//
									// Устанавливаем значение соответствующего атрибута
									//
									if (c && aattrs.hasAttribute ("value"))
									{
										if (aattrs.value ("value").compare ("true", Qt::CaseInsensitive))
											c->setValue (aattrs.value ("name").toString (), true);
										else if (aattrs.value ("value").compare ("yes", Qt::CaseInsensitive))
											c->setValue (aattrs.value ("name").toString (), true);
										else if (aattrs.value ("value").compare ("false", Qt::CaseInsensitive))
											c->setValue (aattrs.value ("name").toString (), false);
										else if (aattrs.value ("value").compare ("no", Qt::CaseInsensitive))
											c->setValue (aattrs.value ("name").toString (), false);
									}
								}

								//
								// Если это целое число
								//
								else if (aattrs.value ("type") == "integer")
								{
									//
									// Устанавливаем значение соответствующего атрибута
									//
									if (c && aattrs.hasAttribute ("value"))
									{
										c->setValue (aattrs.value ("name").toString (),
													 aattrs.value ("value").toString ().toLongLong ());
									}
								}

								//
								// Если это число
								//
								else if (aattrs.value ("type") == "double")
								{
									//
									// Устанавливаем значение соответствующего атрибута
									//
									if (c && aattrs.hasAttribute ("value"))
									{
										c->setValue (aattrs.value ("name").toString (),
													 aattrs.value ("value").toString ().toDouble ());
									}
								}

								//
								// Если это неизвестный тип, то пропускаем его
								//
							}

							//
							// Если мы ещё не находимся в конце елемента, то пропускаем всё
							// до конца oaf:attribute
							//
							if (!xml.isEndElement ())
								xml.skipCurrentElement ();
						}

						//
						// Запоминаем описание класса в списке
						//
						if (c)
						{
							m_class_info_map.insert (cattrs.value ("cid").toString (), c);
						}

						//
						// Если ещё не находимся в конце элемента, то пропускаем всё
						// до конца oaf:class
						//
						if (!xml.isEndElement ())
							xml.skipCurrentElement ();
					}
				}
			}
		}
	}

	return m_class_info_map;
}

const OAF::CClassInfo*
OAF::CClassInfo::getClassInfo (const QString& _cid)
{
	const ClassInfoMap& class_info_map = enumClassInfo ();

	ClassInfoMap::const_iterator i = class_info_map.find (_cid);

	return (i == class_info_map.end ()) ? NULL : i.value ().ptr ();
}

void
OAF::CClassInfo::unloadClassInfo ()
{
	m_class_info_map.clear ();
}
