/**
 * @file
 * @brief Интерфейс базовой реализации источника уведомлений
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#ifndef __CNOTIFY_SOURCE_H
#define __CNOTIFY_SOURCE_H

#include <QtCore>

#include <idl/IInterface.h>
#include <idl/INotifySource.h>

#include <OAF/OafCoreGlobal.h>

namespace OAF
{
	/**
	 * @brief Базовая реализация источника уведомлений
	 */
	class OAFCORE_EXPORT CNotifySource :
		//
		// Экспортируемые интерфейсы
		//
		virtual public OAF::INotifySource
	{
		/**
		 * @brief Множество подключённых обработчиков
		 *
		 * Помимо указателя обработчика в данном списке запоминается и его cid. Это поможет
		 * разобраться, если при удалении нотификатора у него ещё будут подписанты.
		 */
		QMap<OAF::INotifyListener*, QString> m_listeners;

		/**
		 * @brief Признак рассылки уведомлений
		 */
		bool m_notify_in_progress;

	public:
		CNotifySource ();

		/**
		 * @brief Реально ничего не копируется
		 */
		CNotifySource (const CNotifySource& _evs);

		/**
		 * @brief При разрушении нотификатора проверяем подписантов
		 */
		~CNotifySource ();

		/**
		 * @name Реализация методов интерфейса OAF::INotifySource
		 */
		/** @{*/
		void addListener (OAF::INotifyListener* _listener);
		void removeListener (OAF::INotifyListener* _listener);
		/** @}*/

		/**
		 * @brief Рассылка уведомлений всем подписчикам
		 *
		 * @param[in] _event  рассылаемое уведомление
		 * @param[in] _origin инициатор рассылки уведомления
		 *
		 * Данная функция сделана виртуальной, чтобы её можно было переопределять, наращивая
		 * функционал рассылки уведомлений.
		 */
		virtual void notifyListeners (OAF::IInterface* _event, OAF::INotifyListener* _origin);
	};

	/**
	 * @brief Подписывает/отписывает объект от уведомлений
	 *
	 * @param[in] _i объект, посылающий события о своем изменении
	 * @param[in] _l объект, обрабатывающий уведомления
	 * @param[in] _s при true выполняется подписка, иначе отключение
	 */
	inline void
	subscribe (OAF::IInterface* _i, OAF::INotifyListener* _l, bool _s = true)
	{
		Q_ASSERT (_l);

		if (_i)
		{
			if (_s)
				OAF::queryInterface<OAF::INotifySource> (_i)->addListener (_l);
			else
				OAF::queryInterface<OAF::INotifySource> (_i)->removeListener (_l);
		}
	}

	/**
	 * @brief Отписывает/подписывает объект для получения уведомлений
	 *
	 * @param[in] _i объект, посылающий события о своем изменении
	 * @param[in] _l объект, обрабатывающий уведомления
	 * @param[in] _u при true выполняется отписка, иначе подключение
	 *
	 * Данная функция введена для тех, кому больше нравится писать разные
	 * наименования функций, а не использовать параметры.
	 */
	inline void
	unsubscribe (OAF::IInterface* _i, OAF::INotifyListener* _l, bool _u = true)
	{
		OAF::subscribe (_i, _l, !_u);
	}
}

#endif /* __CNOTIFY_SOURCE_H */
