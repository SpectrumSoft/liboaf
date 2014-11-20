/**
 * @file
 * @brief Список атрибутов
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#ifndef __IPROPERTY_BAG_H
#define __IPROPERTY_BAG_H

#include <QString>
#include <QVariant>
#include <QStringList>

#include <idl/IInterface.h>
#include <idl/IUnknown.h>
#include <idl/INotifySource.h>

namespace OAF
{
	/**
	 * @brief Список атрибутов
	 *
	 * @attention
	 * IDL-PBG-001. При установке значения атрибута в QVariant::Invalid данный атрибут должен быть
	 * удалён из списка атрибутов.
	 */
	struct IPropertyBag : virtual public OAF::IInterface
	{
		/**
		 * @brief Ключ атрибута
		 */
		typedef QString Key;

		/**
		 * @brief Значение атрибута
		 */
		typedef QVariant Value;

		/**
		 * @brief Список ключей
		 */
		typedef QStringList KeyList;

		/**
		 * @brief Список атрибутов
		 */
		typedef QVariantMap PropertySet;

		/**
		 * @name Флаги, ассоциированные с атрибутами
		 */
		/** @{*/
		/**
		 * @brief Тип набора флагов
		 */
		typedef quint32 PropertyFlags;

		/**
		 * @brief Атрибут доступен для записи
		 */
		static const PropertyFlags PROPERTY_WRITEABLE = 1 << 0;
		/**
		 * @brief Принудительная рассылка уведомлений об изменении атрибута
		 *
		 * Данный флаг показывает, что уведомление об изменении данного атрибута нужно
		 * рассылать при изменении любого другого атрибута
		 */
		static const PropertyFlags PROPERTY_AUTONOTIFY = 1 << 1;
		/**
		 * @brief Атрибут сохраняется между запусками программы
		 */
		static const PropertyFlags PROPERTY_PERSIST = 1 << 2;
		/**
		 * @brief Атрибут индексируется
		 */
		static const PropertyFlags PROPERTY_INDEX = 1 << 3;
		/**
		 * @brief Атрибут не должен передаваться во внешние системы
		 */
		static const PropertyFlags PROPERTY_PRIVATE = 1 << 4;
		/**
		 * @brief Атрибут является не стандартным
		 *
		 * Понятие не стандартности определяется реализацией. Например это могут быть вычисляемые
		 * атрибуты или атрибуты с особым способом хранения и установки
		 */
		static const PropertyFlags PROPERTY_VIRTUAL = 1 << 5;
		/** @}*/

		/**
		 * @brief Получить список ключей
		 *
		 * @param[out] _out отобранные ключи
		 *
		 * @rеturn количество ключей
		 */
		virtual std::size_t enumKeys (KeyList& _out) const = 0;

		/**
		 * @brief Получить флаги, ассоциированные с заданным атрибутом
		 */
		virtual PropertyFlags getFlags (const Key& _key) const = 0;

		/**
		 * @brief Запросить наличие атрибута
		 */
		virtual bool isDefined (const Key& _key) const = 0;

		/**
		 * @brief Получить значение атрибута
		 */
		virtual Value getValue (const Key& _key) const = 0;

		/**
		 * @brief Получить полный список атрибутов с заданным набором флагов
		 *
		 * @return количество атрибутов
		 */
		virtual std::size_t getValues (PropertySet& _out, PropertyFlags _f = 0) const = 0;

		/**
		 * @brief Установить новое значение атрибута
		 *
		 * @param[in] _key    идентификатор атрибута
		 * @param[in] _value  новое значение атрибута
		 * @param[in] _origin инициатор изменения атрибута
		 *
		 * Реализация данного метода должна удовлетворять требованию IDL-PBG-002.
		 */
		virtual void setValue (const Key& _key, const Value& _value, OAF::INotifyListener* _origin = NULL) = 0;

		/**
		 * @brief Установить заданный набор атрибутов
		 *
		 * @param[in] _values список изменяемых атрибутов
		 * @param[in] _origin инициатор изменения атрибутов
		 *
		 * В списке @a _values можно задавать только изменяемые атрибуты. Значения не перечисленных
		 * в данном списке атрибутов останутся неизменными. Для того, чтобы удалить атрибут для него
		 * нужно явно задать NULL-значение.
		 *
		 * @attention
		 * IDL-PBG-002. Данная операция может изменить, добавить, удалить только атрибуты, отмеченные
		 * признаком OAF::IPropertyBag::PROPERTY_WRITEABLE
		 */
		virtual void setValues (const PropertySet& _values, OAF::INotifyListener* _origin = NULL) = 0;
	};

	/**
	 * @brief Интерфейс расширения доступа к атрибутам
	 */
	struct IPropertyBagExtender : virtual public OAF::IInterface
	{
		/**
		 * @brief Задание атрибутов для расширения
		 *
		 * @attention
		 * IDL-PBG-004. Клиент данного интерфейса должен иметь в виду, что в качестве результата
		 * вызова этого метода ему может вернуться ссылка на другой объект, который он и должен
		 * будет использовать.
		 *
		 * @return Объект, связанный с заданным набором атрибутов
		 */
		virtual OAF::URef<OAF::IUnknown> setPropertyBag (OAF::IPropertyBag* _bag) = 0;
	};

	/**
	 * @brief Уведомление об изменении свойств
	 *
	 * Используется в реализациях, которые предполагают рассылку уведомлений об изменениях своих
	 * атрибутов. Такие объекты должны также поддерживать интерфейс OAF::INotifySource.
	 *
	 * @attention
	 * IDL-PBG-005. При изменении атрибутов списком такое событие рассылается для всех изменяемых
	 * атрибутов
	 */
	struct IPropertyNotify : virtual public OAF::IInterface
	{
		Q_DISABLE_COPY (IPropertyNotify)

		/**
		 * @brief Исключение, позволяющее отменить изменения атрибута
		 *
		 * Обработчик PropertyEvent на этапе PropertyEvent::BEFORE может выбросить это исключение для
		 * того, чтобы прервать изменения атрибута.
		 *
		 * @attention
		 * IDL-PBG-006. При возникновении данного исключения изменение атрибута должно быть прервано и
		 * всем подписчикам должно быть разослано сообщение IPropertyEvent::CANCELLED для того, чтобы
		 * они могли соответствующим образом среагировать на отказ в изменении атрибута
		 */
		struct Cancel
		{}
		;

		/**
		 * @brief Этапы выполнения изменения атрибута
		 */
		enum When
		{
			BEFORE,   //!< Перед изменением атрибута
			AFTER,    //!< После изменения атрибута
			CANCELLED //!< При отмене изменения атрибута
		};

		/**
		 * @brief Текущий этап изменения атрибута
		 */
		const When when;

		/**
		 * @brief Список изменяемых атрибутов
		 *
		 * В идеале данный список должен содержать только изменяемые ключи, но это не обязательно.
		 *
		 * @warning Поскольку список рассылается как ссылка, то рассыльщик уведомлений должен
		 *          гарантировать, что этот список будет существовать всё время обработки
		 *          уведомлений
		 */
		const OAF::IPropertyBag::PropertySet& values;

		/**
		 * @brief Изменённый объект
		 *
		 * Данный параметр может не задаваться при рассылке уведомлений. Это зависит от их источника.
		 */
		OAF::IInterface* const object;

		/**
		 * @brief Конструктор уведомления
		 */
		IPropertyNotify (When _when, const OAF::IPropertyBag::PropertySet& _values, OAF::IInterface* _object) :
			when (_when), values (_values), object (_object)
		{}
		;
	};
}

#endif /* __IPROPERTY_BAG_H */
