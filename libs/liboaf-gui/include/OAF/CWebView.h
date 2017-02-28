/**
 * @file
 * @brief Интерфейс HTML-браузера со строкой поиска текста, реализующий OAF::IUIComponent
 * @author Alexander Kamyshnikov <axill777@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#ifndef __CWEBSEARCH_WIDGET_H
#define __CWEBSEARCH_WIDGET_H

#include <QtCore>
#include <QtGui>
#include <QtWebKit>
#if (QT_VERSION >= QT_VERSION_CHECK (5, 0, 0))
#include <QtWidgets>
#include <QtWebKitWidgets>
#endif

#include <idl/IUIComponent.h>

#include <OAF/OafGuiGlobal.h>

namespace Ui
{
	class CWebView;
	class CWebSearchWidget;
}

namespace OAF
{
	/**
	 * @brief Интерфейс универсальной панели для поиска текста в каком-либо представлении,
	 * с поддержкой анимированного показа/скрытия
	 */
	class OAFGUI_EXPORT CSearchBar : public QWidget
	{
		Q_OBJECT

		/**
		 * @brief Виджет, в котором собственно и будем искать текст
		 */
		QObject* m_object;

		/**
		 * @brief Хитрый виджет панели поиска (нужен для дополнительного контроля размеров и анимации)
		 */
		QWidget* m_widget;

		/**
		 * @brief Объект-аниматор показа/скрытия панели поиска
		 */
		QTimeLine* m_timeLine;

	protected:
		/**
		 * @brief Доступный наследникам класса объект графического интерфейса Qt
		 */
		QScopedPointer<Ui::CWebSearchWidget> m_ui;

	private:
		/**
		 * @brief Инициализация и настройка размеров панели поиска
		 */
		void initializeSearchWidget ();

	private slots:
		/**
		 * @brief Слот, вызываемый при смене очередного кадра анимации
		 */
		void frameChanged (int _frame);

	protected:
		void resizeEvent (QResizeEvent* _event);

	public:
		CSearchBar (QWidget* _parent = 0);
		~CSearchBar ();

		/**
		 * @brief Устанавливает объект, в котором будет искаться текст
		 */
		void setSearchObject (QObject* _object);

		/**
		 * @brief Возвращает объект, в котором будет искаться текст
		 */
		QObject* searchObject () const;

	public slots:
		/**
		 * @brief Найти следующее совпадение с введенным пользователем шаблоном текста
		 */
		virtual void findNext () = 0;

		/**
		 * @brief Найти предыдущее совпадение с введенным пользователем шаблоном текста
		 */
		virtual void findPrevious () = 0;

		/**
		 * @brief Анимировать сокрытие панели поиска
		 */
		void animateHide ();

		/**
		 * @brief Очистить текст шаблона для поиска, введенного пользователем
		 */
		void clear ();

		/**
		 * @brief Показать панель поиска
		 */
		void showFind ();
	};

	/**
	 * @brief Панель для поиска текста в HTML-браузере QWebView,
	 * с поддержкой анимированного показа/скрытия
	 */
	class OAFGUI_EXPORT CWebViewSearch : public CSearchBar
	{
		Q_OBJECT

		/**
		 * @brief Найти введенный пользователем текст в QWebView
		 */
		void find (QWebPage::FindFlags _flags);

		/**
		 * @brief Доступ к объекту HTML-браузера
		 */
		QWebView* webView () const;

		void hideEvent (QHideEvent* _event);
		void showEvent (QShowEvent* _event);

	public:
		CWebViewSearch (QWebView* _webView, QWidget* _parent = 0);
		~CWebViewSearch ();

	public slots:
		void findNext ();
		void findPrevious ();

		/**
		 * @brief Подсветить в HTML-браузере все вхождения введенного пользователем текста для поиска
		 */
		void highlightAll ();
	};

	/**
	 * @brief Обёртка для отображения HTML-браузера со строкой поиска
	 */
	class OAFGUI_EXPORT CWebViewWithSearch : public QWidget
	{
		Q_OBJECT

	public:
		CWebViewWithSearch (QWebView* _webView, QWidget* _parent = 0);

		QWebView* m_webView;
		CWebViewSearch* m_webViewSearch;
	};

	/**
	 * @brief Виджет HTML-браузера с возможностью поиска текста и с реализацией интерфейса OAF::IUIComponent
	 *
	 * Управление временем жизни объектов данного класса должно выполняться средствами Qt так же, как
	 * и управление временем жизни остальных виджетов.
	 */
	class OAFGUI_EXPORT CWebView : public QWidget,
			//
			// Экспортируемые интерфейсы
			//
			virtual public OAF::IUIComponent
	{
		Q_OBJECT

		/**
		 * @brief Объект графического интерфейса Qt
		 */
		QScopedPointer<Ui::CWebView> m_qt_ui;

		/**
		 * @brief Контейнер интерфейса, в который будут добавляться меню
		 */
		OAF::IUIContainer* m_uic;

		/**
		 * @brief Контекстное меню редактора со стандартными действиями для браузера
		 */
		// TODO:
		//QMenu* m_context_menu;

		/**
		 * @brief Набор стандартных действий для меню
		 */
		/** @{*/
		QScopedPointer<QAction> m_find_action;
		QScopedPointer<QAction> m_find_next_action;
		QScopedPointer<QAction> m_find_prev_action;
		/** @}*/

		/**
		 * @brief Идентификатор добавленного интерфейса
		 */
		QUuid m_ui;

		CWebViewWithSearch* m_web_view;

		void init ();

		/**
		 * @brief Инициализация списка действий Qt
		 */
		void createActions ();

	private slots:
		void aboutFind ();
		void aboutFindNext ();
		void aboutFindPrev ();

	public slots:
		void print (QPrinter* _printer) const;

	public:
		CWebView (QWidget* _parent = 0);
		~CWebView ();

		/**
		 * @brief Делает действия для поиска доступными/недоступными для пользователя
		 *
		 * @note По умолчанию они доступны
		 */
		void enableSearchActions (bool _enable = true);

		//
		// FIXME: в Qt 4.6 метод QWebView::hasSelection() отсутствует, поэтому пришлось закомментить;
		// такая древняя Qt используется в МСВС
		//
		/**
		 * @brief Проверяет, есть ли выделенный текст в HTML-браузере
		 */
		//	bool hasSelection () const;

		/**
		 * @brief Устанавливает новую Web-страницу браузеру
		 */
		void setHtml (const QString& _html, const QUrl& _baseUrl = QUrl ());

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

#endif /* __CWEBSEARCH_WIDGET_H */
