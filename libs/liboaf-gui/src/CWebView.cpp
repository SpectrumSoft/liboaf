/**
 * @file
 * @brief Реализация строки поиска текста в веб-странице QWebView
 * @author Alexander Kamyshnikov <axill777@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */

#include <QShowEvent>
#include <QShortcut>

#include <OAF/CWebView.h>

#include "ui_CWebSearchWidget.h"
#include "ui_CWebView.h"


/////////////////////////////////////////////////////////////////////////////////////////////////////

OAF::CSearchBar::CSearchBar (QWidget* _parent) : QWidget (_parent),
	m_object (NULL),
	m_widget (NULL),
	m_timeLine (new QTimeLine (150, this)),
	m_ui (new Ui::CWebSearchWidget ())
{
	initializeSearchWidget ();

	//
	// Изначально панель поиска невидима
	//
	setMaximumHeight (0);
	m_widget->setGeometry (0, -1 * m_widget->height (),
						   m_widget->width (), m_widget->height ());
	hide ();

	connect (m_ui->btn_find_next, SIGNAL (clicked ()), this, SLOT (findNext()));
	connect (m_ui->btn_find_prev, SIGNAL (clicked ()), this, SLOT (findPrevious ()));
	connect (m_ui->edt_pattern, SIGNAL (returnPressed ()), this, SLOT (findNext ()));
	connect (m_ui->edt_pattern, SIGNAL (textEdited (const QString &)), this, SLOT (findNext ()));
	/*connect (ui.doneButton, SIGNAL (clicked ()), this, SLOT (animateHide ()));*/
	connect (m_timeLine, SIGNAL (frameChanged (int)), this, SLOT (frameChanged (int)));

	//
	// Будем скрывать панель поиска по нажатию клавиши Escape и соотв. кнопки в GUI
	//
	new QShortcut (QKeySequence (Qt::Key_Escape), this, SLOT (animateHide ()));
	connect (m_ui->btn_close, SIGNAL (clicked ()), this, SLOT (animateHide ()));
}

OAF::CSearchBar::~CSearchBar ()
{}

void
OAF::CSearchBar::initializeSearchWidget ()
{
	//
	// Создаём виджет-обёртку для панели поиска
	//
	m_widget = new QWidget (this);
	m_widget->setContentsMargins (0, 0, 0, 0);

	m_ui->setupUi (m_widget);

//	m_ui->btn_find_prev->setText (m_widget->layoutDirection () == Qt::LeftToRight? QChar(9664): QChar(9654));
//	m_ui->btn_find_next->setText (m_widget->layoutDirection () == Qt::LeftToRight? QChar(9654): QChar(9664));
//	m_ui->searchInfo->setText(QString());

	//
	// ... и настраиваем его размеры
	//
	setMinimumWidth (m_widget->minimumWidth ());
	setMaximumWidth (m_widget->maximumWidth ());
	setMinimumHeight (m_widget->minimumHeight ());
}

void
OAF::CSearchBar::setSearchObject (QObject* _object)
{
	Q_ASSERT (_object);

	m_object = _object;
}

QObject*
OAF::CSearchBar::searchObject () const
{
	return m_object;
}

void
OAF::CSearchBar::clear ()
{
	m_ui->edt_pattern->clear ();
}

void
OAF::CSearchBar::showFind ()
{
	//
	// Показываем панель поиска и устанавливаем фокус на редактор шаблона для поиска
	//
	if (! isVisible ())
	{
		show ();
		m_timeLine->setFrameRange (-1 * m_widget->height (), 0);
		m_timeLine->setDirection (QTimeLine::Forward);

		disconnect (m_timeLine, SIGNAL (finished ()), this, SLOT (hide ()));
		m_timeLine->start ();
	}

	m_ui->edt_pattern->setFocus ();
	m_ui->edt_pattern->selectAll ();
}

void
OAF::CSearchBar::resizeEvent (QResizeEvent* _event)
{
	if (_event->size ().width () != m_widget->width ())
		m_widget->resize (_event->size ().width (), m_widget->height ());

	QWidget::resizeEvent (_event);
}

void
OAF::CSearchBar::animateHide ()
{
	//
	// Анинимируем сокрытие панели поиска
	//
	m_timeLine->setDirection (QTimeLine::Backward);
	m_timeLine->start ();
	connect (m_timeLine, SIGNAL (finished ()), this, SLOT (hide ()));
}

void
OAF::CSearchBar::frameChanged (int _frame)
{
	//
	// На каждый кадр анимации потихоньку уменьшаем размер панели, что создаёт эффект её сокрытия
	//
	if (!m_widget)
		return;

	m_widget->move (0, _frame);

	int height = qMax (0, m_widget->y () + m_widget->height ());
	setMinimumHeight (height);
	setMaximumHeight (height);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

OAF::CWebViewSearch::CWebViewSearch (QWebView* _webView, QWidget* _parent) :
	OAF::CSearchBar (_parent)
{
	setSearchObject (_webView);
	/*m_ui->highlightAllButton->setVisible(true);

	connect (m_ui->highlightAllButton, SIGNAL(toggled(bool)), this, SLOT(highlightAll()));*/
	connect (m_ui->edt_pattern, SIGNAL (textEdited (const QString &)), this, SLOT (highlightAll ()));
}

OAF::CWebViewSearch::~CWebViewSearch ()
{}

void
OAF::CWebViewSearch::findNext ()
{
	find (QWebPage::FindWrapsAroundDocument);
}

void
OAF::CWebViewSearch::findPrevious ()
{
	find (QWebPage::FindBackward | QWebPage::FindWrapsAroundDocument);
}

void
OAF::CWebViewSearch::highlightAll ()
{
	webView ()->findText (QString (), QWebPage::HighlightAllOccurrences);

//	if (m_ui->highlightAllButton->isChecked())
		find (QWebPage::HighlightAllOccurrences);
}

void
OAF::CWebViewSearch::find (QWebPage::FindFlags _flags)
{
	QString searchString = m_ui->edt_pattern->text();
	if (!searchObject () || searchString.isEmpty ())
		return;

	QString infoString;
	if (!webView ()->findText (searchString, _flags))
		infoString = tr ("Not Found");
//	m_ui->searchInfo->setText( infoString);
}

QWebView*
OAF::CWebViewSearch::webView () const
{
	return qobject_cast <QWebView*> (searchObject ());
}

void
OAF::CWebViewSearch::hideEvent (QHideEvent* _event)
{
	OAF::CSearchBar::hideEvent (_event);

	webView ()->findText (QString (), QWebPage::HighlightAllOccurrences);
}

void
OAF::CWebViewSearch::showEvent (QShowEvent* _event)
{
	OAF::CSearchBar::showEvent (_event);

//	if (m_ui->highlightAllButton->isChecked())
		find (QWebPage::HighlightAllOccurrences);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

OAF::CWebViewWithSearch::CWebViewWithSearch (QWebView* _webView, QWidget* _parent):
	QWidget (_parent),
	m_webView (_webView)
{
	m_webView->setParent (this);

	m_webViewSearch = new OAF::CWebViewSearch (m_webView, this);

	//
	// Создаём вертикальный layout, где находятся панель поиска (сверху) и HTML-браузер
	//
	QVBoxLayout* layout = new QVBoxLayout;
	layout->setSpacing (0);
	layout->setContentsMargins (0, 0, 0, 0);
	layout->addWidget (m_webViewSearch);
	layout->addWidget (m_webView);
	setLayout (layout);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

void
OAF::CWebView::init ()
{
	m_qt_ui->setupUi (this);

	createActions ();
//	createContextMenu ();

	//
	// Создаём обёртку для поиска в QWebView и добавляем её в GUI-контейнер
	//
	m_web_view = new OAF::CWebViewWithSearch (m_qt_ui->web_view, this);

	//
	// Это необходимо, чтобы браузер показал свой фон по умолчанию;
	// иначе он будет просто как у окна, что вводит в заблуждение пользователя (будто браузера и нет)
	//
	m_qt_ui->web_view->setHtml (QString ());

	layout ()->addWidget (m_web_view);
}

void
OAF::CWebView::createActions ()
{
	//
	// Создаём действия для поиска текста в HTML-отчёте
	//
	m_find_action.reset (new QAction (tr ("Find text (Quick)"), this));
	m_find_action->setIcon (QIcon::fromTheme (QLatin1String ("edit-find")));
	m_find_action->setObjectName ("piket:tools:diff:find_action");
	m_find_action->setShortcut (QKeySequence::Find);
	connect (m_find_action.data (), SIGNAL (triggered ()), this, SLOT (aboutFind ()));
	//new QShortcut (QKeySequence (Qt::Key_Slash), this, SLOT (editFind ()));

	m_find_next_action.reset (new QAction (tr ("Find Next"), this));
	m_find_next_action->setObjectName ("piket:tools:diff:find_next_action");
	m_find_next_action->setShortcut (QKeySequence::FindNext);
	connect (m_find_next_action.data (), SIGNAL (triggered ()), this, SLOT (aboutFindNext ()));

	m_find_prev_action.reset (new QAction (tr ("Find Previous"), this));
	m_find_prev_action->setObjectName ("piket:tools:diff:find_prev_action");
	m_find_prev_action->setShortcut (QKeySequence::FindPrevious);
	connect (m_find_prev_action.data (), SIGNAL (triggered ()), this, SLOT (aboutFindPrev ()));
}

void
OAF::CWebView::aboutFind ()
{
	m_web_view->m_webViewSearch->showFind ();
}

void
OAF::CWebView::aboutFindNext ()
{
	m_web_view->m_webViewSearch->findNext ();
}

void
OAF::CWebView::aboutFindPrev ()
{
	m_web_view->m_webViewSearch->findPrevious ();
}


OAF::CWebView::CWebView (QWidget *_parent) : QWidget (_parent), m_qt_ui (new Ui::CWebView ())
{
	init ();
}

OAF::CWebView::~CWebView ()
{}

void
OAF::CWebView::enableSearchActions (bool _enable)
{
	m_find_action->setEnabled (_enable);
	m_find_next_action->setEnabled (_enable);
	m_find_prev_action->setEnabled (_enable);
}

void
OAF::CWebView::print (QPrinter* _printer) const
{
	m_qt_ui->web_view->print (_printer);
}

/*bool
OAF::CWebView::hasSelection () const
{
	return m_qt_ui->web_view->hasSelection ();
}*/

void
OAF::CWebView::setHtml (const QString& _html, const QUrl& _baseUrl)
{
	m_qt_ui->web_view->setHtml (_html, _baseUrl);
}

void
OAF::CWebView::setUILabel (const QString &_bag, const QString &_label)
{
	Q_UNUSED (_bag);
	Q_UNUSED (_label);

	//
	// Переопределение заголовков окна не поддерживается
	//
}

void
OAF::CWebView::setUIContainer (OAF::IUIContainer* _uic)
{
	m_uic = _uic;
}

OAF::IUIContainer*
OAF::CWebView::getUIContainer ()
{
	return m_uic;
}

void
OAF::CWebView::activate (bool _active)
{
	static const QString edit_uidef =
			"<?xml version=\"1.0\"?>"
			"<uidef>"
				"<bag id=\"ui:menubar\">"
					"<folder id=\"ui:edit\" label=\"%1\" priority=\"-500\">"
						"<separator/>"
						"<item id=\"piket:tools:diff:find_action\" priority=\"1000\" />"
						"<item id=\"piket:tools:diff:find_next_action\" priority=\"1000\" />"
						"<item id=\"piket:tools:diff:find_prev_action\" priority=\"1000\" />"
					"</folder>"
				"</bag>"
			"</uidef>";

	if (!_active)
		m_ui = m_uic->removeUI (m_ui);
	else
		m_ui = m_uic->addUI (edit_uidef.arg (::OAF::CWebView::tr ("Edit")), this);
}

QObject*
OAF::CWebView::getUIItem (const QString& _id)
{
	if (_id == m_find_action->objectName ())
		return m_find_action.data ();
	else if (_id == m_find_next_action->objectName ())
		return m_find_next_action.data ();
	else if (_id == m_find_prev_action->objectName ())
		return m_find_prev_action.data ();

	Q_ASSERT_X (0, Q_FUNC_INFO, "Required UI item does not exists");
	return NULL;
}
