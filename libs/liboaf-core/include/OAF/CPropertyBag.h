/**
 * @file
 * @brief Интерфейс стандартных реализаций контейнеров атрибутов
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#ifndef __CPROPERTY_BAG_H
#define __CPROPERTY_BAG_H

#include <QtCore>

#include <idl/IUnknown.h>
#include <idl/IPropertyBag.h>

#include <OAF/CUnknown.h>
#include <OAF/CNotifySource.h>

#include <OAF/OafCoreGlobal.h>

namespace OAF
{
	/**
	 * @brief Базовая реализация контейнера атрибутов
	 */
	class OAFCORE_EXPORT CPropertyBag :
		//
		// Экспортируемые интерфейсы
		//
		virtual public OAF::IPropertyBag
	{
		/**
		 * @brief Список атрибутов
		 */
		PropertySet m_values;

	protected:
		/**
		 * @brief Признак определённости виртуального значения
		 */
		virtual bool isVirtualDefined (const Key& _key) const;

		/**
		 * @brief Получить "виртуальное" значение
		 */
		virtual Value getVirtualValue (const Key& _key) const;

		/**
		 * @brief Установить "виртуальное" значение
		 */
		virtual void setVirtualValue (const Key& _key, const Value& _value);

	public:
		CPropertyBag ();

		/**
		 * @brief Конструирование контенейра на основании предопределённого списка атрибутов
		 *
		 * Этот конструктор задаёт начальный список атрибутов (в том числе и атрибутов
		 * "только для чтения")
		 */
		CPropertyBag (const PropertySet& _values);

		/**
		 * @brief Конструирование контейнера на основе другого контейнера
		 *
		 * Явная инициализация копий всех интерфейсов нужна чтобы не выдавались
		 * предупреждения о неявной инициализации виртуальных базовых классов
		 */
		CPropertyBag (const CPropertyBag& _bag);

		/**
		 * @name Реализация методов интерфейса OAF::IPropertyBag
		 */
		/** @{*/
		std::size_t enumKeys (KeyList& _out) const;

		/*
		 * Флаги, ассоциированные с атрибутами
		 *
		 * По умолчанию все атрибуты являются:
		 *     -# доступными для чтения;
		 *     -# доступными для записи;
		 *     -# без рассылки сообщения об изменении;
		 *     -# без постоянного хранения;
		 *     -# без индексации;
		 *     -# не экспортируемыми;
		 *     -# не виртуальными.
		 */
		PropertyFlags getFlags (const Key& /*_key*/) const;

		bool isDefined (const Key& _key) const;
		Value getValue (const Key& _key) const;
		std::size_t getValues (PropertySet& _out, PropertyFlags _f = 0) const;
		void setValue (const Key& _key, const Value& _value, OAF::INotifyListener* _origin = NULL);
		void setValues (const PropertySet& _values, OAF::INotifyListener* _origin = NULL);
		/** @}*/
	};

	/**
	 * @brief Срез ключей, начинающихся с заданного префикса
	 *
	 * @todo Реализовать возможность подключения уведомлений через Slice, если
	 *       связанный с ним OAF::IPropertyBag это поддерживает
	 */
	class OAFCORE_EXPORT CSlicePropertyBag :
		//
		// Импортируемые реализации
		//
		virtual public CNotifySource,
		//
		// Экспортируемые интерфейсы
		//
		virtual public OAF::INotifySource,
		virtual public OAF::INotifyListener,
		virtual public OAF::IPropertyBag
	{
		/**
		 * @brief Префикс
		 */
		QString m_prefix;

		/**
		 * @brief Слушать ли уведомления от объекта
		 */
		bool m_auto;

		/**
		 * @brief Ссылка на контейнер атрибутов, для которого задан данный срез
		 */
		OAF::URef<OAF::IPropertyBag> m_props;

	public:
		CSlicePropertyBag (const QString& _prefix, OAF::IPropertyBag* _props = NULL, bool _auto = true);
		CSlicePropertyBag (const CSlicePropertyBag& _bag);
		~CSlicePropertyBag ();

		/**
		 * @name Реализация методов интерфейса OAF::INotifyListener
		 */
		/** @{*/
		void notify (OAF::IInterface* _event, OAF::INotifySource* _source, OAF::INotifyListener* _origin);
		/** @}*/

		/**
		 * @name Реализация методов интерфейса OAF::IPropertyBag
		 */
		/** @{*/
		std::size_t enumKeys (KeyList& _out) const;
		PropertyFlags getFlags (const Key& /*_key*/) const;
		bool isDefined (const Key& _key) const;
		Value getValue (const Key& _key) const;
		std::size_t getValues (PropertySet& _out, PropertyFlags _f = 0) const;
		void setValue (const Key& _key, const Value& _value, OAF::INotifyListener* _origin = NULL);
		void setValues (const PropertySet& _values, OAF::INotifyListener* _origin = NULL);
		/** @}*/

		/**
		 * @brief Подключить/отключить срез
		 *
		 * @param[in] _props набор атрибутов, к которому подключается срез
		 * @param[in] _auto выполнять автоматическое подключение/отключение нотификации
		 */
		OAF::URef<OAF::IPropertyBag> attach (OAF::IPropertyBag* _props, bool _auto = true);
	};

	/**
	 * @brief Контейнер атрибутов с рассылкой уведомлений об их изменении
	 */
	class OAFCORE_EXPORT CNotifyPropertyBag :
		//
		// Импортируемые реализации
		//
		virtual public CPropertyBag,
		virtual public CNotifySource,
		//
		// Экспортируемые интерфейсы
		//
		virtual public OAF::IPropertyBag,
		virtual public OAF::INotifySource
	{
	public:
		CNotifyPropertyBag ();
		CNotifyPropertyBag (const PropertySet& _values);
		CNotifyPropertyBag (const CNotifyPropertyBag& _bag);

		/**
		 * @name Реализация методов интерфейса OAF::IPropertyBag
		 *
		 * При изменении свойств списком или по имени рассылается сообщение
		 * IPropertyEvent об изменении свойств
		 */
		/** @{*/
		void setValue (const Key& _key, const Value& _value, OAF::INotifyListener* _origin = NULL);
		void setValues (const PropertySet& _values, OAF::INotifyListener* _origin = NULL);
		/** @}*/
	};

	/**
	 * @brief Стандартная реализация контейнера атрибутов как объекта
	 *
	 * Предназначена для самостоятельного использования в качестве
	 * полноценного объекта контейнера атрибутов.
	 */
	class OAFCORE_EXPORT CPropertyBagObject :
		//
		// Импортируемые реализации
		//
		virtual public CUnknown,
		virtual public CNotifyPropertyBag,
		//
		// Экспортируемые интерфейсы
		//
		virtual public OAF::IUnknown,
		virtual public OAF::IPropertyBag,
		virtual public OAF::INotifySource
	{
	public:
		CPropertyBagObject (const QString& _cid);
		CPropertyBagObject (const QString& _cid, const PropertySet& _values);
		CPropertyBagObject (const CPropertyBagObject& _bag);
	};

	/**
	 * @brief Стандартная реализация контейнера атрибутов как объекта
	 *
	 * Предназначена для самостоятельного использования в качестве
	 * полноценного объекта контейнера атрибутов.
	 */
	class OAFCORE_EXPORT CSlicePropertyBagObject :
		//
		// Импортируемые реализации
		//
		virtual public CUnknown,
		virtual public CSlicePropertyBag,
		//
		// Экспортируемые интерфейсы
		//
		virtual public OAF::IUnknown,
		virtual public OAF::IPropertyBag,
		virtual public OAF::INotifySource
	{
	public:
		CSlicePropertyBagObject (const QString& _cid, const QString& _prefix, OAF::IPropertyBag* _props, bool _auto = true);
		CSlicePropertyBagObject (const CSlicePropertyBagObject& _bag);
	};

	/**
	 * @brief Загрузить атрибуты из заданного XML-потока
	 */
	OAFCORE_EXPORT std::size_t loadXML (OAF::IPropertyBag::PropertySet& _out, QXmlStreamReader& _xml);

	/**
	 * @brief Записать атрибуты в заданный XML-поток
	 */
	OAFCORE_EXPORT std::size_t saveXML (const OAF::IPropertyBag::PropertySet& _in, QXmlStreamWriter& _xml);
}

#endif /* __CPROPERTY_BAG_H */
