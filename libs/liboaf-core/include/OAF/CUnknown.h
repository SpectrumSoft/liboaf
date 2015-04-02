/**
 * @file
 * @brief Интерфейс стандартной реализации OAF::IUnknown
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#ifndef __CUNKNOWN_H
#define __CUNKNOWN_H

#include <QtCore>

#include <idl/IInterface.h>
#include <idl/IUnknown.h>

#include <OAF/OafCoreGlobal.h>

namespace OAF
{
	/**
	 * @brief Стандартная реализация интерфейса OAF::IUnknown
	 *
	 * При создании счётчик ссылок всегда инициализируется в 0, так как наиболее
	 * частым случаем является присвоение указателя на объект OAF::URef ссылке на него,
	 * которая автоматически увеличит счётчик ссылок до 1.
	 */
	class OAFCORE_EXPORT CUnknown :
			//
			// Экспортируемые интерфейсы
			//
			virtual public OAF::IUnknown
	{
		/**
		 * @brief Счетчик ссылок на данный объект
		 */
		std::size_t m_refs;

		/**
		 * @brief Идентификатор OAF-класса объекта
		 */
		QString m_cid;

		/**
		 * @brief Глобальная таблица указателей на объекты
		 *
		 * Предназначена для отслеживания текущего размещения объектов в памяти.
		 * В частности позволяет обнаруживать не освобождение объектов при выходе
		 * из программы.
		 */
		static QSet<CUnknown*> m_objects;

	public:
		CUnknown (const QString& _cid);
		CUnknown (const CUnknown& _ou);
		~CUnknown ();

		/**
		 * @name Реализация интерфейса OAF::IUnknown
		 */
		/** @{*/
		std::size_t ref ();
		std::size_t unref ();
		std::size_t refs () const;
		const QString& cid () const;
		/** @}*/

		/**
		 * @brief Список объектов
		 */
		static const QSet<CUnknown*>& objects ();

		/**
		 * @brief Проверка валидности объекта
		 */
		static bool isValid (CUnknown* _ou);
	};
}

#endif /* __COBJECT_UNKNOWN_H */
