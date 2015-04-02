/**
 * @file
 * @brief QTabWidget с автоматическим подключением/отключением фильтра событий
 * @author Alexander Kamyshnikov <axill777@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#ifndef __CTABWIDGET_H
#define __CTABWIDGET_H

#include <QtCore>
#include <QtGui>
#if (QT_VERSION >= QT_VERSION_CHECK (5, 0, 0))
#include <QtWidgets>
#endif

#include <OAF/OafGuiGlobal.h>

namespace OAF
{
	/**
	 * @brief QTabWidget с автоматическим подключением/отключением фильтра событий
	 */
	class OAFGUI_EXPORT CTabWidget : public QTabWidget
	{
		Q_OBJECT

		/**
		 * @brief Фильтр, который будет устанавливаться всем добавляемым виджетам
		 */
		QObject* m_event_filter;

		/**
		 * @brief Всегда показывать табы
		 */
		bool m_allways_show_tab;

		/**
		 * @brief Список виджетов
		 *
		 * Данный список необходим для того, чтобы корректно отключать виджеты от фильтра,
		 * так как tabRemoved вызывается после удаления таба для виджета и функция widgets
		 * возвращает уже не то, что надо
		 */
		QList<QWidget*> m_widgets;

		/**
		 * @brief Если в контейнере только один таб, скрывать заголовок
		 */
		void updateTabBar ();

	protected:
		void tabInserted (int _idx);
		void tabRemoved (int _idx);

	public:
		CTabWidget (QWidget* _parent = NULL);

		/**
		 * @brief Задать или сбросить фильтр событий, который будет добавляться ко всем виджетам
		 */
		void setInstallableEventFilter (QObject* _event_filter = NULL);

		/**
		 * @brief Всегда показывать табы
		 */
		void setAllwaysShowTab (bool _on);
	};
}

#endif /* __CTABWIDGET_H */
