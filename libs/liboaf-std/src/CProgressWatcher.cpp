/**
 * @file
 * @brief Реализация класса для отслеживания прогресса OAF::IOperation
 * @author Novikov Dmitriy <novikovdimka@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#include <OAF/Consts.h>
#include <OAF/CProgressWatcher.h>

static const int PROGRESS_DEFAULT_VALUE = 0;
static const int PROGRESS_STEP_VALUE    = 1;

QString OAF::CProgressWatcher::STATUS_PROGRESS_MINIMUM = "progress_minimum";
QString OAF::CProgressWatcher::STATUS_PROGRESS_MAXIMUM = "progress_max";
QString OAF::CProgressWatcher::STATUS_PROGRESS_STEP    = "progress_step";
QString OAF::CProgressWatcher::STATUS_PROGRESS_VALUE   = "progress_value";
QString OAF::CProgressWatcher::STATUS_PROGRESS_STATUS  = "message";

void
OAF::CProgressWatcher::notifyListeners(OAF::IInterface* _event, OAF::INotifyListener* _origin)
{
	if (m_notify_mutex.tryLock ())
	{
		OAF::CNotifySource::notifyListeners (_event, _origin);
		m_notify_mutex.unlock ();
	}
}

void
OAF::CProgressWatcher::init ()
{
	m_status = OAF::IOperation::FINISHED;
	m_values.clear ();
}

OAF::CProgressWatcher::CProgressWatcher ()
{
	init ();
}

OAF::IOperation::Status
OAF::CProgressWatcher::status () const
{
	return m_status;
}

const OAF::IPropertyBag::PropertySet&
OAF::CProgressWatcher::values () const
{
	return m_values;
}

void
OAF::CProgressWatcher::started ()
{
	init ();

	m_status = OAF::IOperation::PROCESSED;

	OAF::IOperationNotify event (m_status, m_values);
	notifyListeners (&event, NULL);
}

void
OAF::CProgressWatcher::progressStatus (const QString& _status)
{
	m_values[STATUS_PROGRESS_STATUS] = _status;

	OAF::IOperationNotify event (m_status, m_values);
	notifyListeners (&event, NULL);
}

void
OAF::CProgressWatcher::progressMinimum (int _value)
{
	m_values[STATUS_PROGRESS_MINIMUM] = _value;

	//
	// Обнуляем текущее значение прогресса
	//
	progress (_value);
}

void
OAF::CProgressWatcher::progressMaximum (int _value)
{
	m_values[STATUS_PROGRESS_MAXIMUM] = _value;

	OAF::IOperationNotify event (m_status, m_values);
	notifyListeners (&event, NULL);
}

void
OAF::CProgressWatcher::progressStep ()
{
	//
	// Уведомляем о выполненном шаге
	//
	m_values[STATUS_PROGRESS_STEP] = true;

	//
	// Уведомляем об изменении текущего значения прогресса
	//
	progress ((m_values.contains (STATUS_PROGRESS_VALUE) ? m_values[STATUS_PROGRESS_VALUE].toInt () : 0) + PROGRESS_STEP_VALUE);
}

void
OAF::CProgressWatcher::progress (int _value)
{
	//
	// Обновляем текущее значение прогресса
	//
	if (m_values.contains(STATUS_PROGRESS_MAXIMUM))
	{
		if (_value <= m_values[STATUS_PROGRESS_MAXIMUM].toInt ())
			m_values[STATUS_PROGRESS_VALUE] = _value;
		else
			m_values[STATUS_PROGRESS_VALUE] = m_values[STATUS_PROGRESS_MAXIMUM].toInt ();
	}
	else
		m_values[STATUS_PROGRESS_VALUE] = 0;

	OAF::IOperationNotify event (m_status, m_values);
	notifyListeners (&event, NULL);
}

void
OAF::CProgressWatcher::finished ()
{
	m_status = OAF::IOperation::FINISHED;

	OAF::IOperationNotify event (m_status, m_values);
	notifyListeners (&event, NULL);
}

void
OAF::CProgressWatcher::cancelled ()
{
	m_status = OAF::IOperation::CANCELLED;

	OAF::IOperationNotify event (m_status, m_values);
	notifyListeners (&event, NULL);
}

void
OAF::CProgressWatcher::aborted ()
{
	m_status = OAF::IOperation::ABORTED;

	OAF::IOperationNotify event (m_status, m_values);
	notifyListeners (&event, NULL);
}
