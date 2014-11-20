/**
 * @file
 * @brief Интерфейсы для управления длительными операциями
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#ifndef __IOPERATION_H
#define __IOPERATION_H

#include <idl/IInterface.h>
#include <idl/IPropertyBag.h>
#include <idl/INotifySource.h>

namespace OAF
{
	/**
	 * @brief Интерфейс для управления выполнением длительных операций
	 *
	 * С точки зрения изменения статусов выполнение операций выглядит
	 * следующим образом:
	 * # FINISHED|CANCELLED|ABORTED;
	 * # PROCESSED;
	 * # FINISHED|CANCELLED|ABORTED.
	 *
	 * Поскольку в общем случае объект, реализующий данный интерфейс, может
	 * поддерживать несколько длительных операций, то все методы интерфейса
	 * имеют параметр @a _options, с помощью которого можно как передавать
	 * параметры запуска операций, так и отслеживать статусы и параметры
	 * нескольких параллельно выполняющихся операций.
	 */
	struct IOperation : virtual public OAF::IInterface
	{
		/**
		 * @brief Статус операции
		 */
		enum Status
		{
			/**
			 * @brief Операция выполняется
			 */
			PROCESSED,

			/**
			 * @brief Последняя операция завершилась успешно или никакая операция ещё не выполнялась
			 */
			FINISHED,

			/**
			 * @brief Последняя операция была прервана пользователем
			 */
			CANCELLED,

			/**
			 * @brief Последняя операция завершилась не успешно
			 */
			ABORTED
		};

		/**
		 * @brief Статус операции
		 */
		virtual Status operationStatus (const IPropertyBag* _options = NULL) const = 0;

		/**
		 * @brief Параметры выполнения операции
		 */
		virtual const OAF::IPropertyBag::PropertySet& operationValues (const IPropertyBag* _options = NULL) const = 0;

		/**
		 * @brief Запустить операцию
		 */
		virtual void operationStart (const IPropertyBag* _options = NULL) = 0;

		/**
		 * @brief Прервать операцию
		 */
		virtual void operationStop (const IPropertyBag* _options = NULL) = 0;
	};

	/**
	 * @brief Уведомления, генерируемые объектом в процессе выполнения операции
	 */
	struct IOperationNotify : virtual public OAF::IInterface
	{
		Q_DISABLE_COPY(IOperationNotify)

		/**
		 * @brief Статус операции
		 *
		 * В начале выполнения операции и в её процессе генерируеся уведомление
		 * PROCESSED. Конкретные параметры выполнения операции зависят от реализации).
		 * При завершении операции генерируется уведомление FINISHED (полное заверщение
		 * операции) или CANCELLED (операция прервана вызовом OAF::IOperation::operationStop).
		 */
		const OAF::IOperation::Status status;

		/**
		 * @brief Параметры выполнения операции
		 */
		const OAF::IPropertyBag::PropertySet& values;

		/**
		 * @brief Конструктор уведомления
		 *
		 * После создания уведомления его атрибуты не могут быть изменены.
		 */
		/** @{ */
		IOperationNotify (OAF::IOperation::Status _status, const OAF::IPropertyBag::PropertySet& _values) :
			status (_status), values (_values)
		{}
		;
		/** @} */
	};
}

#endif /* __IOPERATION_H */
