/**
 * @file
 * @brief Реализация QTabWidget с автоматическим подключением/отключением фильтра событий
 * @author Alexander Kamyshnikov <axill777@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#include <OAF/CTabWidget.h>

void
OAF::CTabWidget::updateTabBar ()
{
	tabBar ()->setVisible (m_allways_show_tab || (count () > 1));
}

OAF::CTabWidget::CTabWidget (QWidget* _parent) : QTabWidget (_parent),
	m_event_filter (NULL), m_allways_show_tab (false)
{
	setStyleSheet ("QTabWidget[hideAloneTab=\"true\"] { border: 1px solid grey; }");

	setAllwaysShowTab (false);
}

void
OAF::CTabWidget::tabInserted (int _idx)
{
	m_widgets.insert (_idx, widget (_idx));

	if (m_event_filter)
		m_widgets[_idx]->installEventFilter (m_event_filter);

	updateTabBar ();
}

void
OAF::CTabWidget::tabRemoved (int _idx)
{
	if (m_event_filter)
		m_widgets[_idx]->removeEventFilter (m_event_filter);

	m_widgets.removeAt (_idx);

	updateTabBar ();
}

void
OAF::CTabWidget::setInstallableEventFilter (QObject* _event_filter)
{
	m_event_filter = _event_filter;
}

void
OAF::CTabWidget::setAllwaysShowTab (bool _on)
{
	m_allways_show_tab = _on;

	setProperty ("hideAloneTab", !m_allways_show_tab);

	updateTabBar ();
}
