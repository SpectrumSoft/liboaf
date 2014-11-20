/**
 * @file
 * @brief Интерфейс описания OAF-класса
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#ifndef __OAF_CCLASS_INFO_H
#define __OAF_CCLASS_INFO_H

#include <OAF/OafGlobal.h>

#include <QString>
#include <QMap>
#include <QLibrary>

#include <idl/IInterface.h>
#include <idl/IUnknown.h>
#include <idl/INotifySource.h>
#include <idl/IPropertyBag.h>

#include <OAF/CUnknown.h>
#include <OAF/CPropertyBag.h>

namespace OAF
{
	/**
	 * @brief Описание OAF-класса
	 */
	class OAF_EXPORT CClassInfo :
		//
		// Экспортируемые интерфейсы
		//
		virtual public OAF::IInterface,
		virtual public OAF::IUnknown,
		virtual public OAF::IPropertyBag,
		//
		// Импортируемые реализации
		//
		virtual public CUnknown,
		virtual public CPropertyBag
	{
	public:
		/**
		 * @brief Способы создания объектов OAF-класса
		 */
		enum Type
		{
			DLL,    //!< С помощью динамически загружаемой библиотеки
			FACTORY //!< С помощью объекта-фабрики
		};

		/**
		 * @brief Список описаний OAF-классов
		 */
		typedef QMap<QString, OAF::URef<CClassInfo> > ClassInfoMap;

	private:
		/**
		 * @brief Способ создания объектов данного OAF-класса
		 */
		Type m_type;

		/**
		 * @brief Локатор фабрики объектов данного OAF-класса
		 */
		QString m_location;

		/**
		 * @brief Библиотека для создания объектов данного OAF-класса
		 *
		 * Используется только если @a m_type == DLL
		 */
		mutable QLibrary m_library;

		/**
		 * Загруженные описания классов
		 */
		static ClassInfoMap m_class_info_map;

	public:
		CClassInfo (Type _type, const QString& _location);
		~CClassInfo ();

		/**
		 * @brief Создать объект заданного класса
		 *
		 * @return Ссылка на созданный объект
		 */
		OAF::URef<OAF::IUnknown> createObject (const QString& _cid) const;

		/**
		 * @brief Выгрузить библиотеку если она не используется
		 */
		void unloadUnusedLibrary () const;

		/**
		 * @brief Список загруженных описаний OAF-классов
		 *
		 * Выполняет автоматическую загрузку описаний классов если они ещё не загружены.
		 */
		static const ClassInfoMap& enumClassInfo ();

		/**
		 * @brief Описание заданного OAF-класса
		 *
		 * Выполняет автоматическую загрузку описаний классов если они ещё не загружены.
		 */
		static const CClassInfo* getClassInfo (const QString& _cid);

		/**
		 * @brief Выгрузить из памяти все описания классов
		 */
		static void unloadClassInfo ();
	};
}

#endif /* __OAF_CCLASS_INFO_H */
