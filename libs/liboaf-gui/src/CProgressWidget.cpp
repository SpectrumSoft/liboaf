/**
 * @file
 * @brief Реализация виджета для отображения прогресса длительной операции, а также для её отмены
 * @author Alexander Kamyshnikov <axill777@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#include <OAF/Consts.h>
#include <OAF/CProgressWidget.h>
#include <OAF/CNotifySource.h>

static const char* uidef_progress_with_cancel =
		"<?xml version=\"1.0\"?>"
		"<uidef>"
			"<bag id=\"ui:statusbar\">"
				"<item id=\"piket:ui:cprogresswidget:compare_status\" priority=\"1\"/>"
				"<item id=\"piket:ui:cprogresswidget:compare_cancel\"/>"
				"<item id=\"piket:ui:cprogresswidget:compare_progress\"/>"
			"</bag>"
		"</uidef>";

static const char* uidef_progress_without_cancel =
		"<?xml version=\"1.0\"?>"
		"<uidef>"
			"<bag id=\"ui:statusbar\">"
				"<item id=\"piket:ui:cprogresswidget:compare_status\" priority=\"1\"/>"
				"<item id=\"piket:ui:cprogresswidget:compare_progress\"/>"
			"</bag>"
		"</uidef>";

void
OAF::CProgressWidget::updateOperation (const OAF::IPropertyBag::PropertySet& _v)
{
	if (_v.contains (OAF_ATTR_PROGRESS_MESSAGE))
		emit updateOperationStatus (_v[OAF_ATTR_PROGRESS_MESSAGE].toString ());
	else
		emit updateOperationStatus (m_default_status);

	if (_v.contains (OAF_ATTR_PROGRESS_MAX))
	{
		emit updateOperationProgressMinimum (0);
		emit updateOperationProgressMaximum (_v[OAF_ATTR_PROGRESS_MAX].toInt ());
	}

	if (_v.contains (OAF_ATTR_PROGRESS_VALUE))
		emit updateOperationProgressValue (_v[OAF_ATTR_PROGRESS_VALUE].toInt ());
}

void
OAF::CProgressWidget::notify (OAF::IInterface* _event, OAF::INotifySource* _source, OAF::INotifyListener* _origin)
{
	Q_UNUSED (_source);
	Q_UNUSED (_origin);

	//
	// Обработку уведомлений выполняем только в активном режиме
	//
	if (!m_is_active)
		return;

	if (OAF::IOperationNotify* opn = OAF::queryInterface<OAF::IOperationNotify> (_event))
	{
		switch (opn->status)
		{
			case OAF::IOperation::PROCESSED:
			{
				//
				// Проверяем наличие индикатора прогресса
				//
				emit updateOperationActivate (true);

				//
				// Обновляем данные по операции
				//
				updateOperation (opn->values);

				break;
			}
			case OAF::IOperation::ABORTED:
			case OAF::IOperation::CANCELLED:
			case OAF::IOperation::FINISHED:
			{
				//
				// Убираем индикатор сравнения
				//
				emit updateOperationActivate (false);

				//
				// Возвращаем состояние прогресса в исходное положение
				//
				emit updateOperationProgressValue (0);

				break;
			}

			default:
				Q_ASSERT_X (0, Q_FUNC_INFO, "Unknown operation status");
		}
	}
}

void
OAF::CProgressWidget::aboutOperationActivate (bool _activate)
{
	if (m_uic && m_is_active)
	{
		//
		// Если виджет прогресса деактивируется и находится в режиме
		// отображения хода выполнения операции, то отключаем его
		// интерфейс
		//
		if (!_activate && !m_id_operation_status.isNull ())
			m_id_operation_status = m_uic->removeUI (m_id_operation_status);

		if (_activate && m_id_operation_status.isNull ())
		{
			//
			// Если идёт выполнение операции
			//
			if (m_operation->operationStatus () == OAF::IOperation::PROCESSED)
			{
				//
				// Добавляем интерфейс
				//
				if (m_show_break_button)
					m_id_operation_status = m_uic->addUI (uidef_progress_with_cancel, this);
				else
					m_id_operation_status = m_uic->addUI (uidef_progress_without_cancel, this);

				//
				// Обновляем состояние в соответствии с текущим состоянием операции
				//
				updateOperation (m_operation->operationValues ());
			}
		}
	}
}

OAF::CProgressWidget::CProgressWidget (QWidget *_parent): QWidget (_parent),
	m_uic (NULL), m_show_break_button (true), m_is_active (false)
{
	//
	// Создаём и настраиваем виджеты отображения процесса :
	//
	// 1) состояние операции
	//
	m_operation_status.reset (new QLabel ());
	m_operation_status->setObjectName ("piket:ui:cprogresswidget:compare_status");
	//
	// 2) анимация выполнения процесса
	//
	m_operation_progress.reset (new QProgressBar ());
	m_operation_progress->setObjectName ("piket:ui:cprogresswidget:compare_progress");
	//
	// 3) кнопка отмены
	//
	m_break_operation_button.reset (new QPushButton (tr ("Break")));
	m_break_operation_button->setObjectName ("piket:ui:cprogresswidget:compare_cancel");
	connect (m_break_operation_button.data(), SIGNAL (clicked ()), this, SIGNAL (operationCancelled ()));

	//
	// Т.к. операция может выполняться в отдельном потоке, безопасно обновлять
	// интерфейс через механизм сигналов и слотов Qt
	//
	connect (this, SIGNAL (updateOperationActivate (bool)), this, SLOT (aboutOperationActivate (bool)));
	connect (this, SIGNAL (updateOperationStatus (QString)), m_operation_status.data(), SLOT (setText (QString)));
	connect (this, SIGNAL (updateOperationProgressMinimum (int)), m_operation_progress.data(), SLOT (setMinimum (int)));
	connect (this, SIGNAL (updateOperationProgressMaximum (int)), m_operation_progress.data(), SLOT (setMaximum (int)));
	connect (this, SIGNAL (updateOperationProgressValue (int)), m_operation_progress.data(), SLOT (setValue (int)));
}

OAF::CProgressWidget::~CProgressWidget ()
{
	//
	// Отписываемся от событий процесса
	//
	OAF::unsubscribe (m_operation, this);
}

void
OAF::CProgressWidget::setUIContainer (OAF::IUIContainer* _uic)
{
	m_uic = _uic;
}

OAF::IUIContainer*
OAF::CProgressWidget::getUIContainer ()
{
	return m_uic;
}

void
OAF::CProgressWidget::activate (bool _active)
{
	if (!_active)
		emit updateOperationActivate (false);

	m_is_active = _active;

	if (_active)
		emit updateOperationActivate (true);
}

QObject*
OAF::CProgressWidget::getUIItem (const QString& _id)
{
	if (_id == "piket:ui:cprogresswidget:compare_status")
		return m_operation_status.data ();

	else if (_id == "piket:ui:cprogresswidget:compare_progress")
		return m_operation_progress.data ();

	else if (_id == "piket:ui:cprogresswidget:compare_cancel")
		return m_break_operation_button.data ();

	Q_ASSERT_X (0, Q_FUNC_INFO, "Required UI item was not found");
	return NULL;
}

void
OAF::CProgressWidget::setUILabel (const QString &_bag, const QString &_label)
{
	 Q_UNUSED (_bag);
	 Q_UNUSED (_label);
}

void
OAF::CProgressWidget::setOperationObject (OAF::IOperation* _op)
{
	//
	// Задаём новый объект для отслеживания прогресса, подписываемся на все его события
	//
	OAF::unsubscribe (m_operation, this);
	m_operation = _op;
	OAF::subscribe (m_operation, this);
}

void
OAF::CProgressWidget::setDefaultStatus (const QString& _status)
{
	m_default_status = _status;
}

void
OAF::CProgressWidget::setCancelButtonVisible (bool _visible)
{
	m_show_break_button = _visible;
}
