/**
 * @file
 * @brief Реализация простейшего однострочного редактора plain text, умеющего показывать значки
 * @author Alexander Kamyshnikov <axill777@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#include <QApplication>
#include <QClipboard>
#include <QMimeData>
#include <QStyle>
#include <QMenu>
#include <QContextMenuEvent>

#include <OAF/CLineEdit.h>

bool
OAF::CLineEdit::clipboardHasData ()
{
	if (const QMimeData* mime = QApplication::clipboard ()->mimeData ())
		return mime->hasText ();

	return false;
}

void
OAF::CLineEdit::init ()
{
	createActions ();
	createContextMenu ();

	//
	// Будем отслеживать события буфера обмена и самого редактора (изменение/выделение текста)
	//
	connect (QApplication::clipboard (), SIGNAL(dataChanged ()), this, SLOT (aboutClipboardDataChanged ()));
	connect (this, SIGNAL (textChanged (const QString&)), this, SLOT (aboutTextChanged (const QString&)));
	connect (this, SIGNAL (selectionChanged ()), this, SLOT (aboutSelectionChanged ()));

	//
	// Создаем и скрываем объекты значков
	//
	m_icon [ICON_RIGHT] = new QLabel (this);
	m_icon [ICON_RIGHT]->setScaledContents (true);
	m_icon [ICON_RIGHT]->hide ();
	m_icon_enabled [ICON_RIGHT] = false;

	m_icon [ICON_LEFT] = new QLabel (this);
	m_icon [ICON_LEFT]->setScaledContents (true);
	m_icon [ICON_LEFT]->hide ();
	m_icon_enabled [ICON_LEFT] = false;

	//
	// Убеждаемся, что редактор использует системные стили
	//
	ensurePolished ();
}

void
OAF::CLineEdit::createActions ()
{
	m_undo = new QAction (OAF::CLineEdit::tr ("&Undo"), this);
	m_undo->setIcon (QIcon::fromTheme ("edit-undo", QIcon (":/icons/arrow_undo.png")));
	m_undo->setObjectName ("ui:lineedit:undo");
	m_undo->setShortcut (QKeySequence::Undo);
	m_undo->setEnabled (isUndoAvailable ());
	connect (m_undo, SIGNAL (triggered ()), this, SLOT (undo ()));

	m_redo = new QAction (OAF::CLineEdit::tr ("&Redo"), this);
	m_redo->setIcon (QIcon::fromTheme ("edit-redo", QIcon (":/icons/arrow_redo.png")));
	m_redo->setObjectName ("ui:lineedit:redo");
	m_redo->setShortcut (QKeySequence::Redo);
	m_redo->setEnabled (isRedoAvailable ());
	connect (m_redo, SIGNAL (triggered ()), this, SLOT (redo ()));

	m_cut = new QAction (OAF::CLineEdit::tr ("Cu&t"), this);
	m_cut->setIcon (QIcon::fromTheme ("edit-cut", QIcon (":/icons/cut.png")));
	m_cut->setObjectName ("ui:lineedit:cut");
	m_cut->setShortcut (QKeySequence::Cut);
	m_cut->setEnabled (false);
	connect (m_cut, SIGNAL (triggered ()), this, SLOT (cut ()));

	m_copy = new QAction (OAF::CLineEdit::tr ("&Copy"), this);
	m_copy->setIcon (QIcon::fromTheme ("edit-copy", QIcon (":/icons/copy.png")));
	m_copy->setObjectName ("ui:lineedit:copy");
	m_copy->setShortcut (QKeySequence::Copy);
	m_copy->setEnabled (false);
	connect (m_copy, SIGNAL (triggered ()), this, SLOT (copy ()));

	m_paste = new QAction (OAF::CLineEdit::tr ("&Paste"), this);
	m_paste->setIcon (QIcon::fromTheme ("edit-paste", QIcon (":/icons/paste.png")));
	m_paste->setObjectName ("ui:lineedit:paste");
	m_paste->setShortcut (QKeySequence::Paste);
	m_paste->setEnabled (clipboardHasData ());
	connect (m_paste, SIGNAL (triggered ()), this, SLOT (paste ()));

	m_delete = new QAction (OAF::CLineEdit::tr ("Delete"), this);
	m_delete->setIcon (QIcon::fromTheme ("edit-delete", QIcon (":/icons/delete.png")));
	m_delete->setObjectName ("ui:lineedit:delete");
	m_delete->setEnabled (false);
	connect (m_delete, SIGNAL (triggered ()), this, SLOT (clear ()));

	m_select_all = new QAction (OAF::CLineEdit::tr ("Select All"), this);
	m_select_all->setIcon (QIcon::fromTheme ("edit-select-all", QIcon (":/icons/select_all.png")));
	m_select_all->setObjectName ("ui:lineedit:select_all");
	m_select_all->setShortcut (QKeySequence::SelectAll);
	m_select_all->setEnabled (false);
	connect (m_select_all, SIGNAL (triggered ()), this, SLOT (selectAll ()));
}

void
OAF::CLineEdit::createContextMenu ()
{
	m_context_menu = new QMenu (this);

	m_context_menu->addAction (m_undo);
	m_context_menu->addAction (m_redo);

	m_context_menu->addSeparator ();
	m_context_menu->addAction (m_cut);
	m_context_menu->addAction (m_copy);
	m_context_menu->addAction (m_paste);
	m_context_menu->addAction (m_delete);

	m_context_menu->addSeparator ();
	m_context_menu->addAction (m_select_all);
}

void
OAF::CLineEdit::updateMargins ()
{
	//
	// Объекта изображения может и не быть, особенно если его отображение отключено
	//
	int rightMargin = 0;
	int leftMargin = 0;
	if (m_icon_enabled [ICON_RIGHT])
	{
		rightMargin = m_icon [ICON_RIGHT]->pixmap ()->width () + 8;

		//
		// NOTE: KDE не резервирует пространство для подсветки
		//
		if (style ()->inherits ("OxygenStyle"))
			rightMargin = qMax (24, rightMargin);
	}

	if (m_icon_enabled [ICON_LEFT])
	{
		leftMargin = m_icon [ICON_LEFT]->pixmap ()->width () + 8;

		//
		// NOTE: KDE не резервирует пространство для подсветки
		//
		if (style ()->inherits ("OxygenStyle"))
			leftMargin = qMax (24, leftMargin);
	}

	if (rightMargin || leftMargin)
	{
		QMargins margins (leftMargin, 0, rightMargin, 0);
		setTextMargins (margins);
	}
}

void
OAF::CLineEdit::setIconVisible (bool _visible, IconSide _side)
{
	m_icon [_side]->setVisible (_visible);
	m_icon_enabled [_side] = _visible;

	updateMargins ();
	updateButtonPositions ();
	update ();
}

bool
OAF::CLineEdit::iconVisible (IconSide _side) const
{
	return m_icon_enabled [_side];
}

void
OAF::CLineEdit::setIcon (const QPixmap& _icon, IconSide _side)
{
//	QPixmap scaled_icon = _icon.scaled (height() * 0.6, height () * 0.6,
//										Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
	//
	// FIXME: почему-то здесь размер значка уменьшается еще раз при втором вызове. Нехорошо!
	//
//	qDebug() << "New icon size is " << height() * 0.6;
//	if (height() * 0.6 < 18)
//		qDebug() << "!";
	//
	m_icon [_side]->setPixmap (/*scaled_icon*/ _icon);
	setIconVisible (true, _side);

	updateMargins ();
	updateButtonPositions ();
	update ();
}

const QPixmap*
OAF::CLineEdit::icon (IconSide _side) const
{
	return m_icon [_side]->pixmap ();
}

void
OAF::CLineEdit::setIconToolTip (const QString& _tip, IconSide _side)
{
	m_icon [_side]->setToolTip (_tip);
}

void
OAF::CLineEdit::resizeEvent (QResizeEvent*)
{
	updateButtonPositions ();
}

void
OAF::CLineEdit::contextMenuEvent (QContextMenuEvent* event)
{
	//
	// Показываем модифицированное меню и ждем,
	// пока пользователь выберет какой-то его пункт (т.е. синхронно)
	//
	m_context_menu->exec (event->globalPos ());
}

void
OAF::CLineEdit::updateButtonPositions ()
{
	//
	// FIXME: разобраться с размерами!
	//

	if (m_icon_enabled [ICON_RIGHT])
	{
		const int iconoffset_right = textMargins ().right () + 4;
		QRect icon_rect = m_icon [ICON_RIGHT]->pixmap()->rect ();
		m_icon [ICON_RIGHT]->setGeometry (width () - iconoffset_right, 2, icon_rect.width (), icon_rect.height ());
//		m_icon [ICON_RIGHT]->setGeometry (contentRect.adjusted (width () - iconoffset_right, 0, 0, 0));
	}

	if (m_icon_enabled [ICON_LEFT])
	{
		const int iconoffset_left = textMargins ().left () - height () * 0.6 - 4;
		QRect icon_rect = m_icon [ICON_LEFT]->pixmap()->rect ();
		m_icon [ICON_LEFT]->setGeometry (iconoffset_left, 2, icon_rect.width(), icon_rect.height());
	}

	update ();
}

void
OAF::CLineEdit::aboutClipboardDataChanged ()
{
	m_paste->setEnabled (clipboardHasData ());
}

void
OAF::CLineEdit::aboutTextChanged (const QString& _text)
{
	m_undo      ->setEnabled (isUndoAvailable ());
	m_redo      ->setEnabled (isRedoAvailable ());
	m_delete    ->setEnabled (!_text.isEmpty ());
	m_select_all->setEnabled (!_text.isEmpty ());
}

void
OAF::CLineEdit::aboutSelectionChanged ()
{
	m_copy->setEnabled (hasSelectedText ());
	m_cut ->setEnabled (hasSelectedText ());
}

OAF::CLineEdit::CLineEdit (QWidget *_parent) : QLineEdit (_parent)
{
	init ();
}

OAF::CLineEdit::CLineEdit (const QString &_text, QWidget *_parent) : QLineEdit (_text, _parent)
{
	init ();
}

OAF::CLineEdit::~CLineEdit ()
{}

void
OAF::CLineEdit::setUILabel (const QString &_bag, const QString &_label)
{
	Q_UNUSED (_bag);
	Q_UNUSED (_label);

	//
	// Переопределение заголовков окна не поддерживается
	//
}

void
OAF::CLineEdit::setUIContainer (OAF::IUIContainer* _uic)
{
	m_uic = _uic;
}

OAF::IUIContainer*
OAF::CLineEdit::getUIContainer ()
{
	return m_uic;
}

void
OAF::CLineEdit::activate (bool _active)
{
	static const QString edit_uidef =
		"<?xml version=\"1.0\"?>"
		"<uidef>"
			"<bag id=\"ui:menubar\">"
				"<folder id=\"ui:edit\" label=\"%1\">"
					"<item id=\"ui:lineedit:undo\"/>"
					"<item id=\"ui:lineedit:redo\"/>"
					"<separator/>"
					"<item id=\"ui:lineedit:cut\"/>"
					"<item id=\"ui:lineedit:copy\"/>"
					"<item id=\"ui:lineedit:paste\"/>"
					"<item id=\"ui:lineedit:delete\"/>"
					"<separator/>"
					"<item id=\"ui:lineedit:select_all\"/>"
				"</folder>"
			"</bag>"

			"<bag id=\"ui:toolbar\">"
				"<folder id=\"ui:edit\" label=\"%2\">"
					"<item id=\"ui:lineedit:undo\"/>"
					"<item id=\"ui:lineedit:redo\"/>"
					"<separator/>"
					"<item id=\"ui:lineedit:cut\"/>"
					"<item id=\"ui:lineedit:copy\"/>"
					"<item id=\"ui:lineedit:paste\"/>"
				"</folder>"
			"</bag>"
		"</uidef>";

	if (!_active)
		m_ui = m_uic->removeUI (m_ui);
	else
		m_ui = m_uic->addUI (edit_uidef.arg (::OAF::CLineEdit::tr ("Edit"), ::OAF::CLineEdit::tr ("Edit")), this);
}

QObject*
OAF::CLineEdit::getUIItem (const QString& _id)
{
	if (_id == m_undo->objectName ())
		return m_undo;
	else if (_id == m_redo->objectName ())
		return m_redo;
	else if (_id == m_cut->objectName ())
		return m_cut;
	else if (_id == m_copy->objectName ())
		return m_copy;
	else if (_id == m_paste->objectName ())
		return m_paste;
	else if (_id == m_delete->objectName ())
		return m_delete;
	else if (_id == m_select_all->objectName ())
		return m_select_all;

	Q_ASSERT_X (0, Q_FUNC_INFO, "Required UI item does not exists!");
	return NULL;
}
