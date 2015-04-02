/**
 * @file
 * @brief Реализация HTML редактора
 * @author Sergey N. Yatskevich <syatskevich@gmail.com>
 * @copyright SpectrumSoft. All rights reserved. This file is part of liboaf,
 *            distributed under the GNU GPL v2 with a Linking Exception. For
 *            full terms see the included COPYING file.
 */
#include <OAF/OAF.h>
#include <OAF/StreamUtils.h>
#include <OAF/Helpers.h>
#include <OAF/MimeHelpers.h>
#include <OAF/CNotifySource.h>

#include "CPlainEdit.h"

#include "ui_CResizeDialog.h"
#include "ui_CTableDialog.h"
#include "ui_CInputDialog.h"
#include "ui_CTableAppreance.h"

using namespace OAF::TXT;

bool
CPlainEdit::clipboardHasData ()
{
	if (const QMimeData* mime = QApplication::clipboard ()->mimeData ())
		return canInsertFromMimeData (mime);

	return false;
}

void
CPlainEdit::createEditActions ()
{
	m_undo_action = new QAction (tr ("&Undo"), this);
	m_undo_action->setIcon (QIcon::fromTheme ("edit-undo", QIcon (":/liboaf-txt/icons/editundo.png")));
	m_undo_action->setPriority (QAction::LowPriority);
	m_undo_action->setShortcut (QKeySequence::Undo);
	m_undo_action->setObjectName ("ui:plainedit:undo");

	m_redo_action = new QAction (tr ("&Redo"), this);
	m_redo_action->setIcon (QIcon::fromTheme ("edit-redo", QIcon (":/liboaf-txt/icons/editredo.png")));
	m_redo_action->setPriority (QAction::LowPriority);
	m_redo_action->setShortcut (QKeySequence::Redo);
	m_redo_action->setObjectName ("ui:plainedit:redo");

	m_cut_action = new QAction (tr ("Cu&t"), this);
	m_cut_action->setIcon (QIcon::fromTheme ("edit-cut", QIcon (":/liboaf-txt/icons/editcut.png")));
	m_cut_action->setPriority (QAction::LowPriority);
	m_cut_action->setShortcut (QKeySequence::Cut);
	m_cut_action->setObjectName ("ui:plainedit:cut");

	m_copy_action = new QAction (tr ("&Copy"), this);
	m_copy_action->setIcon (QIcon::fromTheme ("edit-copy", QIcon (":/liboaf-txt/icons/editcopy.png")));
	m_copy_action->setPriority (QAction::LowPriority);
	m_copy_action->setShortcut (QKeySequence::Copy);
	m_copy_action->setObjectName ("ui:plainedit:copy");

	m_paste_action = new QAction (tr ("&Paste"), this);
	m_paste_action->setIcon (QIcon::fromTheme ("edit-paste", QIcon (":/liboaf-txt/icons/editpaste.png")));
	m_paste_action->setPriority (QAction::LowPriority);
	m_paste_action->setShortcut (QKeySequence::Paste);
	m_paste_action->setObjectName ("ui:plainedit:paste");
	m_paste_action->setEnabled (clipboardHasData ());

	m_delete_action = new QAction (tr ("Delete"), this);
	m_delete_action->setPriority (QAction::LowPriority);
	m_delete_action->setObjectName ("ui:plainedit:delete");
	m_delete_action->setEnabled (m_document && !m_document->isEmpty ());

	m_select_all_action = new QAction (tr ("Select &All"), this);
	m_select_all_action->setPriority (QAction::LowPriority);
	m_select_all_action->setObjectName ("ui:plainedit:select_all");
	m_select_all_action->setShortcut (QKeySequence::SelectAll);
	m_select_all_action->setEnabled (m_document && !m_document->isEmpty ());
}

void
CPlainEdit::createContextMenu ()
{
	m_context_menu = new QMenu (this);

	m_context_menu->addAction (m_undo_action);
	m_context_menu->addAction (m_redo_action);

	m_context_menu->addSeparator ();
	m_context_menu->addAction (m_cut_action);
	m_context_menu->addAction (m_copy_action);
	m_context_menu->addAction (m_paste_action);
	m_context_menu->addAction (m_delete_action);

	m_context_menu->addSeparator ();
	m_context_menu->addAction (m_select_all_action);
}

void
CPlainEdit::setupActions ()
{
	//
	// Ловим undo/redo от документа
	//
	connect (document (), SIGNAL (modificationChanged (bool)), this, SLOT (setWindowModified (bool)));
	connect (this, SIGNAL (undoAvailable (bool)), m_undo_action, SLOT (setEnabled (bool)));
	connect (this, SIGNAL (redoAvailable (bool)), m_redo_action, SLOT (setEnabled (bool)));

	m_undo_action->setEnabled (document ()->isUndoAvailable ());
	m_redo_action->setEnabled (document ()->isRedoAvailable ());

	connect (m_undo_action, SIGNAL (triggered ()), this, SLOT (undo ()));
	connect (m_redo_action, SIGNAL (triggered ()), this, SLOT (redo ()));

	//
	// Настраиваем действия вырезания/копирования/вставки
	//
	m_cut_action->setEnabled (false);
	m_copy_action->setEnabled (false);

	connect (m_cut_action, SIGNAL(triggered ()), this, SLOT (cut()));
	connect (m_copy_action, SIGNAL(triggered ()), this, SLOT (copy()));
	connect (m_paste_action, SIGNAL(triggered ()), this, SLOT (paste()));
	connect (m_delete_action, SIGNAL(triggered ()), this, SLOT (clear ()));
	connect (m_select_all_action, SIGNAL(triggered ()), this, SLOT (selectAll ()));

	connect (this, SIGNAL (copyAvailable (bool)), m_cut_action, SLOT (setEnabled (bool)));
	connect (this, SIGNAL (copyAvailable (bool)), m_copy_action, SLOT (setEnabled (bool)));
}

void
CPlainEdit::dropTextFile (const QUrl& _url)
{
	QFile file (OAF::toLocalFile (_url));
	if (file.open (QIODevice::ReadOnly | QIODevice::Text))
		textCursor ().insertText (file.readAll ());
}

void
CPlainEdit::notify (OAF::IInterface* _event, OAF::INotifySource* _source, OAF::INotifyListener* _origin)
{
	Q_UNUSED (_event);
	Q_UNUSED (_source);
	Q_UNUSED (_origin);

	//
	// Если документ пуст, то блокируем delete и select_all
	//
	m_delete_action->setEnabled (m_document && !m_document->isEmpty ());
	m_select_all_action->setEnabled (m_document && !m_document->isEmpty ());
}

void
CPlainEdit::aboutCursorPositionChanged ()
{
	//
	// Нет необходимости что-то настраивать в зависимости от положения
	// курсора в тексте
	//
}

void
CPlainEdit::aboutClipboardDataChanged ()
{
	m_paste_action->setEnabled (clipboardHasData ());
}

bool
CPlainEdit::canInsertFromMimeData (const QMimeData* _source) const
{
	return _source->hasUrls () || _source->hasText () || _source->hasHtml ();
}

void
CPlainEdit::insertFromMimeData (const QMimeData* _source)
{
	if (_source->hasUrls ())
	{
		foreach (const QUrl& url, _source->urls())
			dropTextFile (url);
	}
	else
		QTextEdit::insertFromMimeData (_source);
}

void
CPlainEdit::contextMenuEvent (QContextMenuEvent* _event)
{
	m_context_menu->exec (_event->globalPos ());
}

CPlainEdit::CPlainEdit (CFactory* _factory) : CUnknown (html_edit_cid),
	m_factory (_factory), m_uic (NULL)
{
	setObjectName ("ui:plainedit");

	//
	// Создаем стандартные действия
	//
	createEditActions ();
	createContextMenu ();

	//
	// Реагируем на изменение положения курсора в тексте
	//
	connect (this, SIGNAL (cursorPositionChanged ()), this, SLOT (aboutCursorPositionChanged ()));

	//
	// Связываем команды редактирования с документом
	//
	setupActions ();

	//
	// Ловим события об изменении буфера обмена
	//
	connect (QApplication::clipboard (), SIGNAL(dataChanged ()), this, SLOT(aboutClipboardDataChanged ()));

	//
	// Создаём пустой документ заданного класса для редактирования
	//
	setDocument (m_document = OAF::createFromName<QTextDocument> ("cid:OAF/TXT/CPlainDocument:1.0"));
	OAF::subscribe (m_document.queryInterface<OAF::IInterface> (), this);
}

CPlainEdit::~CPlainEdit ()
{
	setDocument (NULL);
	OAF::unsubscribe (m_document.queryInterface<OAF::IInterface> (), this);

	disconnect (QApplication::clipboard (), SIGNAL(dataChanged ()), this, SLOT(aboutClipboardDataChanged ()));
}

OAF::URef<OAF::IUnknown>
CPlainEdit::setExtendedObject (OAF::IUnknown* _o)
{
	//
	// Устанавливаем новый документ для редактирования
	//
	if (OAF::URef<QTextDocument> document = OAF::queryInterface<QTextDocument> (_o))
	{
		OAF::unsubscribe (m_document.queryInterface<OAF::IInterface> (), this);
		setDocument (NULL);
		m_document = document;
		setDocument (m_document);
		OAF::subscribe (m_document.queryInterface<OAF::IInterface> (), this);
	}

	//
	// Если документ пуст, то блокируем delete и select_all
	//
	m_delete_action->setEnabled (m_document && !m_document->isEmpty ());
	m_select_all_action->setEnabled (m_document && !m_document->isEmpty ());

	return this;
}

void
CPlainEdit::setUILabel (const QString& _bag, const QString& _label)
{
	if (_bag == "ui:main")
		m_main_label = _label;
}

void
CPlainEdit::setUIContainer (OAF::IUIContainer* _uic)
{
	//
	// Добавляем в интерфейс пользователя окно редактора
	//
	static const char uidef[] =
		"<?xml version=\"1.0\"?>"
		"<uidef>"
			"<bag id=\"ui:main\">"
				"<item id=\"ui:plainedit\" label=\"%1\"/>"
			"</bag>"
		"</uidef>";

	if (_uic)
	{
		m_uic = _uic;
		m_id  = m_uic->addUI (QString (uidef).arg (m_main_label.isEmpty () ? tr ("Text Edit") : m_main_label), this);
	}
	else
	{
		m_id  = m_uic->removeUI (m_id);
		m_uic = _uic;
	}
}

OAF::IUIContainer*
CPlainEdit::getUIContainer ()
{
	return m_uic;
}

void
CPlainEdit::activate (bool _activate)
{
	static const QString menubar_edit_uidef =
			"<folder id=\"ui:edit\" label=\"%1\">"
				"<item id=\"ui:plainedit:undo\"/>"
				"<item id=\"ui:plainedit:redo\"/>"
				"<separator/>"
				"<item id=\"ui:plainedit:cut\"/>"
				"<item id=\"ui:plainedit:copy\"/>"
				"<item id=\"ui:plainedit:paste\"/>"
				"<item id=\"ui:plainedit:delete\"/>"
				"<separator/>"
				"<item id=\"ui:plainedit:select_all\"/>"
			"</folder>";

	static const QString toolbar_edit_uidef =
			"<folder id=\"ui:edit\" label=\"%1\">"
				"<item id=\"ui:plainedit:cut\"/>"
				"<item id=\"ui:plainedit:copy\"/>"
				"<item id=\"ui:plainedit:paste\"/>"
				"<separator/>"
				"<item id=\"ui:plainedit:undo\"/>"
				"<item id=\"ui:plainedit:redo\"/>"
			"</folder>";

	static const QString uidef =
			"<?xml version=\"1.0\"?>"
			"<uidef>"
				"<bag id=\"ui:menubar\">"
					+ menubar_edit_uidef.arg (tr ("Edit")) +
				"</bag>"
				"<bag id=\"ui:toolbar\">"
					+ toolbar_edit_uidef.arg (tr ("Edit")) +
				"</bag>"
			"</uidef>";

	if (! _activate)
		m_id_active = m_uic->removeUI (m_id_active);

	if (_activate)
		m_id_active = m_uic->addUI (uidef, this);
}

QObject*
CPlainEdit::getUIItem (const QString& _id)
{
	if (_id == objectName ())
		return this;
	else if (_id == m_undo_action->objectName ())
		return m_undo_action;
	else if (_id == m_redo_action->objectName ())
		return m_redo_action;
	else if (_id == m_cut_action->objectName ())
		return m_cut_action;
	else if (_id == m_copy_action->objectName ())
		return m_copy_action;
	else if (_id == m_paste_action->objectName ())
		return m_paste_action;
	else if (_id == m_delete_action->objectName ())
		return m_delete_action;
	else if (_id == m_select_all_action->objectName ())
		return m_select_all_action;

	return NULL;
}
