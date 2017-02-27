/**
 * @file
 * @brief Интерфейс строки поиска
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#ifndef __CSEARCHLINE_WIDGET_H
#define __CSEARCHLINE_WIDGET_H

#include <QtCore>
#include <QtGui>
#if (QT_VERSION >= QT_VERSION_CHECK (5, 0, 0))
#include <QtWidgets>
#endif

#include <OAF/ViewFindHelper.h>
#include <OAF/OafGuiGlobal.h>

namespace Ui
{
	class CSearchLineWidget;
}

namespace OAF
{
	class OAFGUI_EXPORT CSearchLineWidget : public QWidget
	{
		Q_OBJECT

		/**
		 * @brief Интерфейс
		 */
		QScopedPointer<Ui::CSearchLineWidget> m_ui;

		/**
		 * @brief Помощник для поиска
		 *
		 * В данном помощнике хранится и указатель на представление, в котором ведётся поиск
		 */
		QScopedPointer<OAF::Helpers::ViewFindHelper> m_finder;

		bool eventFilter (QObject* _o, QEvent* _ev);

    signals:
		void activated (const QModelIndex& _idx);

	public:
		explicit CSearchLineWidget (QWidget* _parent = NULL);
		~CSearchLineWidget ();

		/**
		 * @brief Установить view для поисков
		 */
		void setView (QAbstractItemView* _view = NULL);

		/**
		 * @brief Устанавливает индекс столбца, по которому будет осуществляться поиск
		 *
		 * @note Эта функция должна вызываться только после setView()
		 */
		void setSearchColumn (int _idx);
	};
}

#endif /* __CSEARCHLINE_WIDGET_H */
