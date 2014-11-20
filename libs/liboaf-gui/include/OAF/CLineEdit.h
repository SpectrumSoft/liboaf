/**
 * @file
 * @brief Интерфейс однострочного редактора plain text
 * @author Alexander Kamyshnikov <axill777@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 *
 * Данный виджет реализует интерфейс IUIComponent и умеет добавлять в меню
 * приложений свои стандартные действия (копирование, вставка, и т.п.).
 * Кроме этого, он умеет показывать значки слева и справа от текста.
 */
#ifndef __CLINEEDIT_H
#define __CLINEEDIT_H

#include <OAF/OafGuiGlobal.h>

#include <QAction>
#include <QLineEdit>
#include <QLabel>

#include <idl/IUIComponent.h>

namespace OAF
{
	/**
	 * @brief Виджет стандартного строчного редактора с реализацией интерфейса OAF::IUIComponent
	 *
	 * Управление временем жизни объектов данного класса должно выполняться средствами Qt так же, как
	 * и управление временем жизни остальных виджетов.
	 */
	class OAFGUI_EXPORT CLineEdit : public QLineEdit,
			//
			// Экспортируемые интерфейсы
			//
			virtual public OAF::IUIComponent
	{
		Q_OBJECT

		/**
		 * @brief Контейнер интерфейса, в который будут добавляться меню
		 */
		OAF::IUIContainer* m_uic;

		/**
		 * @brief Контекстное меню редактора со стандартными действиями для редактирования
		 */
		QMenu* m_context_menu;

		/**
		 * @brief Набор стандартных действий для меню
		 */
		/** @{*/
		QAction* m_undo;
		QAction* m_redo;
		QAction* m_cut;
		QAction* m_copy;
		QAction* m_paste;
		QAction* m_delete;
		QAction* m_select_all;
		/** @}*/

		/**
		 * @brief Идентификатор добавленного интерфейса
		 */
		QUuid m_ui;

		/**
		 * @brief Значок для отображения рядом с правой/левой границей редактора
		 */
		QLabel* m_icon[2];

		/**
		 * @brief Показывать ли значок справа/слева
		 */
		bool m_icon_enabled[2];

		bool clipboardHasData ();

		void init ();

		/**
		 * @brief Инициализация списка экшенов
		 */
		void createActions ();

		/**
		 * @brief Создание контекстного меню редактора со стандартными действиями
		 */
		void createContextMenu ();

		void updateMargins ();
		void updateButtonPositions ();

	private slots:
		/**
		 * @brief Реакция на изменение данных в буфере обмена
		 */
		void aboutClipboardDataChanged ();

		/**
		 * @brief Реакция на изменение текста в редакторе
		 */
		void aboutTextChanged (const QString& _text);

		/**
		 * @brief Реакция на изменение выбранного текста
		 */
		void aboutSelectionChanged ();

	protected:
		virtual void resizeEvent (QResizeEvent*);
		virtual void contextMenuEvent (QContextMenuEvent*);

	public:
		CLineEdit (QWidget* _parent = 0);
		CLineEdit (const QString& _text, QWidget* _parent = 0);
		~CLineEdit ();

		enum IconSide { ICON_RIGHT = 0, ICON_LEFT };
		/**
		 * @brief Устанавливает/возвращает значок с правой/левой стороны редактора
		 */
		void setIcon (const QPixmap& _icon, IconSide _side = ICON_RIGHT);
		const QPixmap* icon (IconSide _side = ICON_RIGHT) const;

		/**
		 * @brief Устанавливает/возвращает видимость значка (по умолчанию его нет)
		 */
		void setIconVisible (bool _visible, IconSide _side = ICON_RIGHT);
		bool iconVisible (IconSide _side = ICON_RIGHT) const;

		/**
		 * @brief Устанавливает всплывающую подсказку для значка
		 */
		void setIconToolTip (const QString& _tip, IconSide _side = ICON_RIGHT);

		/**
		 * @name Реализация интерфейса OAF::IUIComponent
		 */
		/** @{*/
		void setUILabel (const QString& _bag, const QString& _label);
		void setUIContainer (OAF::IUIContainer* _uic);
		OAF::IUIContainer* getUIContainer ();
		void activate (bool _active);
		QObject* getUIItem (const QString& _id);
		/** @}*/
	};
}

#endif /* __CLINEEDIT_H */
