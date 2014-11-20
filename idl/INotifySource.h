/**
 * @file
 * @brief Пакет интерфейсов подсистемы уведомлений
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 *
 * Данный пакет определяет набор интерфейсов для реализации уведомлений без постоянного
 * хранения. То есть такого рода уведомления должны обрабатываться в момент возникновения
 * и после аннулироваться.
 */
#ifndef __INOTIFY_SOURCE_H
#define __INOTIFY_SOURCE_H

#include <idl/IInterface.h>

namespace OAF
{
	struct INotifySource;
}

namespace OAF
{
	/**
	 * @brief Обработчик уведомлений
	 *
	 * @attention
	 * IDL-EVS-001. Обработчики уведомлений не должны предполагать управление их временем жизни со
	 * стороны источника уведомлений так как по сути данный набор интерфейсов - это описание обратных
	 * вызовов
	 *
	 * @see OAF::INotifySource
	 */
	struct INotifyListener : virtual public OAF::IInterface
	{
		/**
		 * @brief Обработать уведомление
		 *
		 * @param[in] _event  уведомление
		 * @param[in] _source источник уведомления
		 * @param[in] _origin инициатор уведомления
		 */
		virtual void notify (OAF::IInterface* _event, OAF::INotifySource* _source, OAF::INotifyListener* _origin) = 0;
	};

	/**
	 * @brief Источник уведомлений
	 *
	 * @attention
	 * IDL-EVS-002. Для обработки уведомлений необходимо реализовать интерфейс OAF::INotifyListener и
	 * зарегистрироваться в их источнике
	 */
	struct INotifySource : virtual public OAF::IInterface
	{
		/**
		 * @brief Подключить обработчик уведомлений к списку рассылки
		 *
		 * @param[in] _listener обработчик уведомлений
		 */
		virtual void addListener (OAF::INotifyListener* _listener) = 0;

		/**
		 * @brief Отключить обработчик уведомлений от списка рассылки
		 *
		 * @param[in] _listener обработчик уведомлений
		 */
		virtual void removeListener (OAF::INotifyListener* _listener) = 0;
	};
}

#endif /* __INOTIFY_SOURCE_H */
