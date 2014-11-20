/**
 * @file
 * @brief Базовая реализация источника уведомлений
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#include <OAF/TypeUtils.h>
#include <OAF/Utils.h>

#include <OAF/CNotifySource.h>

OAF::CNotifySource::CNotifySource () : m_notify_in_progress (false)
{}

OAF::CNotifySource::CNotifySource (const CNotifySource& _evs) : OAF::IInterface (_evs), OAF::INotifySource (_evs),
	m_notify_in_progress (false)
{}

OAF::CNotifySource::~CNotifySource()
{
	for (QMap<OAF::INotifyListener*, QString>::const_iterator l = m_listeners.begin (); l != m_listeners.end (); ++l)
		qWarning ("%s: %s not unregistered", Q_FUNC_INFO, qPrintable (l.value ()));
}

void
OAF::CNotifySource::addListener (OAF::INotifyListener* _listener)
{
	Q_ASSERT_X (!m_notify_in_progress, Q_FUNC_INFO, "Add listener not allowed during notify handling");
	Q_ASSERT_X (_listener, Q_FUNC_INFO, "Listener is NULL");

	//
	// Определяем класс слушателя (делаем это через механизмы RTTI C++, так как не все
	// классы реализуют интерфейс OAF::IUnknown)
	//
	QString cid = OAF::interfaceName (typeid (*_listener));

	//
	// Проверяем, что данный слушатель ещё не подключался. Это не является проблемой, просто
	// может свидетельствовать о наличии ошибки в коде, поэтому просто предупреждаем
	//
	if (m_listeners.contains (_listener))
	{
		qWarning ("%s: listener '%s' is registered allready", Q_FUNC_INFO, qPrintable (cid));
		return;
	}

	m_listeners.insert (_listener, cid);
}

void
OAF::CNotifySource::removeListener (OAF::INotifyListener* _listener)
{
	Q_ASSERT_X (!m_notify_in_progress, Q_FUNC_INFO, "Remove listener not allowed during notify handling");
	Q_ASSERT_X (_listener, Q_FUNC_INFO, "Listener is NULL");

	//
	// Проверяем, что данный слушатель подключён. Это не является проблемой, просто
	// может свидетельствовать о наличии ошибки в коде, поэтому просто предупреждаем
	//
	if (!m_listeners.contains (_listener))
	{
		//
		// Определяем класс слушателя (делаем это через механизмы RTTI C++, так как не все
		// классы реализуют интерфейс OAF::IUnknown)
		//
		QString cid = OAF::interfaceName (typeid (*_listener));

		qWarning ("%s: listener '%s' is unregistered allready", Q_FUNC_INFO, qPrintable (cid));
		return;
	}

	m_listeners.remove (_listener);
}

void
OAF::CNotifySource::notifyListeners (OAF::IInterface* _event, OAF::INotifyListener* _origin)
{
	Q_ASSERT_X (!m_notify_in_progress, Q_FUNC_INFO, "Notification not allowed during notify handling");
	Q_ASSERT_X (_event, Q_FUNC_INFO, "Event is NULL");

	OAF::ScopedValue<bool> notify_in_progress (m_notify_in_progress, true, false);
	for (QMap<OAF::INotifyListener*, QString>::const_iterator l = m_listeners.begin (); l != m_listeners.end (); ++l)
		l.key ()->notify (_event, this, _origin);
}
