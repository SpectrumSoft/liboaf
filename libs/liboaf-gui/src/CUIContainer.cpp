/**
 * @file
 * @brief Базовые реализации контейнеров компонентов
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#include <OAF/CUIContainer.h>

OAF::CUIContainer_Simple::CUIContainer_Simple () : m_uic (NULL)
{}

OAF::CUIContainer_Simple::~CUIContainer_Simple ()
{}

QUuid
OAF::CUIContainer_Simple::addUI (const QString& _uidef, OAF::IUIComponent* _uic)
{
	Q_ASSERT (m_uic != NULL);

	return m_uic->addUI (_uidef, _uic);
}

QUuid
OAF::CUIContainer_Simple::removeUI (const QUuid& _ui)
{
	Q_ASSERT (m_uic != NULL);

	return m_uic->removeUI (_ui);
}

void
OAF::CUIContainer_Simple::_setUIContainer (OAF::IUIContainer* _uic)
{
	m_uic = _uic;
}

OAF::IUIContainer*
OAF::CUIContainer_Simple::_getUIContainer ()
{
	return m_uic;
}

OAF::CUIContainer_Managed::CUIContainer_Managed ()
{}

OAF::CUIContainer_Managed::~CUIContainer_Managed ()
{}

QUuid
OAF::CUIContainer_Managed::addUI (const QString& _uidef, OAF::IUIComponent* _uic)
{
	//
	// Добавляем интерфейс к своим контейнерам интерфейса
	//
	QUuid ui = m_manager.addUI (_uidef, _uic);

	//
	// Передаём оставшееся описание интерфейса компонента выше по иерархии
	//
	m_uilinks[ui] = CUIContainer_Simple::addUI (m_manager.purgeUI (_uidef), _uic);

	return ui;
}

QUuid
OAF::CUIContainer_Managed::removeUI (const QUuid& _ui)
{
	//
	// Удаляем интерфейс из компонента-контейнера
	//
	if (m_uilinks.find (_ui) != m_uilinks.end ())
	{
		CUIContainer_Simple::removeUI (m_uilinks[_ui]);

		m_uilinks.remove (_ui);
	}

	//
	// Удаляем интерфейс из своих контейнеров
	//
	return m_manager.removeUI (_ui);
}

OAF::CUIManager&
OAF::CUIContainer_Managed::_getUIManager ()
{
	return m_manager;
}
