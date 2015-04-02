/**
 * @file
 * @brief Реализация помощника, для изменения размеров QDockWidget
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#include <OAF/DockResizeHelper.h>

using namespace OAF::Helpers;

DockResizeHelper::DockResizeHelper (QDockWidget* _dock, QObject* _parent) :
	QObject (_parent), m_dock (_dock)
{
	Q_ASSERT (_dock);
}

void
DockResizeHelper::aboutReturnToOldMaxMinSizes ()
{
	m_dock->setMinimumSize (m_old_min_size);
	m_dock->setMaximumSize (m_old_max_size);
}

void
DockResizeHelper::setSize (const QSize& _sz)
{
	m_old_min_size = m_dock->minimumSize ();
	m_old_max_size = m_dock->maximumSize ();

	if (0 <= _sz.width ())
	{
		if (m_dock->width() < _sz.width ())
			m_dock->setMinimumWidth (_sz.width ());
		else
			m_dock->setMaximumWidth (_sz.width ());
	}

	if (0 <= _sz.height ())
	{
		if (m_dock->height() < _sz.height ())
			m_dock->setMinimumHeight (_sz.height ());
		else
			m_dock->setMaximumHeight (_sz.height ());
	}

	QTimer::singleShot (1, this, SLOT (aboutReturnToOldMaxMinSizes ()));
}
