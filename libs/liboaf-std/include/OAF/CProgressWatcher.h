/**
 * @file
 * @brief Интерфейс класса для отслеживания прогресса OAF::IOperation
 * @author Novikov Dmitriy <novikovdimka@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#ifndef __COPERATIONWATCHER_H
#define __COPERATIONWATCHER_H

#include <QtCore>

#include <idl/IOperation.h>
#include <idl/INotifySource.h>
#include <idl/IPropertyBag.h>

#include <OAF/CNotifySource.h>
#include <OAF/OafStdGlobal.h>

namespace OAF
{
	class OAFSTD_EXPORT CProgressWatcher :
		//
		// Импортируемые реализации
		//
		virtual public OAF::CNotifySource,
		//
		// Экспортируемые интерфейсы
		//
		virtual public OAF::INotifySource
	{
		/**
		 * @brief Текущее состояние операции
		 */
		OAF::IOperation::Status m_status;

		/**
		 * @brief Текущие параметры операции
		 */
		OAF::IPropertyBag::PropertySet m_values;

		/**
		 * @brief Мьютекс для безопасной рассылки уведомлений
		 */
		QMutex m_notify_mutex;

		/**
		 * @brief Рассылка уведомлений только если нет текущей рассылки
		 */
		void notifyListeners (OAF::IInterface* _event, OAF::INotifyListener* _origin);

		/**
		 * @brief Инициализация параметров операции
		 */
		void init ();

	public:
		/**
		 * @name Именования дейсвий, о которых уведомляет наблюдатель
		 */
		/** @{ */
		static QString STATUS_PROGRESS_MINIMUM;
		static QString STATUS_PROGRESS_MAXIMUM;
		static QString STATUS_PROGRESS_STEP;
		static QString STATUS_PROGRESS_VALUE;
		static QString STATUS_PROGRESS_STATUS;
		/** @} */

		CProgressWatcher ();

		/**
		 * @brief Текущий статус операции
		 */
		OAF::IOperation::Status status () const;

		/**
		 * @brief Текущие параметры операции
		 */
		const OAF::IPropertyBag::PropertySet& values () const;

		/**
		 * @brief Процесс начал выполнение
		 */
		void started ();

		/**
		 * @brief Уведомление о состоянии выполнения процесса
		 */
		void progressStatus (const QString& _status);

		/**
		 * @brief Минимальное значение прогресса
		 *
		 * @note При установке минимального значения прогресса
		 *       обнуляется текущее значение, т.е. становится
		 *       равным минимальному
		 */
		void progressMinimum (int _value);

		/**
		 * @brief Максимальное значение прогресса
		 */
		void progressMaximum (int _value);

		/**
		 * @brief Информирует о выполнении одного шага процесса
		 */
		void progressStep ();

		/**
		 * @brief Информирует о текущем значении прогресса
		 */
		void progress (int _value);

		/**
		 * @brief Процесс завершился успешно
		 */
		void finished ();

		/**
		 * @brief Процесс прерван пользователем
		 */
		void cancelled ();

		/**
		 * @brief Процесс завершился не успешно
		 */
		void aborted ();
	};
}

#endif /* __COPERATIONWATCHER_H */
