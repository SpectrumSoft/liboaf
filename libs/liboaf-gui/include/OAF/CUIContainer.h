/**
 * @file
 * @brief Базовые реализации контейнеров компонентов
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#ifndef __CUICONTAINER_H
#define __CUICONTAINER_H

#include <QtCore>
#include <QtGui>
#if (QT_VERSION >= QT_VERSION_CHECK (5, 0, 0))
#include <QtWidgets>
#endif

#include <idl/IUIComponent.h>

#include <OAF/CUIManager.h>
#include <OAF/OafGuiGlobal.h>

namespace OAF
{
	/**
	 * @brief Контейнер компонентов
	 *
	 * Этот класс реализует простейшую модель контейнера компонентов. В этой модели компонент
	 * обеспечивает только трансляцию запросов к интерфейсу OAF::IUIContainer от компонента к своему
	 * контейнеру
	 */
	class OAFGUI_EXPORT CUIContainer_Simple :
		//
		// Экспортируемые интерфейсы
		//
		virtual public OAF::IUIContainer
	{
		/**
		 * @brief Контейнер, к которому подключён этот контейнер
		 */
		OAF::IUIContainer* m_uic;

	public:
		CUIContainer_Simple ();

		virtual ~CUIContainer_Simple ();

		/**
		 * @name Реализация методов интерфейса OAF::IUIContainer
		 */
		/** @{*/
		QUuid addUI (const QString& _uidef, OAF::IUIComponent* _uic);
		QUuid removeUI (const QUuid& _ui);
		/** @}*/

		/**
		 * @brief Задать родительский контейнер
		 */
		void _setUIContainer (OAF::IUIContainer* _uic);

		/**
		 * @brief Вернуть указатель на родительский контейнер
		 */
		OAF::IUIContainer* _getUIContainer ();
	};

	/**
	 * @brief Контейнер компонентов с менеджером интерфейса пользователя
	 *
	 * Это реализация модели контейнера компонентов, который предоставляет подключаемым компонентам
	 * дополнителные контейнеры интерфейса пользователя
	 */
	class OAFGUI_EXPORT CUIContainer_Managed :
		//
		// Импортируемые реализации
		//
		virtual public CUIContainer_Simple
	{
		/**
		 * @brief Менеджер контейнеров интерфейса этого контенера компонентов
		 */
		CUIManager m_manager;

		/**
		 * @brief Карта идентификаторов интерфейса
		 */
		QMap<QUuid, QUuid> m_uilinks;

	public:
		CUIContainer_Managed ();

		virtual ~CUIContainer_Managed ();

		/**
		 * @name Дополнительная семантика методов интерфейса OAF::IUIContainer
		 */
		/** @{*/
		QUuid addUI (const QString& _uidef, OAF::IUIComponent* _uic);
		QUuid removeUI (const QUuid& _ui);
		/** @}*/

		CUIManager& _getUIManager ();
	};

	/**
	 * @brief Найти заданный интерфейс
	 *
	 * Возвращается заданный интерфейс любого компонента, расположенного выше по иерархии
	 */
	template<class _IInterface>
	_IInterface*
	findInterface (OAF::IUIComponent* _uic)
	{
		if (_uic)
		{
			if (_IInterface* i = OAF::queryInterface<_IInterface> (_uic))
				return i;

			if (OAF::IUIContainer* c = _uic->getUIContainer ())
				return OAF::findInterface<_IInterface> (OAF::queryInterface<OAF::IUIComponent> (c));
		}

		return NULL;
	}

	/**
	 * @brief Найти заданный интерфейс в иерархии виджетов
	 *
	 * Возвращается заданный интерфейс любого объекта, расположенного выше по иерархии
	 */
	template<class _IInterface>
	_IInterface*
	findInterface (QObject* _w)
	{
		if (_w)
		{
			if (_IInterface* i = OAF::queryInterface<_IInterface> (_w))
				return i;

			if (QObject* o = _w->parent ())
				return OAF::findInterface<_IInterface> (o);
		}

		return NULL;
	}
}

#endif /* __CUICONTAINER_H */
