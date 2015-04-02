/**
 * @file
 * @brief Управление временем жизни объекта
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#ifndef __IUNKNOWN_H
#define __IUNKNOWN_H

#include <QtCore>

#include <idl/IInterface.h>

namespace OAF
{
	/**
	 * @brief Интерфейс управления временем жизни объекта
	 *
	 * Данный интерфейс предназначен для управления временем жизни объекта с помощью подсчета активных
	 * ссылок на него.
	 *
	 * @note При создании объекта его счётчик ссылок равен нулю. При вызове IUnknown::unref
	 *       для такого объекта он будет удалён так же, как и при вызове этого метода для объекта со
	 *       счётчиком ссылок, равным единице. Таким образом первое увеличение счётчика ссылок означает
	 *       что у объекта появился владелец, который будет управлять его временем жизни. Такой приём
	 *       позволяет создавать объекты, обрабатывать их и передавать дальше в виде указателей без
	 *       захвата владения ими. Этот приём является просто небольшой оптимизацией для уменьшения
	 *       количества вычислений при операциях со ссылками. Однако пользоваться им не рекомендуется
	 *       так как он снижает гарантии по освобождению памяти.
	 */
	struct IUnknown : virtual public OAF::IInterface
	{
		/**
		 * @brief Увеличить счетчик активных ссылок на объект
		 *
		 * Каждый вызов данного метода увеличивает счетчик активных ссылок соответствующего объекта.
		 *
		 * @attention
		 * IDL-OUK-001. Для корректного контроля за временем жизни объекта на каждый вызов метода
		 * IUnknown::ref необходимо один раз вызывать метод IUnknown::unref после завершения
		 * использования объекта
		 *
		 * @return текущее число активных ссылок на данный объект
		 */
		virtual std::size_t ref () = 0;

		/**
		 * @brief Уменьшить счетчик активных ссылок на объект
		 *
		 * Каждый вызов данного метода приводит к уменьшению счетчика активных ссылок на единицу для
		 * соответствующего объекта.
		 *
		 * @attention
		 * IDL-OUK-002. При достижении счетчиком ссылок нулевого значения, объект должен быть удалён из
		 * памяти и запрещается любое использование указателей на этот объект.
		 *
		 * @return Оставшееся число активных ссылок на данный объект
		 */
		virtual std::size_t unref () = 0;

		/**
		 * @brief Число активных ссылок на объект
		 */
		virtual std::size_t refs () const = 0;

		/**
		 * @brief Идентифкатор OAF-класса данного объекта
		 */
		virtual const QString& cid () const = 0;
	};

	/**
	 * @brief Ссылка на объект, экспортирующий интерфейс IUnknown
	 *
	 * Этот шаблон предназначен для автоматизации подсчета активных ссылок на объект. Он удобен для
	 * предотвращения утечек памяти при обработке исключений и организации автоматических контейнеров
	 * объектов.
	 *
	 * Данный класс реализован как часть определения интерфейса IUnknown так как он используется
	 * при определении других интерфейсов.
	 *
	 * @note Предполагается, что при создании объекта и возвращении указателя на него он имеет счётчик
	 *       ссылок равный нулю
	 */
	template <class _Object>
	class URef
	{
		/**
		 * @brief Указатель на объект, которым управляет ссылка
		 */
		_Object* m_ref;

		/**
		 * @brief Функция увеличения счетчика ссылок с проверкой на NULL
		 */
		static void
		ref (_Object* _o)
		{
			if (_o)
			{
				OAF::IUnknown* o = OAF::queryInterface<OAF::IUnknown> (_o);

				//
				// Объект обязан экспортировать интерфейс OAF::IUnknown
				//
				Q_ASSERT (o != NULL);

				o->ref ();
			}
		};

		/**
		 * @brief Функция уменьшения счетчика ссылок с проверкой на NULL
		 */
		static void
		unref (_Object* _o)
		{
			if (_o)
			{
				OAF::IUnknown* o = OAF::queryInterface<OAF::IUnknown> (_o);

				//
				// Объект обязан экспортировать интерфейс OAF::IUnknown
				//
				Q_ASSERT (o != NULL);

				o->unref ();
			}
		};

	public:
		/**
		 * @brief Конструктор ссылки по умолчанию
		 */
		URef () : m_ref (NULL)
		{}
		;

		/**
		 * @brief Конструктор ссылки из указателя на объект
		 */
		URef (_Object* _ref) : m_ref (_ref)
		{
			ref (m_ref);
		};

		/**
		 * @brief Конструктор копии ссылки
		 */
		URef (const URef& _ref) : m_ref (_ref.m_ref)
		{
			ref (m_ref);
		};

		/**
		 * @brief Уничтожение ссылки
		 */
		~URef ()
		{
			unref (m_ref);
		};

		/**
		 * @brief Присвоение ссылке нового указателя на объект
		 */
		URef& operator= (_Object* _ref)
		{
			if (m_ref != _ref)
			{
				unref (m_ref);
				m_ref = _ref;
				ref (m_ref);
			}

			return *this;
		};

		/**
		 * @brief Присвоение ссылке нового значения в виде другой ссылки
		 */
		URef& operator= (const URef& _ref)
		{
			if (m_ref != _ref.m_ref)
			{
				unref (m_ref);
				m_ref = _ref;
				ref (m_ref);
			}

			return *this;
		};

		/**
		 * @brief Операция разыменования ссылки
		 */
		_Object& operator* () const
		{
			return *m_ref;
		};

		/**
		 * @brief Операция преобразования ссылки в указатель на объект
		 */
		operator _Object* () const
		{
			return m_ref;
		};

		/**
		 * @brief Операция разыменования ссылки как указателя на объект
		 */
		_Object* operator-> () const
		{
			return m_ref;
		};

		/**
		 * @brief Явное получение указателя
		 */
		_Object* ptr () const
		{
			return m_ref;
		};

		/**
		 * @brief Проверка ссылки на достоверность
		 */
		operator bool () const
		{
			return (m_ref != NULL);
		};

		/**
		 * @brief Проверка ссылки на NULL
		 */
		bool isNull () const
		{
			return (m_ref == NULL);
		};

		/**
		 * @brief Проверка ссылки на значение
		 */
		bool is (_Object* _ptr) const
		{
			return (m_ref == _ptr);
		};

		/**
		 * @brief Преобразование к указателю на другой совместимый тип
		 */
		template<class _Object2>
		_Object2* queryInterface () const
		{
			return OAF::queryInterface<_Object2> (m_ref);
		};

		/**
		 * @brief Равенство ссылок определяется равенством адресов объектов
		 */
		friend bool operator== (const OAF::URef<_Object>& _ref1, const OAF::URef<_Object>& _ref2)
		{
			return (_ref1.m_ref == _ref2.m_ref);
		};

		/**
		 * @brief Не равенство ссылок определяется не равенством адресов объектов
		 */
		friend bool operator!= (const OAF::URef<_Object>& _ref1, const OAF::URef<_Object>& _ref2)
		{
			return (_ref1.m_ref != _ref2.m_ref);
		};

		/**
		 * @brief Упорядочивание ссылок по возрастанию адресов объектов
		 *
		 * Это упорядочивание не имеет никакого смысла за исключением возможности
		 * использовать ссылки в качестве ключей для STL-контейнеров std::set и
		 * std::map.
		 */
		friend bool operator< (const OAF::URef<_Object>& _ref1, const OAF::URef<_Object>& _ref2)
		{
			return (_ref1.m_ref < _ref2.m_ref);
		};

		/**
		 * @brief Упорядочивание ссылок по убыванию
		 */
		friend bool operator> (const OAF::URef<_Object>& _ref1, const OAF::URef<_Object>& _ref2)
		{
			return (_ref1.m_ref > _ref2.m_ref);
		};
	};

	/**
	 * @brief Интерфейс расширения объекта
	 */
	struct IUnknownExtender : virtual public OAF::IInterface
	{
		/**
		 * @brief Задание объекта для расширения
		 *
		 * @attention
		 * IDL-OUK-003. Клиент данного интерфейса должен иметь в виду, что в качестве результата
		 * вызова этого метода ему может вернуться ссылка на другой объект, который он и должен
		 * будет использовать.
		 *
		 * @attention
		 * IDL-OUK-004. Помимо списка интерфейсов, которые поддерживает OAF-класс, реализующий
		 * интерфейс IUnknownExtender для него должен быть задан атрибут extends типа stringv,
		 * в котором перечислены все CID OAF-классов, которые данный класс может обрабатывать.
		 *
		 * @return Объект, связанный с заданным объектом
		 */
		virtual OAF::URef<OAF::IUnknown> setExtendedObject (OAF::IUnknown* _o) = 0;
	};

	/**
	 * @brief Интерфейс проксирования объекта
	 *
	 * Дополнительный интерфейс, который позволяет определять объекты-прокси, связанные
	 * с заданными OAF-классами объектов и доступные через стандартные механизмы доступа
	 * к объектам-расширениям, реализующим дополнительные интерфейсы.
	 *
	 * Данный интерфейс предназначен для создания структур с разделяемыми данными.
	 */
	struct IUnknownProxy : virtual public OAF::IUnknown, virtual public OAF::IUnknownExtender
	{
		/**
		 * @brief Запросить проксируемый объект
		 */
		virtual OAF::URef<OAF::IUnknown> getExtendedObject () = 0;
	};
}

#endif /* __IUNKNOWN_H */
